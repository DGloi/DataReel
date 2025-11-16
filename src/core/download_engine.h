#ifndef DOWNLOAD_ENGINE_H
#define DOWNLOAD_ENGINE_H

#include "common.h"
#include "metadata_fetcher.h"

DownloadItem* download_item_new(const char *url, const char *output_path,
                                DownloadOptions *opts);
void download_item_free(DownloadItem *item);
gboolean download_item_start(DownloadItem *item);
gboolean download_item_cancel(DownloadItem *item);

#endif
