#include "metadata_fetcher.h"
#include <json-glib/json-glib.h>

void metadata_fetch_async(const char *url, GCancellable *cancellable,
                         GAsyncReadyCallback callback, gpointer user_data) {
    GTask *task = g_task_new(NULL, cancellable, callback, user_data);
    g_task_set_task_data(task, g_strdup(url), g_free);
    g_task_run_in_thread(task, metadata_fetch_thread);
}

// Helper function to extract available qualities from formats
static gchar** extract_qualities_from_formats(JsonArray *formats_array) {
    if (!formats_array) return NULL;

    GPtrArray *qualities = g_ptr_array_new_with_free_func(g_free);
    GHashTable *unique_qualities = g_hash_table_new(g_str_hash, g_str_equal);

    guint length = json_array_get_length(formats_array);
    for (guint i = 0; i < length; i++) {
        JsonNode *format_node = json_array_get_element(formats_array, i);
        if (!JSON_NODE_HOLDS_OBJECT(format_node)) continue;

        JsonObject *format_obj = json_node_get_object(format_node);

        // Extract quality information with proper null checks
        const char *format_id = NULL;
        const char *format_note = NULL;
        const char *resolution = NULL;
        const char *ext = NULL;
        gint64 filesize = 0;

        if (json_object_has_member(format_obj, "format_id")) {
            format_id = json_object_get_string_member(format_obj, "format_id");
        }
        if (json_object_has_member(format_obj, "format_note")) {
            format_note = json_object_get_string_member(format_obj, "format_note");
        }
        if (json_object_has_member(format_obj, "resolution")) {
            resolution = json_object_get_string_member(format_obj, "resolution");
        }
        if (json_object_has_member(format_obj, "ext")) {
            ext = json_object_get_string_member(format_obj, "ext");
        }
        if (json_object_has_member(format_obj, "filesize")) {
            filesize = json_object_get_int_member(format_obj, "filesize");
        } else if (json_object_has_member(format_obj, "filesize_approx")) {
            filesize = json_object_get_int_member(format_obj, "filesize_approx");
        }

        // Build quality string - only use meaningful information
        GString *quality_str = g_string_new("");

        // Prioritize format_note (like "1080p", "720p", "best") over other fields
        if (format_note && g_strcmp0(format_note, "") != 0 && g_strcmp0(format_note, "unknown") != 0) {
            g_string_append(quality_str, format_note);
        } else if (resolution && g_strcmp0(resolution, "") != 0 && g_strcmp0(resolution, "audio only") != 0) {
            g_string_append(quality_str, resolution);
        } else if (format_id && g_strcmp0(format_id, "") != 0) {
            g_string_append(quality_str, format_id);
        } else {
            g_string_append(quality_str, "Unknown Format");
        }

        // Add resolution and extension if available and meaningful
        gboolean has_meaningful_resolution = (resolution && g_strcmp0(resolution, "") != 0 &&
                                            g_strcmp0(resolution, "audio only") != 0 &&
                                            g_strcmp0(resolution, "unknown") != 0);
        if (has_meaningful_resolution) {
            g_string_append_printf(quality_str, " (%s", resolution);
            if (ext && g_strcmp0(ext, "") != 0) {
                g_string_append_printf(quality_str, ", %s", ext);
            }
            g_string_append(quality_str, ")");
        } else if (ext && g_strcmp0(ext, "") != 0 && g_strcmp0(ext, "unknown") != 0) {
            g_string_append_printf(quality_str, " (%s)", ext);
        }

        // Add file size if available
        if (filesize > 0) {
            char *size_str = g_format_size(filesize);
            g_string_append_printf(quality_str, " [%s]", size_str);
            g_free(size_str);
        }

        // Add to hash table to avoid duplicates - only if quality string is not empty
        if (quality_str->str && g_strcmp0(quality_str->str, "") != 0 &&
            g_strcmp0(quality_str->str, "Unknown Format") != 0) {
            char *quality = g_strdup(quality_str->str);
            if (!g_hash_table_contains(unique_qualities, quality)) {
                g_hash_table_insert(unique_qualities, quality, quality); // Store key as value
                g_ptr_array_add(qualities, quality);
            } else {
                g_free(quality); // Free if duplicate
            }
        }

        g_string_free(quality_str, TRUE);
    }

    // Add "Best Quality" option at the beginning if we have any formats
    if (qualities->len > 0) {
        g_ptr_array_insert(qualities, 0, g_strdup("Best Quality"));
    }

    // Convert to NULL-terminated array
    g_ptr_array_add(qualities, NULL);
    gchar **result = (gchar **)g_ptr_array_free(qualities, FALSE);

    g_hash_table_destroy(unique_qualities);
    return result;
}

