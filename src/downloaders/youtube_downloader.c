#include "youtube_downloader.h"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

static DownloadItem* youtube_create_item(const char *url, const char *output_path) {
    if (!url || strlen(url) == 0) {
        return NULL;
    }

    DownloadItem *item = g_malloc0(sizeof(DownloadItem));
    item->url = g_strdup(url);
    item->output_path = g_strdup(output_path ? output_path : g_get_home_dir());
    item->status = DOWNLOAD_STATUS_IDLE;
    item->progress = 0.0;
    item->process_id = -1;
    item->type = DOWNLOADER_YOUTUBE;

    return item;
}

static gboolean youtube_start(DownloadItem *item) {
    if (!item || item->status == DOWNLOAD_STATUS_DOWNLOADING) {
        return FALSE;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // Child process - execute yt-dlp
        execlp("yt-dlp", "yt-dlp",
               "-o", g_strdup_printf("%s/%%(title)s.%%(ext)s", item->output_path),
               item->url,
               NULL);

        // If execlp fails
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        item->process_id = pid;
        item->status = DOWNLOAD_STATUS_DOWNLOADING;
        g_print("YouTube download started with PID: %d\n", pid);
        return TRUE;
    } else {
        // Fork failed
        item->status = DOWNLOAD_STATUS_FAILED;
        item->error_message = g_strdup("Failed to start download process");
        return FALSE;
    }
}

static gboolean youtube_cancel(DownloadItem *item) {
    if (!item || item->process_id <= 0) {
        return FALSE;
    }

    if (kill(item->process_id, SIGTERM) == 0) {
        item->status = DOWNLOAD_STATUS_CANCELLED;
        return TRUE;
    }

    return FALSE;
}

static DownloadStatus youtube_get_status(DownloadItem *item) {
    if (!item) return DOWNLOAD_STATUS_IDLE;

    if (item->process_id > 0 && item->status == DOWNLOAD_STATUS_DOWNLOADING) {
        int status;
        pid_t result = waitpid(item->process_id, &status, WNOHANG);

        if (result == 0) {
            return DOWNLOAD_STATUS_DOWNLOADING;
        } else if (result == item->process_id) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                item->status = DOWNLOAD_STATUS_COMPLETED;
            } else {
                item->status = DOWNLOAD_STATUS_FAILED;
            }
        }
    }

    return item->status;
}

static void youtube_free_item(DownloadItem *item) {
    if (!item) return;

    g_free(item->url);
    g_free(item->output_path);
    g_free(item->filename);
    g_free(item->error_message);
    g_free(item);
}

DownloaderInterface* youtube_downloader_get_interface(void) {
    static DownloaderInterface interface = {
        .name = "YouTube",
        .icon_name = "video-display",
        .type = DOWNLOADER_YOUTUBE,
        .create_item = youtube_create_item,
        .start = youtube_start,
        .cancel = youtube_cancel,
        .get_status = youtube_get_status,
        .free_item = youtube_free_item
    };

    return &interface;
}
