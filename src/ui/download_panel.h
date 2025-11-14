#ifndef DOWNLOAD_PANEL_H
#define DOWNLOAD_PANEL_H

#include "common.h"
#include "../downloaders/downloader_interface.h"

GtkWidget* download_panel_new(void);
void download_panel_set_downloader(GtkWidget *panel, DownloaderInterface *iface);

#endif
