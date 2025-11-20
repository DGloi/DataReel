#ifndef COMMON_H
#define COMMON_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// Application constants
#define APP_ID "com.datareel.ytdlp"
#define APP_NAME "DataReel - yt-dlp Manager"
#define APP_VERSION "1.0.0"

// Download status
typedef enum {
    DOWNLOAD_STATUS_IDLE,
    DOWNLOAD_STATUS_FETCHING_INFO,
    DOWNLOAD_STATUS_QUEUED,
    DOWNLOAD_STATUS_DOWNLOADING,
    DOWNLOAD_STATUS_PROCESSING,
    DOWNLOAD_STATUS_COMPLETED,
    DOWNLOAD_STATUS_FAILED,
    DOWNLOAD_STATUS_CANCELLED
} DownloadStatus;

// Download options structure
typedef struct {
    gboolean audio_only;
    gboolean subtitles;
    gboolean embed_thumbnail;
    gboolean playlist;
    char *custom_format;
    char *time_range_start;  // e.g., "00:01:30"
    char *time_range_end;    // e.g., "00:05:00"
    int max_downloads;       // For playlists
    char *output_template;
} DownloadOptions;

// Video metadata
typedef struct {
    char *title;
    char *uploader;
    char *duration;
    char *thumbnail_url;
    char *description;
    GdkPixbuf *thumbnail_pixbuf;
    gint64 filesize;
    char *format_note;
    char **available_qualities; // NULL-terminated array of quality strings for UI
} VideoMetadata;

// Download item
typedef struct {
    char *url;
    char *output_path;
    DownloadOptions *options;
    VideoMetadata *metadata;
    DownloadStatus status;
    double progress;
    double speed;           // bytes per second
    char *eta;
    char *error_message;
    pid_t process_id;
    int read_fd;           // For reading stdout/stderr
    GIOChannel *io_channel;
    guint io_watch_id;
    gpointer user_data;    // For UI widget reference
} DownloadItem;

// yt-dlp version info
typedef struct {
    char *version;
    char *path;
    gboolean is_installed;
} YtdlpInfo;

#endif
