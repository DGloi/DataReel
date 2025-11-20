#ifndef DOWNLOAD_ITEM_WIDGET_H
#define DOWNLOAD_ITEM_WIDGET_H

#include "common.h"
#include "../core/download_engine.h"

GtkWidget* download_item_widget_new(DownloadItem *item);
void download_item_widget_update(GtkWidget *widget, DownloadItem *item);
DownloadItem* download_item_widget_get_item(GtkWidget *widget);

#endif
