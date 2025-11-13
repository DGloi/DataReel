#include "process_manager.h"

// Placeholder for managing multiple download processes
// Future: queue management, concurrent downloads, process monitoring

static GList *active_downloads = NULL;

void process_manager_add(DownloadItem *item) {
    if (item) {
        active_downloads = g_list_append(active_downloads, item);
    }
}

void process_manager_remove(DownloadItem *item) {
    if (item) {
        active_downloads = g_list_remove(active_downloads, item);
    }
}

GList *process_manager_get_all(void) {
    return active_downloads;
}

void process_manager_cleanup(void) {
    g_list_free(active_downloads);
    active_downloads = NULL;
}
