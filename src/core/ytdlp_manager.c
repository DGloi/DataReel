#include "ytdlp_manager.h"
#include <gio/gio.h>

// Check if yt-dlp is installed and get version
YtdlpInfo* ytdlp_get_info(void) {
    YtdlpInfo *info = g_malloc0(sizeof(YtdlpInfo));

    // Check if yt-dlp exists in PATH
    char *path = g_find_program_in_path("yt-dlp");
    if (path) {
        info->path = path;
        info->is_installed = TRUE;

        // Get version
        char *output = NULL;
        char *cmd[] = {"yt-dlp", "--version", NULL};

        if (g_spawn_sync(NULL, cmd, NULL, G_SPAWN_SEARCH_PATH,
                        NULL, NULL, &output, NULL, NULL, NULL)) {
            if (output) {
                g_strchomp(output);
                info->version = output;
            }
        }
    } else {
        info->is_installed = FALSE;
    }

    return info;
}

void ytdlp_info_free(YtdlpInfo *info) {
    if (!info) return;
    g_free(info->version);
    g_free(info->path);
    g_free(info);
}

// Update yt-dlp to latest version
gboolean ytdlp_update(GError **error) {
    char *cmd[] = {"yt-dlp", "-U", NULL};
    int exit_status;

    if (!g_spawn_sync(NULL, cmd, NULL, G_SPAWN_SEARCH_PATH,
                     NULL, NULL, NULL, NULL, &exit_status, error)) {
        return FALSE;
    }

    return exit_status == 0;
}

// Build command line arguments from DownloadOptions
char** ytdlp_build_args(const char *url, const char *output_path,
                        DownloadOptions *opts, int *argc) {
    GPtrArray *args = g_ptr_array_new();

    g_ptr_array_add(args, g_strdup("yt-dlp"));

    // Audio only
    if (opts->audio_only) {
        g_ptr_array_add(args, g_strdup("-x"));
    }

    // Subtitles
    if (opts->subtitles) {
        g_ptr_array_add(args, g_strdup("--write-subs"));
        g_ptr_array_add(args, g_strdup("--write-auto-subs"));
        g_ptr_array_add(args, g_strdup("--embed-subs"));
    }

    // Thumbnail
    if (opts->embed_thumbnail) {
        g_ptr_array_add(args, g_strdup("--embed-thumbnail"));
    }

    // Playlist handling
    if (!opts->playlist) {
        g_ptr_array_add(args, g_strdup("--no-playlist"));
    } else if (opts->max_downloads > 0) {
        g_ptr_array_add(args, g_strdup("--max-downloads"));
        g_ptr_array_add(args, g_strdup_printf("%d", opts->max_downloads));
    }

    // Time range
    if (opts->time_range_start && strlen(opts->time_range_start) > 0) {
        g_ptr_array_add(args, g_strdup("--download-sections"));
        char *range = g_strdup_printf("*%s-%s",
                                     opts->time_range_start,
                                     opts->time_range_end ? opts->time_range_end : "inf");
        g_ptr_array_add(args, range);
    }

    // Progress output
    g_ptr_array_add(args, g_strdup("--newline"));
    g_ptr_array_add(args, g_strdup("--progress"));

    // Output path
    g_ptr_array_add(args, g_strdup("-o"));
    if (opts->output_template) {
        g_ptr_array_add(args, g_strdup_printf("%s/%s", output_path, opts->output_template));
    } else {
        g_ptr_array_add(args, g_strdup_printf("%s/%%(title)s.%%(ext)s", output_path));
    }

    // URL (must be last)
    g_ptr_array_add(args, g_strdup(url));
    g_ptr_array_add(args, NULL);

    *argc = args->len - 1;
    return (char **)g_ptr_array_free(args, FALSE);
}

void ytdlp_free_args(char **args) {
    if (!args) return;
    for (int i = 0; args[i] != NULL; i++) {
        g_free(args[i]);
    }
    g_free(args);
}
