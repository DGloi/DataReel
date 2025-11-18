#include "download_item_widget.h"

typedef struct {
    DownloadItem *item;
    GtkWidget *thumbnail;
    GtkWidget *title_label;
    GtkWidget *progress_bar;
    GtkWidget *status_label;
    GtkWidget *speed_label;
    GtkWidget *cancel_button;
    guint update_timer;
} DownloadItemWidgetData;

static gboolean update_progress(gpointer user_data);
static void on_cancel_clicked(GtkButton *button, gpointer user_data);

GtkWidget* download_item_widget_new(DownloadItem *item) {
    DownloadItemWidgetData *data = g_malloc0(sizeof(DownloadItemWidgetData));
    data->item = item;

    // Main container
    GtkWidget *list_row = gtk_list_box_row_new();
    gtk_widget_add_css_class(list_row, "download-item");

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top(main_box, 8);
    gtk_widget_set_margin_bottom(main_box, 8);
    gtk_widget_set_margin_start(main_box, 12);
    gtk_widget_set_margin_end(main_box, 12);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(list_row), main_box);

    // Thumbnail
    data->thumbnail = gtk_image_new();
    gtk_widget_set_size_request(data->thumbnail, 120, 90);

    if (item->metadata && item->metadata->thumbnail_pixbuf) {
        GdkPixbuf *scaled = gdk_pixbuf_scale_simple(
            item->metadata->thumbnail_pixbuf,
            120, 90,
            GDK_INTERP_BILINEAR
        );
        GdkTexture *texture = gdk_texture_new_for_pixbuf(scaled);
        gtk_image_set_from_paintable(GTK_IMAGE(data->thumbnail), GDK_PAINTABLE(texture));
        g_object_unref(scaled);
        g_object_unref(texture);
    } else {
        gtk_image_set_from_icon_name(GTK_IMAGE(data->thumbnail), "video-x-generic");
        gtk_image_set_icon_size(GTK_IMAGE(data->thumbnail), GTK_ICON_SIZE_LARGE);
    }

    gtk_box_append(GTK_BOX(main_box), data->thumbnail);

    // Content box (title, progress, status)
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand(content_box, TRUE);
    gtk_box_append(GTK_BOX(main_box), content_box);

    // Title
    data->title_label = gtk_label_new(NULL);
    gtk_widget_set_halign(data->title_label, GTK_ALIGN_START);
    gtk_label_set_ellipsize(GTK_LABEL(data->title_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(data->title_label), 60);

    if (item->metadata && item->metadata->title) {
        char *markup = g_markup_printf_escaped("<b>%s</b>", item->metadata->title);
        gtk_label_set_markup(GTK_LABEL(data->title_label), markup);
        g_free(markup);
    } else {
        gtk_label_set_text(GTK_LABEL(data->title_label), item->url);
    }
    gtk_box_append(GTK_BOX(content_box), data->title_label);

    // Uploader and duration info
    if (item->metadata) {
        GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

        if (item->metadata->uploader) {
            GtkWidget *uploader = gtk_label_new(item->metadata->uploader);
            gtk_widget_add_css_class(uploader, "dim-label");
            gtk_widget_set_halign(uploader, GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(info_box), uploader);
        }

        if (item->metadata->duration) {
            GtkWidget *duration = gtk_label_new(item->metadata->duration);
            gtk_widget_add_css_class(duration, "dim-label");
            gtk_box_append(GTK_BOX(info_box), duration);
        }

        if (item->metadata->filesize > 0) {
            char *size_str = g_format_size(item->metadata->filesize);
            GtkWidget *filesize = gtk_label_new(size_str);
            gtk_widget_add_css_class(filesize, "dim-label");
            gtk_box_append(GTK_BOX(info_box), filesize);
            g_free(size_str);
        }

        gtk_box_append(GTK_BOX(content_box), info_box);
    }

    // Progress bar
    data->progress_bar = gtk_progress_bar_new();
    gtk_widget_set_hexpand(data->progress_bar, TRUE);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(data->progress_bar), TRUE);
    gtk_box_append(GTK_BOX(content_box), data->progress_bar);

    // Status and speed box
    GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    data->status_label = gtk_label_new("Queued");
    gtk_widget_add_css_class(data->status_label, "dim-label");
    gtk_widget_set_halign(data->status_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(status_box), data->status_label);

    data->speed_label = gtk_label_new("");
    gtk_widget_add_css_class(data->speed_label, "dim-label");
    gtk_widget_set_hexpand(data->speed_label, TRUE);
    gtk_widget_set_halign(data->speed_label, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(status_box), data->speed_label);

    gtk_box_append(GTK_BOX(content_box), status_box);

    // Cancel button
    data->cancel_button = gtk_button_new_from_icon_name("process-stop");
    gtk_widget_set_valign(data->cancel_button, GTK_ALIGN_CENTER);
    gtk_widget_set_tooltip_text(data->cancel_button, "Cancel Download");
    g_signal_connect(data->cancel_button, "clicked", G_CALLBACK(on_cancel_clicked), data);
    gtk_box_append(GTK_BOX(main_box), data->cancel_button);

    // Start update timer
    data->update_timer = g_timeout_add(500, update_progress, data);

    g_object_set_data_full(G_OBJECT(list_row), "widget-data", data, g_free);

    return list_row;
}

static gboolean update_progress(gpointer user_data) {
    DownloadItemWidgetData *data = (DownloadItemWidgetData *)user_data;
    DownloadItem *item = data->item;

    if (!item) return FALSE;

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
            status_text = "Queued";
            break;
        case DOWNLOAD_STATUS_DOWNLOADING:
            status_text = "Downloading";
            break;
        case DOWNLOAD_STATUS_PROCESSING:
            status_text = "Processing...";
            break;
        case DOWNLOAD_STATUS_COMPLETED:
            status_text = "✓ Completed";
            gtk_widget_set_sensitive(data->cancel_button, FALSE);
            break;
        case DOWNLOAD_STATUS_FAILED:
            status_text = "✗ Failed";
            gtk_widget_set_sensitive(data->cancel_button, FALSE);
            break;
        case DOWNLOAD_STATUS_CANCELLED:
            status_text = "Cancelled";
            gtk_widget_set_sensitive(data->cancel_button, FALSE);
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
        }
    } else {
        gtk_label_set_text(GTK_LABEL(data->speed_label), "");
    }

    // Stop timer if download is finished
    if (item->status == DOWNLOAD_STATUS_COMPLETED ||
        item->status == DOWNLOAD_STATUS_FAILED ||
        item->status == DOWNLOAD_STATUS_CANCELLED) {
        return FALSE;
    }

    return TRUE;
}

static void on_cancel_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    DownloadItemWidgetData *data = (DownloadItemWidgetData *)user_data;

    if (data->item) {
        download_item_cancel(data->item);
        gtk_widget_set_sensitive(data->cancel_button, FALSE);
    }
}
