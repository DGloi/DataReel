#include "main_window.h"
#include "download_options.h"
#include "download_item_widget.h"
#include "settings_panel.h"
#include "../core/download_engine.h"
#include "../core/metadata_fetcher.h"
#include "../utils/string_utils.h"

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
} MainWindowData;

static void on_folder_selected(GObject *source, GAsyncResult *result, gpointer user_data);
static void on_browse_clicked(GtkButton *button, gpointer user_data);
static void on_download_clicked(GtkButton *button, gpointer user_data);
static void on_settings_clicked(GtkButton *button, gpointer user_data);
static void on_url_changed(GtkEditable *editable, gpointer user_data);
static void on_metadata_fetched(VideoMetadata *meta, gpointer user_data);

GtkWidget* main_window_new(GtkApplication *app) {
    MainWindowData *data = g_malloc0(sizeof(MainWindowData));

    // Create main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), APP_NAME);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);

    // Create header bar
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

    // Settings button in header
    GtkWidget *settings_button = gtk_button_new_from_icon_name("preferences-system");
    gtk_widget_set_tooltip_text(settings_button, "Settings");
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_clicked), window);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), settings_button);

    // Main container
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_window_set_child(GTK_WINDOW(window), paned);

    // Left side: Input and options
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_size_request(left_box, 400, -1);
    gtk_widget_set_margin_top(left_box, 12);
    gtk_widget_set_margin_bottom(left_box, 12);
    gtk_widget_set_margin_start(left_box, 12);
    gtk_widget_set_margin_end(left_box, 12);
    gtk_paned_set_start_child(GTK_PANED(paned), left_box);

    // URL input
    GtkWidget *url_frame = gtk_frame_new("Video URL");
    gtk_box_append(GTK_BOX(left_box), url_frame);

    GtkWidget *url_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_top(url_box, 12);
    gtk_widget_set_margin_bottom(url_box, 12);
    gtk_widget_set_margin_start(url_box, 12);
    gtk_widget_set_margin_end(url_box, 12);
    gtk_frame_set_child(GTK_FRAME(url_frame), url_box);

    data->url_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(data->url_entry),
                                   "https://www.youtube.com/watch?v=...");
    g_signal_connect(data->url_entry, "changed", G_CALLBACK(on_url_changed), data);
    gtk_box_append(GTK_BOX(url_box), data->url_entry);

    // Preview area
    data->preview_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_visible(data->preview_box, FALSE);
    gtk_widget_set_margin_top(data->preview_box, 8);
    gtk_box_append(GTK_BOX(url_box), data->preview_box);

    data->preview_thumbnail = gtk_image_new();
    gtk_widget_set_size_request(data->preview_thumbnail, -1, 150);
    gtk_box_append(GTK_BOX(data->preview_box), data->preview_thumbnail);

    data->preview_title = gtk_label_new("");
    gtk_label_set_wrap(GTK_LABEL(data->preview_title), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(data->preview_title), 40);
    gtk_widget_add_css_class(data->preview_title, "title-2");
    gtk_box_append(GTK_BOX(data->preview_box), data->preview_title);

    data->preview_info = gtk_label_new("");
    gtk_widget_add_css_class(data->preview_info, "dim-label");
    gtk_box_append(GTK_BOX(data->preview_box), data->preview_info);

    // Path input
    GtkWidget *path_frame = gtk_frame_new("Download Location");
    gtk_box_append(GTK_BOX(left_box), path_frame);

    GtkWidget *path_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_top(path_box, 12);
    gtk_widget_set_margin_bottom(path_box, 12);
    gtk_widget_set_margin_start(path_box, 12);
    gtk_widget_set_margin_end(path_box, 12);
    gtk_frame_set_child(GTK_FRAME(path_frame), path_box);

    data->path_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(data->path_entry), g_get_home_dir());
    gtk_widget_set_hexpand(data->path_entry, TRUE);
    gtk_box_append(GTK_BOX(path_box), data->path_entry);

    GtkWidget *browse_button = gtk_button_new_with_label("Browse");
    g_signal_connect(browse_button, "clicked", G_CALLBACK(on_browse_clicked), data);
    gtk_box_append(GTK_BOX(path_box), browse_button);

    // Download options
    data->options_panel = download_options_panel_new();
    gtk_widget_set_vexpand(data->options_panel, TRUE);
    gtk_box_append(GTK_BOX(left_box), data->options_panel);

    // Download button
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);

    data->start_button = gtk_button_new_with_label("Start Download");
    gtk_widget_add_css_class(data->start_button, "suggested-action");
    gtk_widget_add_css_class(data->start_button, "pill");
    g_signal_connect(data->start_button, "clicked", G_CALLBACK(on_download_clicked), data);
    gtk_box_append(GTK_BOX(button_box), data->start_button);

    gtk_box_append(GTK_BOX(left_box), button_box);

    // Right side: Downloads list
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_top(right_box, 12);
    gtk_widget_set_margin_bottom(right_box, 12);
    gtk_widget_set_margin_start(right_box, 12);
    gtk_widget_set_margin_end(right_box, 12);
    gtk_paned_set_end_child(GTK_PANED(paned), right_box);

    GtkWidget *downloads_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(downloads_label), "<b>Active Downloads</b>");
    gtk_widget_set_halign(downloads_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(right_box), downloads_label);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);

    data->download_list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(data->download_list), GTK_SELECTION_NONE);
    gtk_widget_add_css_class(data->download_list, "boxed-list");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), data->download_list);
    gtk_box_append(GTK_BOX(right_box), scrolled);

    // Store data
    g_object_set_data_full(G_OBJECT(window), "window-data", data, g_free);

    return window;
}

