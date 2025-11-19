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

// Download quality options
typedef enum {
    QUALITY_BEST,
    QUALITY_1080P,
    QUALITY_720P,
    QUALITY_480P,
    QUALITY_360P,
    QUALITY_AUDIO_ONLY,
    QUALITY_CUSTOM
} VideoQuality;

// Download format
typedef enum {
    FORMAT_MP4,
    FORMAT_WEBM,
    FORMAT_MKV,
    FORMAT_MP3,
    FORMAT_M4A,
    FORMAT_OPUS
} DownloadFormat;

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
    VideoQuality quality;
    DownloadFormat format;
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

// Format information from yt-dlp
typedef struct {
    char *format_id;
    char *format_note;      // e.g., "1080p", "720p60"
    char *ext;              // e.g., "mp4", "webm"
    int width;
    int height;
    int fps;
    int64_t filesize;
    char *vcodec;           // e.g., "avc1", "vp9"
    char *acodec;           // e.g., "mp4a", "opus"
    int tbr;                // Total bitrate
    gboolean has_video;
    gboolean has_audio;
} FormatInfo;

// Video metadata
typedef struct {
    char *title;
    char *uploader;
    char *duration;
    char *thumbnail_url;
    char *description;
    GdkPixbuf *thumbnail_pixbuf;
    int64_t filesize;
    char *format_note;
    GList *formats;         // List of FormatInfo*
    char **available_qualities; // NULL-terminated array of quality strings
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
} DownloadItem;

// yt-dlp version info
typedef struct {
    char *version;
    char *path;
    gboolean is_installed;
} YtdlpInfo;

#endif
