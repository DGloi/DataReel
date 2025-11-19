#ifndef METADATA_FETCHER_H
#define METADATA_FETCHER_H

#include "common.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

typedef void (*MetadataCallback)(VideoMetadata *metadata, gpointer user_data);

void metadata_fetch_thread(GTask *task, gpointer source_object,
                           gpointer task_data, GCancellable *cancellable);

void metadata_fetch_async(const char *url, MetadataCallback callback, gpointer user_data);
void metadata_free(VideoMetadata *meta);

#endif
