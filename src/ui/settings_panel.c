#include "settings_panel.h"
#include "../core/ytdlp_manager.h"

typedef struct {
    GtkWidget *version_label;
    GtkWidget *path_label;
    GtkWidget *update_button;
    GtkWidget *status_label;
} SettingsPanelData;

static void on_update_clicked(GtkButton *button, gpointer user_data);
static void on_check_version_clicked(GtkButton *button, gpointer user_data);

GtkWidget* settings_panel_new(void) {
    GtkWidget *window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Settings");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Header bar
    GtkWidget *header = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    // Content
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(content, 24);
    gtk_widget_set_margin_bottom(content, 24);
    gtk_widget_set_margin_start(content, 24);
    gtk_widget_set_margin_end(content, 24);
    gtk_box_append(GTK_BOX(main_box), content);

    SettingsPanelData *data = g_malloc0(sizeof(SettingsPanelData));

    // yt-dlp Section
    GtkWidget *ytdlp_frame = gtk_frame_new("yt-dlp Configuration");
    gtk_box_append(GTK_BOX(content), ytdlp_frame);

    GtkWidget *ytdlp_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(ytdlp_box, 12);
    gtk_widget_set_margin_bottom(ytdlp_box, 12);
    gtk_widget_set_margin_start(ytdlp_box, 12);
    gtk_widget_set_margin_end(ytdlp_box, 12);
    gtk_frame_set_child(GTK_FRAME(ytdlp_frame), ytdlp_box);

    // Version info
    GtkWidget *version_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *version_title = gtk_label_new("Version:");
    gtk_widget_set_halign(version_title, GTK_ALIGN_START);
    gtk_label_set_width_chars(GTK_LABEL(version_title), 15);
    gtk_box_append(GTK_BOX(version_box), version_title);

    data->version_label = gtk_label_new("Checking...");
    gtk_widget_set_halign(data->version_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(data->version_label, TRUE);
    gtk_box_append(GTK_BOX(version_box), data->version_label);

    gtk_box_append(GTK_BOX(ytdlp_box), version_box);

    // Path info
    GtkWidget *path_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *path_title = gtk_label_new("Installation Path:");
    gtk_widget_set_halign(path_title, GTK_ALIGN_START);
    gtk_label_set_width_chars(GTK_LABEL(path_title), 15);
    gtk_box_append(GTK_BOX(path_box), path_title);

    data->path_label = gtk_label_new("N/A");
    gtk_widget_set_halign(data->path_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(data->path_label, TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(data->path_label), PANGO_ELLIPSIZE_MIDDLE);
    gtk_box_append(GTK_BOX(path_box), data->path_label);

    gtk_box_append(GTK_BOX(ytdlp_box), path_box);

    // Action buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);

    GtkWidget *check_button = gtk_button_new_with_label("Check Version");
    g_signal_connect(check_button, "clicked", G_CALLBACK(on_check_version_clicked), data);
    gtk_box_append(GTK_BOX(button_box), check_button);

    data->update_button = gtk_button_new_with_label("Update yt-dlp");
    gtk_widget_add_css_class(data->update_button, "suggested-action");
    g_signal_connect(data->update_button, "clicked", G_CALLBACK(on_update_clicked), data);
    gtk_box_append(GTK_BOX(button_box), data->update_button);

    gtk_box_append(GTK_BOX(ytdlp_box), button_box);

    // Status label
    data->status_label = gtk_label_new("");
    gtk_widget_set_halign(data->status_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(data->status_label, "dim-label");
    gtk_box_append(GTK_BOX(ytdlp_box), data->status_label);

    // Download Settings Section
    GtkWidget *download_frame = gtk_frame_new("Download Settings");
    gtk_box_append(GTK_BOX(content), download_frame);

    GtkWidget *download_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(download_box, 12);
    gtk_widget_set_margin_bottom(download_box, 12);
    gtk_widget_set_margin_start(download_box, 12);
    gtk_widget_set_margin_end(download_box, 12);
    gtk_frame_set_child(GTK_FRAME(download_frame), download_box);

    // Max concurrent downloads
    GtkWidget *concurrent_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *concurrent_label = gtk_label_new("Max Concurrent Downloads:");
    gtk_widget_set_halign(concurrent_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(concurrent_label, TRUE);
    gtk_box_append(GTK_BOX(concurrent_box), concurrent_label);

    GtkWidget *concurrent_spin = gtk_spin_button_new_with_range(1, 10, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(concurrent_spin), 3);
    gtk_box_append(GTK_BOX(concurrent_box), concurrent_spin);

    gtk_box_append(GTK_BOX(download_box), concurrent_box);

    // Auto-update check
    GtkWidget *auto_update = gtk_check_button_new_with_label(
        "Automatically check for yt-dlp updates on startup");
    gtk_box_append(GTK_BOX(download_box), auto_update);

    // Store data
    g_object_set_data_full(G_OBJECT(window), "settings-data", data, g_free);

    // Initial version check
    on_check_version_clicked(NULL, data);

    return window;
}

static void on_check_version_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    SettingsPanelData *data = (SettingsPanelData *)user_data;

    gtk_label_set_text(GTK_LABEL(data->version_label), "Checking...");
    gtk_label_set_text(GTK_LABEL(data->status_label), "");

    YtdlpInfo *info = ytdlp_get_info();

    if (info->is_installed) {
        gtk_label_set_text(GTK_LABEL(data->version_label),
                          info->version ? info->version : "Unknown");
        gtk_label_set_text(GTK_LABEL(data->path_label),
                          info->path ? info->path : "N/A");
        gtk_widget_set_sensitive(data->update_button, TRUE);
        gtk_label_set_text(GTK_LABEL(data->status_label),
                          "✓ yt-dlp is installed and ready");
    } else {
        gtk_label_set_text(GTK_LABEL(data->version_label), "Not Installed");
        gtk_label_set_text(GTK_LABEL(data->path_label), "N/A");
        gtk_widget_set_sensitive(data->update_button, FALSE);

        char *markup = g_markup_printf_escaped(
            "⚠ yt-dlp is not installed. Install it with:\n"
            "<tt>pip install yt-dlp</tt> or <tt>sudo apt install yt-dlp</tt>");
        gtk_label_set_markup(GTK_LABEL(data->status_label), markup);
        g_free(markup);
    }

    ytdlp_info_free(info);
}

static void on_update_clicked(GtkButton *button, gpointer user_data) {
    SettingsPanelData *data = (SettingsPanelData *)user_data;

    gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    gtk_label_set_text(GTK_LABEL(data->status_label), "Updating yt-dlp...");

    // Run update in background (simplified - should use async)
    GError *error = NULL;
    gboolean success = ytdlp_update(&error);

    if (success) {
        gtk_label_set_text(GTK_LABEL(data->status_label),
                          "✓ yt-dlp updated successfully");
        on_check_version_clicked(NULL, data);
    } else {
        char *error_msg = g_strdup_printf("✗ Update failed: %s",
                                         error ? error->message : "Unknown error");
        gtk_label_set_text(GTK_LABEL(data->status_label), error_msg);
        g_free(error_msg);
        if (error) g_error_free(error);
    }

    gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);
}

void settings_panel_show(GtkWindow *parent) {
    GtkWidget *settings = settings_panel_new();
    gtk_window_set_transient_for(GTK_WINDOW(settings), parent);
    gtk_window_present(GTK_WINDOW(settings));
}
