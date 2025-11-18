#ifndef METADATA_FETCHER_H
#define METADATA_FETCHER_H

#include "common.h"

typedef void (*MetadataCallback)(VideoMetadata *meta, gpointer user_data);

void metadata_fetch_async(const char *url, MetadataCallback callback, gpointer user_data);
static void metadata_fetch_thread(GTask *task, gpointer source,
                                 gpointer task_data, GCancellable *cancellable);
void metadata_free(VideoMetadata *meta);

#endif
