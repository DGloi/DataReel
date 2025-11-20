#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "common.h"

typedef struct {
    GtkWidget *url_entry;
    GtkWidget *path_entry;
    GtkWidget *options_panel;
    GtkWidget *download_list;
    GtkWidget *start_button;
    GtkWidget *preview_box;
    GtkWidget *preview_thumbnail;
    GtkWidget *preview_title;
    GtkWidget *preview_info;
    GList *active_downloads;
    GList *selected_downloads;  // Track selected downloads
    int window_width;  // Track current window width
    int window_height; // Track current window height
} MainWindowData;
GtkWidget *main_window_new(GtkApplication *app);

#endif
