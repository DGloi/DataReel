#ifndef DOWNLOAD_OPTIONS_H
#define DOWNLOAD_OPTIONS_H

#include "common.h"

GtkWidget* download_options_panel_new(void);
DownloadOptions* download_options_get(GtkWidget *panel);

#endif
