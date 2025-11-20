#include "download_item_widget.h"
#include <glib/gprintf.h>

typedef struct {
    DownloadItem *item;
    GtkWidget *thumbnail;
    GtkWidget *title_label;
    GtkWidget *progress_bar;
    GtkWidget *status_label;
    GtkWidget *speed_label;
    GtkWidget *pause_button;
    GtkWidget *resume_button;
    GtkWidget *cancel_button;
    GtkWidget *retry_button;
    GtkWidget *main_box;
    GtkWidget *content_box;
    GtkWidget *info_box;
    guint update_timer;
} DownloadItemWidgetData;

static gboolean update_progress(gpointer user_data);
static void on_pause_clicked(GtkButton *button, gpointer user_data);
static void on_resume_clicked(GtkButton *button, gpointer user_data);
static void on_cancel_clicked(GtkButton *button, gpointer user_data);
static void on_retry_clicked(GtkButton *button, gpointer user_data);
static void on_download_row_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);

GtkWidget* download_item_widget_new(DownloadItem *item) {
    DownloadItemWidgetData *data = g_malloc0(sizeof(DownloadItemWidgetData));
    data->item = item;

    // Main container
    GtkWidget *list_row = gtk_list_box_row_new();
    gtk_widget_add_css_class(list_row, "download-item");

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_top(main_box, 6);
    gtk_widget_set_margin_bottom(main_box, 6);
    gtk_widget_set_margin_start(main_box, 8);
    gtk_widget_set_margin_end(main_box, 8);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(list_row), main_box);
    data->main_box = main_box;

    // Store the data in the main_box so we can access it from the click handler
    g_object_set_data_full(G_OBJECT(main_box), "widget-data", data, g_free);

    // Thumbnail - always visible with fixed size
    data->thumbnail = gtk_image_new();
    gtk_widget_set_size_request(data->thumbnail, 60, 45);
    gtk_box_append(GTK_BOX(main_box), data->thumbnail);

    // Content box (title, progress, status)
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(content_box, TRUE);
    gtk_box_append(GTK_BOX(main_box), content_box);
    data->content_box = content_box;

    // Title - always show the title or URL
    data->title_label = gtk_label_new(NULL);
    gtk_widget_set_halign(data->title_label, GTK_ALIGN_START);
    gtk_label_set_ellipsize(GTK_LABEL(data->title_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(data->title_label), 30);
    gtk_widget_add_css_class(data->title_label, "caption");

    if (item->metadata && item->metadata->title) {
        char *markup = g_markup_printf_escaped("<b>%s</b>", item->metadata->title);
        gtk_label_set_markup(GTK_LABEL(data->title_label), markup);
        g_free(markup);
    } else {
        // Always show the URL if no title is available
        gtk_label_set_text(GTK_LABEL(data->title_label), item->url);
    }
    gtk_box_append(GTK_BOX(content_box), data->title_label);

    // Uploader and duration info
    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    data->info_box = info_box;

    if (item->metadata && item->metadata->uploader) {
        GtkWidget *uploader = gtk_label_new(item->metadata->uploader);
        gtk_widget_add_css_class(uploader, "dim-label");
        gtk_widget_add_css_class(uploader, "caption");
        gtk_widget_set_halign(uploader, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(info_box), uploader);
    }

    if (item->metadata && item->metadata->duration) {
        GtkWidget *duration = gtk_label_new(item->metadata->duration);
        gtk_widget_add_css_class(duration, "dim-label");
        gtk_widget_add_css_class(duration, "caption");
        gtk_box_append(GTK_BOX(info_box), duration);
    }

    if (item->metadata && item->metadata->filesize > 0) {
        char *size_str = g_format_size(item->metadata->filesize);
        GtkWidget *filesize = gtk_label_new(size_str);
        gtk_widget_add_css_class(filesize, "dim-label");
        gtk_widget_add_css_class(filesize, "caption");
        gtk_box_append(GTK_BOX(info_box), filesize);
        g_free(size_str);
    }

    gtk_box_append(GTK_BOX(content_box), info_box);

    // Progress bar
    data->progress_bar = gtk_progress_bar_new();
    gtk_widget_set_hexpand(data->progress_bar, TRUE);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(data->progress_bar), TRUE);
    gtk_widget_add_css_class(data->progress_bar, "small");
    gtk_box_append(GTK_BOX(content_box), data->progress_bar);

    // Status and speed box
    GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    data->status_label = gtk_label_new("Queued");
    gtk_widget_add_css_class(data->status_label, "dim-label");
    gtk_widget_add_css_class(data->status_label, "caption");
    gtk_widget_set_halign(data->status_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(status_box), data->status_label);

    data->speed_label = gtk_label_new("");
    gtk_widget_add_css_class(data->speed_label, "dim-label");
    gtk_widget_add_css_class(data->speed_label, "caption");
    gtk_widget_set_hexpand(data->speed_label, TRUE);
    gtk_widget_set_halign(data->speed_label, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(status_box), data->speed_label);

    gtk_box_append(GTK_BOX(content_box), status_box);

    // Control buttons container
    GtkWidget *controls_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_valign(controls_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), controls_box);

    // Pause button
    data->pause_button = gtk_button_new_from_icon_name("media-playback-pause");
    gtk_widget_set_tooltip_text(data->pause_button, "Pause Download");
    gtk_widget_set_size_request(data->pause_button, 28, 28);
    g_signal_connect(data->pause_button, "clicked", G_CALLBACK(on_pause_clicked), data);
    gtk_widget_set_visible(data->pause_button, FALSE);
    gtk_box_append(GTK_BOX(controls_box), data->pause_button);

    // Resume button
    data->resume_button = gtk_button_new_from_icon_name("media-playback-start");
    gtk_widget_set_tooltip_text(data->resume_button, "Resume Download");
    gtk_widget_set_size_request(data->resume_button, 28, 28);
    g_signal_connect(data->resume_button, "clicked", G_CALLBACK(on_resume_clicked), data);
    gtk_widget_set_visible(data->resume_button, FALSE);
    gtk_box_append(GTK_BOX(controls_box), data->resume_button);

    // Cancel button
    data->cancel_button = gtk_button_new_from_icon_name("process-stop");
    gtk_widget_set_tooltip_text(data->cancel_button, "Cancel Download");
    gtk_widget_set_size_request(data->cancel_button, 28, 28);
    g_signal_connect(data->cancel_button, "clicked", G_CALLBACK(on_cancel_clicked), data);
    gtk_box_append(GTK_BOX(controls_box), data->cancel_button);

    // Retry button
    data->retry_button = gtk_button_new_from_icon_name("view-refresh");
    gtk_widget_set_tooltip_text(data->retry_button, "Retry Download");
    gtk_widget_set_size_request(data->retry_button, 28, 28);
    g_signal_connect(data->retry_button, "clicked", G_CALLBACK(on_retry_clicked), data);
    gtk_widget_set_visible(data->retry_button, FALSE);
    gtk_box_append(GTK_BOX(controls_box), data->retry_button);

    // Add click gesture to the main box for selection
    GtkGesture *click_gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_gesture), 1);
    g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_download_row_clicked), main_box);
    gtk_widget_add_controller(main_box, GTK_EVENT_CONTROLLER(click_gesture));

    // Start update timer
    data->update_timer = g_timeout_add(500, update_progress, data);

    g_object_set_data_full(G_OBJECT(list_row), "widget-data", data, g_free);

    return list_row;
}

