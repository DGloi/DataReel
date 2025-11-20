#ifndef DOWNLOAD_ENGINE_H
#define DOWNLOAD_ENGINE_H

#include "common.h"
#include "metadata_fetcher.h"

gboolean download_item_start(DownloadItem *item);
gboolean download_item_cancel(DownloadItem *item);
gboolean download_item_pause(DownloadItem *item);
gboolean download_item_resume(DownloadItem *item);
DownloadItem* download_item_new(const char *url, const char *output_path,
                                DownloadOptions *opts);
void download_item_free(DownloadItem *item);

#endif
