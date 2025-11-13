#include "main_window.h"
#include "../core/downloader.h"

typedef struct {
    GtkWidget *url_entry;
    GtkWidget *path_entry;
    GtkWidget *download_list;
    GtkWidget *start_button;
} MainWindowWidgets;

static void on_folder_selected(GObject *source, GAsyncResult *result, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    MainWindowWidgets *widgets = (MainWindowWidgets *)user_data;
    GFile *folder = gtk_file_dialog_select_folder_finish(dialog, result, NULL);

    if (folder) {
        char *path = g_file_get_path(folder);
        gtk_editable_set_text(GTK_EDITABLE(widgets->path_entry), path);
        g_free(path);
        g_object_unref(folder);
    }
}

static void on_browse_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    MainWindowWidgets *widgets = (MainWindowWidgets *)user_data;
    GtkFileDialog *dialog = gtk_file_dialog_new();

    gtk_file_dialog_set_title(dialog, "Select Download Location");
    gtk_file_dialog_select_folder(dialog,
                                  GTK_WINDOW(gtk_widget_get_root(widgets->path_entry)),
                                  NULL,
                                  on_folder_selected,
                                  widgets);
}

static void on_download_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    MainWindowWidgets *widgets = (MainWindowWidgets *)user_data;

    const char *url = gtk_editable_get_text(GTK_EDITABLE(widgets->url_entry));
    const char *path = gtk_editable_get_text(GTK_EDITABLE(widgets->path_entry));

    if (strlen(url) == 0) {
        g_print("Error: URL is empty\n");
        return;
    }

    DownloadItem *item = downloader_create_item(url, path);
    if (item) {
        g_print("Starting download: %s\n", url);
        downloader_start(item);
        gtk_editable_set_text(GTK_EDITABLE(widgets->url_entry), "");
    }
}

GtkWidget *main_window_new(GtkApplication *app) {
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *header_bar;
    GtkWidget *url_box;
    GtkWidget *path_box;
    GtkWidget *button_box;
    GtkWidget *scrolled_window;
    GtkWidget *label;
    GtkWidget *browse_button;

    MainWindowWidgets *widgets = g_malloc(sizeof(MainWindowWidgets));

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), APP_NAME);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create header bar
    header_bar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

    // Create main vertical box
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(main_box, 12);
    gtk_widget_set_margin_bottom(main_box, 12);
    gtk_widget_set_margin_start(main_box, 12);
    gtk_widget_set_margin_end(main_box, 12);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // URL input section
    url_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    label = gtk_label_new("Download URL:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(url_box), label);

    widgets->url_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->url_entry), "Enter URL here...");
    gtk_box_append(GTK_BOX(url_box), widgets->url_entry);
    gtk_box_append(GTK_BOX(main_box), url_box);

    // Path input section
    path_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    label = gtk_label_new("Download Path:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(path_box), label);

    GtkWidget *path_input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    widgets->path_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widgets->path_entry), g_get_home_dir());
    gtk_widget_set_hexpand(widgets->path_entry, TRUE);
    gtk_box_append(GTK_BOX(path_input_box), widgets->path_entry);

    browse_button = gtk_button_new_with_label("Browse...");
    g_signal_connect(browse_button, "clicked", G_CALLBACK(on_browse_clicked), widgets);
    gtk_box_append(GTK_BOX(path_input_box), browse_button);

    gtk_box_append(GTK_BOX(path_box), path_input_box);
    gtk_box_append(GTK_BOX(main_box), path_box);

    // Download button
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    widgets->start_button = gtk_button_new_with_label("Start Download");
    gtk_widget_add_css_class(widgets->start_button, "suggested-action");
    g_signal_connect(widgets->start_button, "clicked", G_CALLBACK(on_download_clicked), widgets);
    gtk_box_append(GTK_BOX(button_box), widgets->start_button);
    gtk_box_append(GTK_BOX(main_box), button_box);

    // Downloads list section
    label = gtk_label_new("Downloads:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(main_box), label);

    scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    widgets->download_list = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), widgets->download_list);
    gtk_box_append(GTK_BOX(main_box), scrolled_window);

    g_object_set_data_full(G_OBJECT(window), "widgets", widgets, g_free);

    return window;
}
