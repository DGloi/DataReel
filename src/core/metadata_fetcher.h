#ifndef METADATA_FETCHER_H
#define METADATA_FETCHER_H

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "common.h"

void metadata_fetch_async(const char *url, GCancellable *cancellable,
                         GAsyncReadyCallback callback, gpointer user_data);
VideoMetadata *metadata_fetch_finish(GAsyncResult *result, GError **error);
void metadata_fetch_thread(GTask *task, gpointer source_object,
                          gpointer task_data, GCancellable *cancellable);
void metadata_free(VideoMetadata *meta);

#endif
