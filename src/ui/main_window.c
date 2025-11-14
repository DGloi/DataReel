#include "main_window.h"
#include "sidebar.h"
#include "download_panel.h"
#include "../downloaders/downloader_interface.h"

typedef struct {
    GtkWidget *sidebar;
    GtkWidget *panel;
} MainWindowData;

static void on_downloader_selected(DownloaderInterface *iface, gpointer user_data) {
    MainWindowData *data = (MainWindowData *)user_data;
    download_panel_set_downloader(data->panel, iface);
}

GtkWidget* main_window_new(GtkApplication *app) {
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *header_bar;
    GtkWidget *paned;

    MainWindowData *data = g_malloc(sizeof(MainWindowData));

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), APP_NAME);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);

    // Create header bar
    header_bar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

    // Create main container
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Create paned layout (sidebar + content)
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_vexpand(paned, TRUE);
    gtk_box_append(GTK_BOX(main_box), paned);

    // Create sidebar
    data->sidebar = sidebar_new(on_downloader_selected, data);
    gtk_paned_set_start_child(GTK_PANED(paned), data->sidebar);
    gtk_paned_set_shrink_start_child(GTK_PANED(paned), FALSE);

    // Create download panel
    data->panel = download_panel_new();
    gtk_paned_set_end_child(GTK_PANED(paned), data->panel);
    gtk_paned_set_shrink_end_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_resize_end_child(GTK_PANED(paned), TRUE);

    // Set initial downloader (YouTube by default)
    DownloaderInterface *default_iface = downloader_get_interface(DOWNLOADER_YOUTUBE);
    if (default_iface) {
        download_panel_set_downloader(data->panel, default_iface);
    }

    g_object_set_data_full(G_OBJECT(window), "window-data", data, g_free);

    return window;
}
