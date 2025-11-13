#include "download_row.h"

// Placeholder for download row widget
// This will display individual download items in the list

GtkWidget *download_row_new(DownloadItem *item) {
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top(box, 6);
    gtk_widget_set_margin_bottom(box, 6);
    gtk_widget_set_margin_start(box, 6);
    gtk_widget_set_margin_end(box, 6);

    // URL label
    GtkWidget *url_label = gtk_label_new(item->url);
    gtk_widget_set_hexpand(url_label, TRUE);
    gtk_widget_set_halign(url_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), url_label);

    // Status label
    const char *status_text = "Idle";
    switch (item->status) {
        case DOWNLOAD_STATUS_DOWNLOADING:
            status_text = "Downloading...";
            break;
        case DOWNLOAD_STATUS_COMPLETED:
            status_text = "Completed";
            break;
        case DOWNLOAD_STATUS_FAILED:
            status_text = "Failed";
            break;
        case DOWNLOAD_STATUS_CANCELLED:
            status_text = "Cancelled";
            break;
        default:
            break;
    }

    GtkWidget *status_label = gtk_label_new(status_text);
    gtk_box_append(GTK_BOX(box), status_label);

    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);

    return row;
}
