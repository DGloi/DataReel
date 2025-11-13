#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "common.h"
#include <signal.h>

DownloadItem *downloader_create_item(const char *url, const char *output_path);
void downloader_free_item(DownloadItem *item);
gboolean downloader_start(DownloadItem *item);
gboolean downloader_cancel(DownloadItem *item);
DownloadStatus downloader_get_status(DownloadItem *item);

#endif
