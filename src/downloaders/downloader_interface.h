#ifndef DOWNLOADER_INTERFACE_H
#define DOWNLOADER_INTERFACE_H

#include "common.h"

// Common interface for all downloaders
typedef struct {
    const char *name;
    const char *icon_name;
    DownloaderType type;

    // Function pointers
    DownloadItem* (*create_item)(const char *url, const char *output_path);
    gboolean (*start)(DownloadItem *item);
    gboolean (*cancel)(DownloadItem *item);
    DownloadStatus (*get_status)(DownloadItem *item);
    void (*free_item)(DownloadItem *item);
} DownloaderInterface;

// Registry functions
void downloader_register_all(void);
DownloaderInterface* downloader_get_interface(DownloaderType type);
GList* downloader_get_all_interfaces(void);

#endif
