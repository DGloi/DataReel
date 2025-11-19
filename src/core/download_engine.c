#include "download_engine.h"
#include "ytdlp_manager.h"
#include <fcntl.h>

static gboolean parse_progress_line(const char *line, DownloadItem *item);

DownloadItem* download_item_new(const char *url, const char *output_path,
                                DownloadOptions *opts) {
    DownloadItem *item = g_malloc0(sizeof(DownloadItem));
    item->url = g_strdup(url);
    item->output_path = g_strdup(output_path);
    item->options = opts;
    item->status = DOWNLOAD_STATUS_IDLE;
    item->progress = 0.0;
    item->process_id = -1;
    item->read_fd = -1;

    return item;
}

void download_item_free(DownloadItem *item) {
    if (!item) return;

    if (item->io_watch_id > 0) {
        g_source_remove(item->io_watch_id);
    }
    if (item->io_channel) {
        g_io_channel_unref(item->io_channel);
    }
    if (item->read_fd >= 0) {
        close(item->read_fd);
    }

    g_free(item->url);
    g_free(item->output_path);
    g_free(item->eta);
    g_free(item->error_message);

    if (item->options) {
        g_free(item->options->custom_format);
        g_free(item->options->time_range_start);
        g_free(item->options->time_range_end);
        g_free(item->options->output_template);
        g_free(item->options);
    }

    if (item->metadata) {
        metadata_free(item->metadata);
    }

    g_free(item);
}

static gboolean on_stdout_readable(GIOChannel *channel, GIOCondition cond,
                                   gpointer user_data) {
    DownloadItem *item = (DownloadItem *)user_data;

    if (cond & G_IO_HUP) {
        item->status = DOWNLOAD_STATUS_COMPLETED;
        return FALSE;
    }

    char *line = NULL;
    gsize length;
    GError *error = NULL;

    GIOStatus status = g_io_channel_read_line(channel, &line, &length, NULL, &error);

    if (status == G_IO_STATUS_NORMAL && line) {
        parse_progress_line(line, item);
        g_free(line);
        return TRUE;
    } else if (status == G_IO_STATUS_EOF) {
        item->status = DOWNLOAD_STATUS_COMPLETED;
        return FALSE;
    }

    return TRUE;
}

gboolean download_item_start(DownloadItem *item) {
    if (!item || item->status == DOWNLOAD_STATUS_DOWNLOADING) {
        return FALSE;
    }

    int argc;
    char **args = ytdlp_build_args(item->url, item->output_path, item->options, &argc);

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        ytdlp_free_args(args);
        return FALSE;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        close(pipefd[0]); // Close read end

        // Redirect stdout and stderr to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        execvp(args[0], args);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        close(pipefd[1]); // Close write end

        item->process_id = pid;
        item->status = DOWNLOAD_STATUS_DOWNLOADING;
        item->read_fd = pipefd[0];

        // Make read end non-blocking
        int flags = fcntl(item->read_fd, F_GETFL, 0);
        fcntl(item->read_fd, F_SETFL, flags | O_NONBLOCK);

        // Create GIOChannel for async reading
        item->io_channel = g_io_channel_unix_new(item->read_fd);
        g_io_channel_set_encoding(item->io_channel, NULL, NULL);
        g_io_channel_set_buffered(item->io_channel, FALSE);

        item->io_watch_id = g_io_add_watch(item->io_channel,
                                          G_IO_IN | G_IO_HUP | G_IO_ERR,
                                          on_stdout_readable,
                                          item);

        ytdlp_free_args(args);
        g_print("Download started with PID: %d\n", pid);
        return TRUE;
    }

    ytdlp_free_args(args);
    return FALSE;
}

gboolean download_item_cancel(DownloadItem *item) {
    if (!item || item->process_id <= 0) {
        return FALSE;
    }

    if (kill(item->process_id, SIGTERM) == 0) {
        item->status = DOWNLOAD_STATUS_CANCELLED;
        return TRUE;
    }

    return FALSE;
}

static gboolean parse_progress_line(const char *line, DownloadItem *item) {
    // Parse yt-dlp progress output
    // Format: [download]  45.2% of 123.45MiB at 1.23MiB/s ETA 00:42

    if (strstr(line, "[download]") && strstr(line, "%")) {
        float percent;
        if (sscanf(line, "[download] %f%%", &percent) == 1) {
            item->progress = percent;
        }

        // Parse speed
        char *speed_str = strstr(line, "at ");
        if (speed_str) {
            float speed_val;
            char unit[10];
            if (sscanf(speed_str, "at %f%s", &speed_val, unit) == 2) {
                // Convert to bytes/sec
                if (strstr(unit, "MiB")) {
                    item->speed = speed_val * 1024 * 1024;
                } else if (strstr(unit, "KiB")) {
                    item->speed = speed_val * 1024;
                }
            }
        }

        // Parse ETA
        char *eta_str = strstr(line, "ETA ");
        if (eta_str) {
            char eta[20];
            if (sscanf(eta_str, "ETA %19s", eta) == 1) {
                g_free(item->eta);
                item->eta = g_strdup(eta);
            }
        }

        return TRUE;
    }

    return FALSE;
}
