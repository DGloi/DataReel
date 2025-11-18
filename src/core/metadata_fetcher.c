#include "metadata_fetcher.h"
#include <json-glib/json-glib.h>

// Fetch video metadata using yt-dlp --dump-json
void metadata_fetch_async(const char *url, MetadataCallback callback, gpointer user_data) {
    GTask *task = g_task_new(NULL, NULL, (GAsyncReadyCallback)callback, user_data);
    g_task_set_task_data(task, g_strdup(url), g_free);
    g_task_run_in_thread(task, metadata_fetch_thread);
    g_object_unref(task);
}

static void metadata_fetch_thread(GTask *task, gpointer source,
                                  gpointer task_data, GCancellable *cancellable) {
    (void)source;
    (void)cancellable;

    const char *url = (const char *)task_data;
    VideoMetadata *meta = g_malloc0(sizeof(VideoMetadata));

    char *output = NULL;
    char *stderr_output = NULL;
    int exit_status;
    GError *error = NULL;

    char *cmd[] = {
        "yt-dlp",
        "--dump-json",
        "--no-playlist",
        (char *)url,
        NULL
    };

    if (g_spawn_sync(NULL, cmd, NULL, G_SPAWN_SEARCH_PATH,
                    NULL, NULL, &output, &stderr_output, &exit_status, &error)) {

        if (exit_status == 0 && output) {
            // Parse JSON
            JsonParser *parser = json_parser_new();
            if (json_parser_load_from_data(parser, output, -1, NULL)) {
                JsonNode *root = json_parser_get_root(parser);
                JsonObject *obj = json_node_get_object(root);

                // Extract metadata
                if (json_object_has_member(obj, "title")) {
                    meta->title = g_strdup(json_object_get_string_member(obj, "title"));
                }
                if (json_object_has_member(obj, "uploader")) {
                    meta->uploader = g_strdup(json_object_get_string_member(obj, "uploader"));
                }
                if (json_object_has_member(obj, "duration")) {
                    int duration = json_object_get_int_member(obj, "duration");
                    meta->duration = g_strdup_printf("%02d:%02d:%02d",
                                                    duration / 3600,
                                                    (duration % 3600) / 60,
                                                    duration % 60);
                }
                if (json_object_has_member(obj, "thumbnail")) {
                    meta->thumbnail_url = g_strdup(json_object_get_string_member(obj, "thumbnail"));
                }
                if (json_object_has_member(obj, "description")) {
                    meta->description = g_strdup(json_object_get_string_member(obj, "description"));
                }
                if (json_object_has_member(obj, "filesize")) {
                    meta->filesize = json_object_get_int_member(obj, "filesize");
                }
                if (json_object_has_member(obj, "format_note")) {
                    meta->format_note = g_strdup(json_object_get_string_member(obj, "format_note"));
                }
            }
            g_object_unref(parser);

            // Download thumbnail
            if (meta->thumbnail_url) {
                GInputStream *stream = NULL;
                GFile *thumb_file = g_file_new_for_uri(meta->thumbnail_url);
                stream = G_INPUT_STREAM(g_file_read(thumb_file, NULL, NULL));

                if (stream) {
                    meta->thumbnail_pixbuf = gdk_pixbuf_new_from_stream(stream, NULL, NULL);
                    g_object_unref(stream);
                }
                g_object_unref(thumb_file);
            }
        }
    }

    g_free(output);
    g_free(stderr_output);

    if (error) {
        g_task_return_error(task, error);
    } else {
        g_task_return_pointer(task, meta, (GDestroyNotify)metadata_free);
    }
}

void metadata_free(VideoMetadata *meta) {
    if (!meta) return;

    g_free(meta->title);
    g_free(meta->uploader);
    g_free(meta->duration);
    g_free(meta->thumbnail_url);
    g_free(meta->description);
    g_free(meta->format_note);

    if (meta->thumbnail_pixbuf) {
        g_object_unref(meta->thumbnail_pixbuf);
    }

    g_free(meta);
}