static gboolean update_progress(gpointer user_data) {
    DownloadItemWidgetData *data = (DownloadItemWidgetData *)user_data;
    DownloadItem *item = data->item;

    if (!item) {
        data->update_timer = 0;
        return FALSE;
    }

    // Update progress bar
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data->progress_bar),
                                  item->progress / 100.0);

    char *progress_text = g_strdup_printf("%.1f%%", item->progress);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(data->progress_bar), progress_text);
    g_free(progress_text);

    // Update status
    const char *status_text = NULL;

    switch (item->status) {
        case DOWNLOAD_STATUS_IDLE:
            status_text = "Idle";
            break;
        case DOWNLOAD_STATUS_FETCHING_INFO:
            status_text = "Fetching info...";
            break;
        case DOWNLOAD_STATUS_QUEUED:
            status_text = "Paused";
            break;
        case DOWNLOAD_STATUS_DOWNLOADING:
            status_text = "Downloading";
            break;
        case DOWNLOAD_STATUS_PROCESSING:
            status_text = "Processing...";
            break;
        case DOWNLOAD_STATUS_COMPLETED:
            status_text = "✓ Completed";
            gtk_widget_set_sensitive(GTK_WIDGET(data->cancel_button), FALSE);
            gtk_widget_set_visible(data->cancel_button, FALSE);
            gtk_widget_set_visible(data->pause_button, FALSE);
            gtk_widget_set_visible(data->resume_button, FALSE);
            gtk_widget_set_visible(data->retry_button, FALSE);
            break;
        case DOWNLOAD_STATUS_FAILED:
            status_text = "✗ Failed";
            gtk_widget_set_sensitive(GTK_WIDGET(data->cancel_button), FALSE);
            gtk_widget_set_visible(data->cancel_button, FALSE);
            gtk_widget_set_visible(data->pause_button, FALSE);
            gtk_widget_set_visible(data->resume_button, FALSE);
            gtk_widget_set_visible(data->retry_button, TRUE);
            if (item->error_message) {
                gtk_widget_set_tooltip_text(data->status_label, item->error_message);
            }
            break;
        case DOWNLOAD_STATUS_CANCELLED:
            status_text = "Cancelled";
            gtk_widget_set_sensitive(GTK_WIDGET(data->cancel_button), FALSE);
            gtk_widget_set_visible(data->cancel_button, FALSE);
            gtk_widget_set_visible(data->pause_button, FALSE);
            gtk_widget_set_visible(data->resume_button, FALSE);
            gtk_widget_set_visible(data->retry_button, TRUE);
            break;
    }

    gtk_label_set_text(GTK_LABEL(data->status_label), status_text);

    // Update speed and ETA
    if (item->status == DOWNLOAD_STATUS_DOWNLOADING) {
        if (item->speed > 0) {
            char *speed_str = g_format_size((guint64)item->speed);
            char *speed_text = g_strdup_printf("%s/s", speed_str);

            if (item->eta) {
                char *full_text = g_strdup_printf("%s • ETA %s", speed_text, item->eta);
                gtk_label_set_text(GTK_LABEL(data->speed_label), full_text);
                g_free(full_text);
            } else {
                gtk_label_set_text(GTK_LABEL(data->speed_label), speed_text);
            }

            g_free(speed_str);
            g_free(speed_text);
        } else {
            gtk_label_set_text(GTK_LABEL(data->speed_label), "Starting...");
        }
    } else {
        gtk_label_set_text(GTK_LABEL(data->speed_label), "");
    }

    // Update button visibility based on status
    gboolean is_downloading = (item->status == DOWNLOAD_STATUS_DOWNLOADING);
    gboolean is_paused = (item->status == DOWNLOAD_STATUS_QUEUED);
    gboolean is_completed = (item->status == DOWNLOAD_STATUS_COMPLETED);
    gboolean is_cancelled = (item->status == DOWNLOAD_STATUS_CANCELLED);
    gboolean is_failed = (item->status == DOWNLOAD_STATUS_FAILED);

    gtk_widget_set_visible(data->pause_button, is_downloading);
    gtk_widget_set_visible(data->resume_button, is_paused);
    gtk_widget_set_visible(data->cancel_button, !is_completed && !is_cancelled && !is_failed);

    // Stop timer if download is finished
    if (item->status == DOWNLOAD_STATUS_COMPLETED ||
        item->status == DOWNLOAD_STATUS_FAILED ||
        item->status == DOWNLOAD_STATUS_CANCELLED) {
        data->update_timer = 0;
        return FALSE;
    }

    return TRUE;
}

