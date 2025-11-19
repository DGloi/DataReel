#include "download_options.h"

typedef struct {
    GtkWidget *quality_combo;
    GtkWidget *format_combo;
    GtkWidget *audio_only_check;
    GtkWidget *subtitles_check;
    GtkWidget *thumbnail_check;
    GtkWidget *playlist_check;
    GtkWidget *time_start_entry;
    GtkWidget *time_end_entry;
    GtkWidget *custom_format_entry;
} DownloadOptionsWidgets;

GtkWidget* download_options_panel_new(void) {
    GtkWidget *frame = gtk_frame_new("Download Options");
    gtk_widget_set_margin_top(frame, 6);
    gtk_widget_set_margin_bottom(frame, 6);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_widget_set_margin_top(grid, 12);
    gtk_widget_set_margin_bottom(grid, 12);
    gtk_widget_set_margin_start(grid, 12);
    gtk_widget_set_margin_end(grid, 12);
    gtk_frame_set_child(GTK_FRAME(frame), grid);

    DownloadOptionsWidgets *widgets = g_malloc0(sizeof(DownloadOptionsWidgets));
    int row = 0;

    // Quality selector (initially shows placeholder)
    GtkWidget *label = gtk_label_new("Quality:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);

    widgets->quality_combo = gtk_combo_box_text_new();
    // Show placeholder until metadata is loaded
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->quality_combo), "Enter URL to load options...");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->quality_combo), 0);
    gtk_widget_set_sensitive(widgets->quality_combo, FALSE);
    gtk_widget_set_hexpand(widgets->quality_combo, TRUE);
    gtk_grid_attach(GTK_GRID(grid), widgets->quality_combo, 1, row++, 1, 1);

    // Format selector
    label = gtk_label_new("Format:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);

    widgets->format_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->format_combo), "MP4");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->format_combo), "WebM");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->format_combo), "MKV");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->format_combo), "MP3");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->format_combo), "M4A");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->format_combo), "Opus");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->format_combo), 0);
    gtk_widget_set_hexpand(widgets->format_combo, TRUE);
    gtk_grid_attach(GTK_GRID(grid), widgets->format_combo, 1, row++, 1, 1);

    // Custom format (hidden by default)
    label = gtk_label_new("Custom Format:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);

    widgets->custom_format_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->custom_format_entry), "e.g., bestvideo+bestaudio");
    gtk_widget_set_hexpand(widgets->custom_format_entry, TRUE);
    gtk_widget_set_visible(widgets->custom_format_entry, FALSE);
    gtk_widget_set_visible(label, FALSE);
    gtk_grid_attach(GTK_GRID(grid), widgets->custom_format_entry, 1, row++, 1, 1);

    // Time range
    label = gtk_label_new("Time Range:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);

    GtkWidget *time_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    widgets->time_start_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->time_start_entry), "00:00:00");
    gtk_widget_set_hexpand(widgets->time_start_entry, TRUE);
    gtk_box_append(GTK_BOX(time_box), widgets->time_start_entry);

    gtk_box_append(GTK_BOX(time_box), gtk_label_new(" to "));

    widgets->time_end_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->time_end_entry), "00:00:00");
    gtk_widget_set_hexpand(widgets->time_end_entry, TRUE);
    gtk_box_append(GTK_BOX(time_box), widgets->time_end_entry);

    gtk_grid_attach(GTK_GRID(grid), time_box, 1, row++, 1, 1);

    // Checkboxes
    widgets->audio_only_check = gtk_check_button_new_with_label("Audio Only");
    gtk_grid_attach(GTK_GRID(grid), widgets->audio_only_check, 0, row++, 2, 1);

    widgets->subtitles_check = gtk_check_button_new_with_label("Download Subtitles");
    gtk_grid_attach(GTK_GRID(grid), widgets->subtitles_check, 0, row++, 2, 1);

    widgets->thumbnail_check = gtk_check_button_new_with_label("Embed Thumbnail");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(widgets->thumbnail_check), TRUE);
    gtk_grid_attach(GTK_GRID(grid), widgets->thumbnail_check, 0, row++, 2, 1);

    widgets->playlist_check = gtk_check_button_new_with_label("Download Full Playlist");
    gtk_grid_attach(GTK_GRID(grid), widgets->playlist_check, 0, row++, 2, 1);

    g_object_set_data_full(G_OBJECT(frame), "options-widgets", widgets, g_free);

    return frame;
}

void download_options_update_from_metadata(GtkWidget *panel, VideoMetadata *meta) {
    if (!meta) return;

    DownloadOptionsWidgets *widgets = g_object_get_data(G_OBJECT(panel), "options-widgets");
    if (!widgets) return;

    // Clear existing items
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widgets->quality_combo));

    if (meta->available_qualities) {
        // Add qualities from metadata
        for (int i = 0; meta->available_qualities[i] != NULL; i++) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->quality_combo),
                                           meta->available_qualities[i]);
        }

        // Select "Best Quality" by default
        gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->quality_combo), 0);
        gtk_widget_set_sensitive(widgets->quality_combo, TRUE);

        g_print("Updated quality options from metadata (%d options)\n",
                g_strv_length(meta->available_qualities));
    } else {
        // No qualities available - show error message
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->quality_combo),
                                       "No formats available");
        gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->quality_combo), 0);
        gtk_widget_set_sensitive(widgets->quality_combo, FALSE);
    }
}

void download_options_reset(GtkWidget *panel) {
    DownloadOptionsWidgets *widgets = g_object_get_data(G_OBJECT(panel), "options-widgets");
    if (!widgets) return;

    // Reset to placeholder state
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widgets->quality_combo));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets->quality_combo),
                                   "Enter URL to load options...");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->quality_combo), 0);
    gtk_widget_set_sensitive(widgets->quality_combo, FALSE);
}

DownloadOptions* download_options_get(GtkWidget *panel) {
    DownloadOptionsWidgets *widgets = g_object_get_data(G_OBJECT(panel), "options-widgets");
    if (!widgets) return NULL;

    DownloadOptions *opts = g_malloc0(sizeof(DownloadOptions));

    // Quality
    int quality_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->quality_combo));
    opts->quality = (VideoQuality)quality_idx;

    // Format
    int format_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->format_combo));
    opts->format = (DownloadFormat)format_idx;

    // Options
    opts->audio_only = gtk_check_button_get_active(GTK_CHECK_BUTTON(widgets->audio_only_check));
    opts->subtitles = gtk_check_button_get_active(GTK_CHECK_BUTTON(widgets->subtitles_check));
    opts->embed_thumbnail = gtk_check_button_get_active(GTK_CHECK_BUTTON(widgets->thumbnail_check));
    opts->playlist = gtk_check_button_get_active(GTK_CHECK_BUTTON(widgets->playlist_check));

    // Time range
    const char *start = gtk_editable_get_text(GTK_EDITABLE(widgets->time_start_entry));
    const char *end = gtk_editable_get_text(GTK_EDITABLE(widgets->time_end_entry));
    if (start && strlen(start) > 0) {
        opts->time_range_start = g_strdup(start);
    }
    if (end && strlen(end) > 0) {
        opts->time_range_end = g_strdup(end);
    }

    // Custom format
    if (opts->quality == QUALITY_CUSTOM) {
        const char *custom = gtk_editable_get_text(GTK_EDITABLE(widgets->custom_format_entry));
        if (custom && strlen(custom) > 0) {
            opts->custom_format = g_strdup(custom);
        }
    }

    return opts;
}
