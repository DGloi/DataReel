#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "common.h"
#include "../downloaders/downloader_interface.h"

typedef void (*SidebarCallback)(DownloaderInterface *iface, gpointer user_data);

GtkWidget* sidebar_new(SidebarCallback callback, gpointer user_data);

#endif
