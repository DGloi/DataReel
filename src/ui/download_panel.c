#include "download_panel.h"

typedef struct {
    GtkWidget *url_entry;
    GtkWidget *path_entry;
    GtkWidget *download_list;
    GtkWidget *start_button;
    DownloaderInterface *current_downloader;
} DownloadPanelData;

static void on_folder_selected(GObject *source, GAsyncResult *result, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    DownloadPanelData *data = (DownloadPanelData *)user_data;
    GFile *folder = gtk_file_dialog_select_folder_finish(dialog, result, NULL);

    if (folder) {
        char *path = g_file_get_path(folder);
        gtk_editable_set_text(GTK_EDITABLE(data->path_entry), path);
        g_free(path);
        g_object_unref(folder);
    }
}

static void on_browse_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DownloadPanelData *data = (DownloadPanelData *)user_data;
    GtkFileDialog *dialog = gtk_file_dialog_new();

    gtk_file_dialog_set_title(dialog, "Select Download Location");
    gtk_file_dialog_select_folder(dialog,
                                  GTK_WINDOW(gtk_widget_get_root(data->path_entry)),
                                  NULL,
                                  on_folder_selected,
                                  data);
}

static void on_download_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DownloadPanelData *data = (DownloadPanelData *)user_data;

    if (!data->current_downloader) {
        g_print("Error: No downloader selected\n");
        return;
    }

    const char *url = gtk_editable_get_text(GTK_EDITABLE(data->url_entry));
    const char *path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));

    if (strlen(url) == 0) {
        g_print("Error: URL is empty\n");
        return;
    }

    DownloadItem *item = data->current_downloader->create_item(url, path);
    if (item) {
        g_print("Starting %s download: %s\n", data->current_downloader->name, url);
        data->current_downloader->start(item);
        gtk_editable_set_text(GTK_EDITABLE(data->url_entry), "");
    }
}

GtkWidget* download_panel_new(void) {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(main_box, 12);
    gtk_widget_set_margin_bottom(main_box, 12);
    gtk_widget_set_margin_start(main_box, 12);
    gtk_widget_set_margin_end(main_box, 12);

    DownloadPanelData *data = g_malloc0(sizeof(DownloadPanelData));

    // URL input section
    GtkWidget *url_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *label = gtk_label_new("Download URL:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(url_box), label);

    data->url_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(data->url_entry), "Enter URL here...");
    gtk_box_append(GTK_BOX(url_box), data->url_entry);
    gtk_box_append(GTK_BOX(main_box), url_box);

    // Path input section
    GtkWidget *path_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    label = gtk_label_new("Download Path:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(path_box), label);

    GtkWidget *path_input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    data->path_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(data->path_entry), g_get_home_dir());
    gtk_widget_set_hexpand(data->path_entry, TRUE);
    gtk_box_append(GTK_BOX(path_input_box), data->path_entry);

    GtkWidget *browse_button = gtk_button_new_with_label("Browse...");
    g_signal_connect(browse_button, "clicked", G_CALLBACK(on_browse_clicked), data);
    gtk_box_append(GTK_BOX(path_input_box), browse_button);

    gtk_box_append(GTK_BOX(path_box), path_input_box);
    gtk_box_append(GTK_BOX(main_box), path_box);

    // Download button
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    data->start_button = gtk_button_new_with_label("Start Download");
    gtk_widget_add_css_class(data->start_button, "suggested-action");
    g_signal_connect(data->start_button, "clicked", G_CALLBACK(on_download_clicked), data);
    gtk_box_append(GTK_BOX(button_box), data->start_button);
    gtk_box_append(GTK_BOX(main_box), button_box);

    // Downloads list section
    label = gtk_label_new("Downloads:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(main_box), label);

    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    data->download_list = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), data->download_list);
    gtk_box_append(GTK_BOX(main_box), scrolled_window);

    g_object_set_data_full(G_OBJECT(main_box), "panel-data", data, g_free);

    return main_box;
}

void download_panel_set_downloader(GtkWidget *panel, DownloaderInterface *iface) {
    DownloadPanelData *data = g_object_get_data(G_OBJECT(panel), "panel-data");
    if (data) {
        data->current_downloader = iface;
        g_print("Switched to %s downloader\n", iface->name);
    }
}
