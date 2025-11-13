#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

typedef struct {
    char *default_download_path;
    int max_concurrent_downloads;
    gboolean auto_start_downloads;
} AppConfig;

AppConfig *config_load(void);
void config_save(AppConfig *config);
void config_free(AppConfig *config);

#endif
