#ifndef COLLECTORS_CPU_COLLECTOR_H
#define COLLECTORS_CPU_COLLECTOR_H

#include "../core/types.h"

// Читает первую общую строку из /proc/stat, заполняет out
// Возвращает 0 при успехе, -1 при ошибке открытия или парсинга файла
int cpu_collect_stats(cpu_stats_t *out);

#endif