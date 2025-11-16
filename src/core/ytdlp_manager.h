#ifndef YTDLP_MANAGER_H
#define YTDLP_MANAGER_H

#include "common.h"

YtdlpInfo* ytdlp_get_info(void);
void ytdlp_info_free(YtdlpInfo *info);
gboolean ytdlp_update(GError **error);
char** ytdlp_build_args(const char *url, const char *output_path,
                        DownloadOptions *opts, int *argc);
void ytdlp_free_args(char **args);

#endif