void metadata_fetch_thread(GTask *task, gpointer source_object,
                          gpointer task_data, GCancellable *cancellable) {
    (void)source_object;
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

    gboolean success = g_spawn_sync(NULL, cmd, NULL, G_SPAWN_SEARCH_PATH,
                    NULL, NULL, &output, &stderr_output, &exit_status, &error);

    if (success && exit_status == 0 && output) {
        // Parse JSON with error handling
        JsonParser *parser = json_parser_new();
        GError *parse_error = NULL;

        if (json_parser_load_from_data(parser, output, -1, &parse_error)) {
            JsonNode *root = json_parser_get_root(parser);
            if (root && JSON_NODE_HOLDS_OBJECT(root)) {
                JsonObject *obj = json_node_get_object(root);

                // Extract metadata with proper null checks
                if (json_object_has_member(obj, "title")) {
                    const char *title = json_object_get_string_member(obj, "title");
                    if (title && g_utf8_validate(title, -1, NULL)) {
                        meta->title = g_strdup(title);
                    } else if (title) {
                        // Sanitize non-UTF-8 strings
                        char *utf8_str = g_locale_to_utf8(title, -1, NULL, NULL, NULL);
                        meta->title = utf8_str ? utf8_str : g_strdup(title);
                    }
                }
                if (json_object_has_member(obj, "uploader")) {
                    const char *uploader = json_object_get_string_member(obj, "uploader");
                    if (uploader && g_utf8_validate(uploader, -1, NULL)) {
                        meta->uploader = g_strdup(uploader);
                    } else if (uploader) {
                        char *utf8_str = g_locale_to_utf8(uploader, -1, NULL, NULL, NULL);
                        meta->uploader = utf8_str ? utf8_str : g_strdup(uploader);
                    }
                }
                if (json_object_has_member(obj, "duration")) {
                    int duration = json_object_get_int_member(obj, "duration");
                    meta->duration = g_strdup_printf("%02d:%02d:%02d",
                                                    duration / 3600,
                                                    (duration % 3600) / 60,
                                                    duration % 60);
                }
                if (json_object_has_member(obj, "thumbnail")) {
                    const char *thumb = json_object_get_string_member(obj, "thumbnail");
                    if (thumb && g_utf8_validate(thumb, -1, NULL)) {
                        meta->thumbnail_url = g_strdup(thumb);
                    } else if (thumb) {
                        char *utf8_str = g_locale_to_utf8(thumb, -1, NULL, NULL, NULL);
                        meta->thumbnail_url = utf8_str ? utf8_str : g_strdup(thumb);
                    }
                }
                if (json_object_has_member(obj, "description")) {
                    const char *desc = json_object_get_string_member(obj, "description");
                    if (desc && g_utf8_validate(desc, -1, NULL)) {
                        meta->description = g_strdup(desc);
                    } else if (desc) {
                        char *utf8_str = g_locale_to_utf8(desc, -1, NULL, NULL, NULL);
                        meta->description = utf8_str ? utf8_str : g_strdup(desc);
                    }
                }
                if (json_object_has_member(obj, "filesize")) {
                    meta->filesize = json_object_get_int_member(obj, "filesize");
                } else if (json_object_has_member(obj, "filesize_approx")) {
                    meta->filesize = json_object_get_int_member(obj, "filesize_approx");
                }
                if (json_object_has_member(obj, "format_note")) {
                    const char *note = json_object_get_string_member(obj, "format_note");
                    if (note && g_utf8_validate(note, -1, NULL)) {
                        meta->format_note = g_strdup(note);
                    } else if (note) {
                        char *utf8_str = g_locale_to_utf8(note, -1, NULL, NULL, NULL);
                        meta->format_note = utf8_str ? utf8_str : g_strdup(note);
                    }
                }

                // Extract formats array
                if (json_object_has_member(obj, "formats")) {
                    JsonArray *formats_array = json_object_get_array_member(obj, "formats");
                    meta->available_qualities = extract_qualities_from_formats(formats_array);
                }
            }
        }

        if (parse_error) {
            g_warning("JSON parse error: %s", parse_error->message);
            g_error_free(parse_error);
        }

        g_object_unref(parser);

        // Download thumbnail asynchronously
        if (meta->thumbnail_url) {
            GError *thumb_error = NULL;
            GFile *thumb_file = g_file_new_for_uri(meta->thumbnail_url);
            GFileInputStream *stream = g_file_read(thumb_file, NULL, &thumb_error);

            if (stream) {
                meta->thumbnail_pixbuf = gdk_pixbuf_new_from_stream(
                    G_INPUT_STREAM(stream), NULL, NULL);
                g_object_unref(stream);
            } else if (thumb_error) {
                g_warning("Thumbnail download failed: %s", thumb_error->message);
                g_error_free(thumb_error);
            }
            g_object_unref(thumb_file);
        }
    } else {
        g_warning("yt-dlp metadata fetch failed: %s",
                 stderr_output ? stderr_output : "Unknown error");
    }

    g_free(output);
    g_free(stderr_output);

    if (error) {
        g_task_return_error(task, error);
    } else {
        g_task_return_pointer(task, meta, (GDestroyNotify)metadata_free);
    }
}

VideoMetadata *metadata_fetch_finish(GAsyncResult *result, GError **error) {
    g_return_val_if_fail(g_task_is_valid(result, NULL), NULL);
    return g_task_propagate_pointer(G_TASK(result), error);
}

void metadata_free(VideoMetadata *meta) {
    if (!meta) return;

    g_free(meta->title);
    g_free(meta->uploader);
    g_free(meta->duration);
    g_free(meta->thumbnail_url);
    g_free(meta->description);
    g_free(meta->format_note);

    // Free available qualities array
    if (meta->available_qualities) {
        g_strfreev(meta->available_qualities);
    }

    if (meta->thumbnail_pixbuf) {
        g_object_unref(meta->thumbnail_pixbuf);
    }

    g_free(meta);
}
