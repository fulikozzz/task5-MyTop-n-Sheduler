#ifndef COLLECTORS_MEM_COLLECTOR_H
#define COLLECTORS_MEM_COLLECTOR_H

#include "../core/types.h"

// Функция парсинга статистики памяти
// Читает /proc/meminfo, возвращает заполненную структуру mem_stats_t через параметр.
// Возвращает 0 при успехе, -1 при ошибке открытия или парсинга файла
int meminfo_collect_stats(mem_stats_t *out);

#endif