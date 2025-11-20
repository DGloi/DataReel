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
    item->speed = 0;

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

    if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL)) {
        // Check if process finished successfully
        int status;
        pid_t result = waitpid(item->process_id, &status, WNOHANG);

        if (result == item->process_id) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                item->status = DOWNLOAD_STATUS_COMPLETED;
                item->progress = 100.0;
            } else {
                item->status = DOWNLOAD_STATUS_FAILED;
                item->error_message = g_strdup("Download process failed");
            }
        }
        return FALSE; // Remove the watch
    }

    if (cond & G_IO_IN) {
        // Use a different approach for reading from the pipe
        char buffer[4096];
        gsize bytes_read;
        GError *error = NULL;

        GIOStatus status = g_io_channel_read_chars(channel, buffer, sizeof(buffer) - 1, &bytes_read, &error);

        if (status == G_IO_STATUS_NORMAL && bytes_read > 0) {
            buffer[bytes_read] = '\0';

            // Split the buffer into lines and process each
            char *line_start = buffer;
            char *newline;

            while ((newline = strchr(line_start, '\n')) != NULL) {
                *newline = '\0';

                // Process the line
                if (parse_progress_line(line_start, item)) {
                    // Progress was parsed successfully
                }

                line_start = newline + 1;
            }

            // Process any remaining partial line (if the buffer didn't end with \n)
            if (*line_start != '\0') {
                if (parse_progress_line(line_start, item)) {
                    // Progress was parsed successfully
                }
            }
        } else if (status == G_IO_STATUS_EOF) {
            item->status = DOWNLOAD_STATUS_COMPLETED;
            item->progress = 100.0;
            return FALSE; // Remove the watch
        }

        if (error) {
            g_warning("IO error: %s", error->message);
            g_error_free(error);
            return FALSE; // Remove the watch
        }
    }

    return TRUE; // Keep the watch
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

        // Set the encoding to NULL (raw binary) - this is important for pipes
        g_io_channel_set_encoding(item->io_channel, NULL, NULL);
        g_io_channel_set_buffered(item->io_channel, FALSE);

        item->io_watch_id = g_io_add_watch(item->io_channel,
                                          G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
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

gboolean download_item_pause(DownloadItem *item) {
    if (!item || item->process_id <= 0 || item->status != DOWNLOAD_STATUS_DOWNLOADING) {
        return FALSE;
    }

    if (kill(item->process_id, SIGSTOP) == 0) {
        item->status = DOWNLOAD_STATUS_QUEUED; // Use QUEUED to represent paused
        return TRUE;
    }

    return FALSE;
}

gboolean download_item_resume(DownloadItem *item) {
    if (!item || item->process_id <= 0 || item->status != DOWNLOAD_STATUS_QUEUED) {
        return FALSE;
    }

    if (kill(item->process_id, SIGCONT) == 0) {
        item->status = DOWNLOAD_STATUS_DOWNLOADING;
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