static void on_folder_selected(GObject *source, GAsyncResult *result, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    MainWindowData *data = (MainWindowData *)user_data;
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
    MainWindowData *data = (MainWindowData *)user_data;
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
    MainWindowData *data = (MainWindowData *)user_data;

    const char *url = gtk_editable_get_text(GTK_EDITABLE(data->url_entry));
    const char *path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));

    if (!string_is_valid_url(url)) {
        g_print("Error: Invalid URL\n");
        return;
    }

    // Get download options
    DownloadOptions *opts = download_options_get(data->options_panel);
    if (!opts) {
        g_print("Error: Failed to get download options\n");
        return;
    }

    // Create download item
    DownloadItem *item = download_item_new(url, path, opts);

    // Copy metadata if already fetched
    VideoMetadata *preview_meta = g_object_get_data(G_OBJECT(data->preview_box), "metadata");
    if (preview_meta) {
        item->metadata = g_malloc0(sizeof(VideoMetadata));
        if (preview_meta->title) item->metadata->title = g_strdup(preview_meta->title);
        if (preview_meta->uploader) item->metadata->uploader = g_strdup(preview_meta->uploader);
        if (preview_meta->duration) item->metadata->duration = g_strdup(preview_meta->duration);
        if (preview_meta->thumbnail_url) item->metadata->thumbnail_url = g_strdup(preview_meta->thumbnail_url);
        item->metadata->filesize = preview_meta->filesize;
        if (preview_meta->thumbnail_pixbuf) {
            item->metadata->thumbnail_pixbuf = g_object_ref(preview_meta->thumbnail_pixbuf);
        }
    }

    // Start download
    if (download_item_start(item)) {
        // Add to downloads list
        GtkWidget *download_widget = download_item_widget_new(item);
        gtk_list_box_append(GTK_LIST_BOX(data->download_list), download_widget);

        data->active_downloads = g_list_append(data->active_downloads, item);

        // Clear URL
        gtk_editable_set_text(GTK_EDITABLE(data->url_entry), "");
        gtk_widget_set_visible(data->preview_box, FALSE);
    }
}

static void on_settings_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkWindow *window = GTK_WINDOW(user_data);
    settings_panel_show(window);
}

static guint url_timeout_id = 0;

static gboolean fetch_metadata_timeout(gpointer user_data) {
    MainWindowData *data = (MainWindowData *)user_data;
    const char *url = gtk_editable_get_text(GTK_EDITABLE(data->url_entry));

    if (string_is_valid_url(url)) {
        gtk_widget_set_visible(data->preview_box, TRUE);
        gtk_label_set_text(GTK_LABEL(data->preview_title), "Fetching video info...");

        metadata_fetch_async(url, on_metadata_fetched, data);
    }

    url_timeout_id = 0;
    return FALSE;
}

static void on_url_changed(GtkEditable *editable, gpointer user_data) {
    (void)editable;

    if (url_timeout_id > 0) {
        g_source_remove(url_timeout_id);
    }

    url_timeout_id = g_timeout_add(1000, fetch_metadata_timeout, user_data);
}

static void on_metadata_fetched(VideoMetadata *meta, gpointer user_data) {
    MainWindowData *data = (MainWindowData *)user_data;

    if (!meta) {
        gtk_widget_set_visible(data->preview_box, FALSE);
        return;
    }

    // Update preview
    if (meta->title) {
        gtk_label_set_text(GTK_LABEL(data->preview_title), meta->title);
    }

    if (meta->thumbnail_pixbuf) {
        GdkPixbuf *scaled = gdk_pixbuf_scale_simple(meta->thumbnail_pixbuf,
                                                    350, 200, GDK_INTERP_BILINEAR);
        GdkTexture *texture = gdk_texture_new_for_pixbuf(scaled);
        gtk_image_set_from_paintable(GTK_IMAGE(data->preview_thumbnail), GDK_PAINTABLE(texture));
        g_object_unref(scaled);
        g_object_unref(texture);
    }

    // Build info text
    GString *info = g_string_new("");
    if (meta->uploader) {
        g_string_append_printf(info, "%s", meta->uploader);
    }
    if (meta->duration) {
        if (info->len > 0) g_string_append(info, " • ");
        g_string_append(info, meta->duration);
    }
    if (meta->filesize > 0) {
        if (info->len > 0) g_string_append(info, " • ");
        char *size = string_format_size(meta->filesize);
        g_string_append(info, size);
        g_free(size);
    }

    gtk_label_set_text(GTK_LABEL(data->preview_info), info->str);
    g_string_free(info, TRUE);

    // Store metadata for later use
    g_object_set_data_full(G_OBJECT(data->preview_box), "metadata", meta,
                          (GDestroyNotify)metadata_free);

    gtk_widget_set_visible(data->preview_box, TRUE);
}
