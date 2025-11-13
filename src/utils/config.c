#include "config.h"

AppConfig *config_load(void) {
    AppConfig *config = g_malloc0(sizeof(AppConfig));

    // Set defaults
    config->default_download_path = g_strdup(g_get_home_dir());
    config->max_concurrent_downloads = 3;
    config->auto_start_downloads = FALSE;

    // TODO: Load from config file (e.g., ~/.config/youtube-dl-gtk/config.ini)

    return config;
}

void config_save(AppConfig *config) {
    if (!config) return;

    // TODO: Save to config file
}

void config_free(AppConfig *config) {
    if (!config) return;

    g_free(config->default_download_path);
    g_free(config);
}
