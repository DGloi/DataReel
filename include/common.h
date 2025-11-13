#ifndef COMMON_H
#define COMMON_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Application constants
#define APP_ID "com.example.youtube-dl-gtk"
#define APP_NAME "Download Manager"
#define APP_VERSION "1.0.0"

// Download status enumeration
typedef enum {
    DOWNLOAD_STATUS_IDLE,
    DOWNLOAD_STATUS_QUEUED,
    DOWNLOAD_STATUS_DOWNLOADING,
    DOWNLOAD_STATUS_COMPLETED,
    DOWNLOAD_STATUS_FAILED,
    DOWNLOAD_STATUS_CANCELLED
} DownloadStatus;

// Download item structure
typedef struct {
    char *url;
    char *output_path;
    char *filename;
    DownloadStatus status;
    double progress;
    char *error_message;
    pid_t process_id;
} DownloadItem;

#endif
