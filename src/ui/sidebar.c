#include "sidebar.h"
#include "../downloaders/downloader_interface.h"

typedef struct {
    GtkWidget *list_box;
    SidebarCallback callback;
    gpointer user_data;
} SidebarData;

static void on_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)box;
    SidebarData *data = (SidebarData *)user_data;

    DownloaderInterface *iface = g_object_get_data(G_OBJECT(row), "downloader-interface");
    if (iface && data->callback) {
        data->callback(iface, data->user_data);
    }
}

GtkWidget* sidebar_new(SidebarCallback callback, gpointer user_data) {
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled_window, 200, -1);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);

    SidebarData *data = g_malloc(sizeof(SidebarData));
    data->callback = callback;
    data->user_data = user_data;

    GtkWidget *list_box = gtk_list_box_new();
    data->list_box = list_box;
    gtk_widget_add_css_class(list_box, "navigation-sidebar");

    g_signal_connect(list_box, "row-activated", G_CALLBACK(on_row_activated), data);

    // Add downloader entries
    GList *interfaces = downloader_get_all_interfaces();
    for (GList *l = interfaces; l != NULL; l = l->next) {
        DownloaderInterface *iface = (DownloaderInterface *)l->data;

        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_set_margin_top(box, 8);
        gtk_widget_set_margin_bottom(box, 8);
        gtk_widget_set_margin_start(box, 12);
        gtk_widget_set_margin_end(box, 12);

        // Icon
        GtkWidget *icon = gtk_image_new_from_icon_name(iface->icon_name);
        gtk_image_set_icon_size(GTK_IMAGE(icon), GTK_ICON_SIZE_NORMAL);
        gtk_box_append(GTK_BOX(box), icon);

        // Label
        GtkWidget *label = gtk_label_new(iface->name);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(box), label);

        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
        g_object_set_data(G_OBJECT(row), "downloader-interface", iface);

        gtk_list_box_append(GTK_LIST_BOX(list_box), row);
    }

    // Select first row by default
    if (interfaces) {
        GtkListBoxRow *first_row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list_box), 0);
        if (first_row) {
            gtk_list_box_select_row(GTK_LIST_BOX(list_box), first_row);
        }
    }

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), list_box);
    g_object_set_data_full(G_OBJECT(scrolled_window), "sidebar-data", data, g_free);

    return scrolled_window;
}
