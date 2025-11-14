#include "downloader_interface.h"
#include "youtube_downloader.h"

static GList *registered_downloaders = NULL;

void downloader_register_all(void) {
    if (registered_downloaders) {
        return; // Already registered
    }

    registered_downloaders = g_list_append(registered_downloaders, youtube_downloader_get_interface());
}

DownloaderInterface* downloader_get_interface(DownloaderType type) {
    for (GList *l = registered_downloaders; l != NULL; l = l->next) {
        DownloaderInterface *iface = (DownloaderInterface *)l->data;
        if (iface->type == type) {
            return iface;
        }
    }
    return NULL;
}

GList* downloader_get_all_interfaces(void) {
    return registered_downloaders;
}
