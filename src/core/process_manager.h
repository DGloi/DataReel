#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "common.h"

void process_manager_add(DownloadItem *item);
void process_manager_remove(DownloadItem *item);
GList *process_manager_get_all(void);
void process_manager_cleanup(void);

#endif