static void on_pause_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DownloadItemWidgetData *data = (DownloadItemWidgetData *)user_data;
    if (data && data->item) {
        download_item_pause(data->item);
    }
}

static void on_resume_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DownloadItemWidgetData *data = (DownloadItemWidgetData *)user_data;
    if (data && data->item) {
        download_item_resume(data->item);
    }
}

static void on_cancel_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DownloadItemWidgetData *data = (DownloadItemWidgetData *)user_data;
    if (data && data->item) {
        download_item_cancel(data->item);
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    }
}

static void on_retry_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DownloadItemWidgetData *data = (DownloadItemWidgetData *)user_data;
    if (data && data->item) {
        // Reset item status and start again
        data->item->status = DOWNLOAD_STATUS_IDLE;
        data->item->progress = 0.0;
        g_free(data->item->error_message);
        data->item->error_message = NULL;

        // Reset UI elements
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data->progress_bar), 0.0);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(data->progress_bar), "0.0%");
        gtk_label_set_text(GTK_LABEL(data->status_label), "Queued");
        gtk_label_set_text(GTK_LABEL(data->speed_label), "");

        // Update button visibility based on new status
        gtk_widget_set_visible(data->pause_button, FALSE);
        gtk_widget_set_visible(data->resume_button, FALSE);
        gtk_widget_set_visible(data->cancel_button, TRUE);
        gtk_widget_set_visible(data->retry_button, FALSE);

        // Restart the download
        download_item_start(data->item);
    }
}

// Internal function to handle the click for selection
static void on_download_row_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {
    (void)gesture;
    (void)n_press;
    (void)x;
    (void)y;

    GtkWidget *main_box = (GtkWidget *)user_data;

    // Toggle selection state - just add/remove CSS class
    gboolean has_class = gtk_widget_has_css_class(main_box, "selected");
    if (has_class) {
        gtk_widget_remove_css_class(main_box, "selected");
    } else {
        gtk_widget_add_css_class(main_box, "selected");
    }
}

// Function to get the download item from the widget
DownloadItem* download_item_widget_get_item(GtkWidget *widget) {
    // Navigate up to find the list row, then get the widget data
    GtkWidget *parent = widget;
    while (parent && !GTK_IS_LIST_BOX_ROW(parent)) {
        parent = gtk_widget_get_parent(parent);
    }

    if (GTK_IS_LIST_BOX_ROW(parent)) {
        DownloadItemWidgetData *data = g_object_get_data(G_OBJECT(parent), "widget-data");
        return data ? data->item : NULL;
    }

    return NULL;
}
