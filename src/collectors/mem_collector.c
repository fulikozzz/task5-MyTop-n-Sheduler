#include "../../include/collectors/mem_collector.h"

#include <stdio.h>
#include <string.h>

#define MEM_STAT_BUF_SIZE 256

int meminfo_collect_stats(mem_stats_t *out) {
    if (!out) return -1;

    // формируем путь к файлу meminfo 
    char path[64];
    snprintf(path, sizeof(path), "/proc/meminfo");

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    // Очищаем переданную структуру
    memset(out, 0, sizeof(mem_stats_t));

    // Буффер для чтения meminfo
    char buffer[MEM_STAT_BUF_SIZE];
    int matched = 0;

    // Читаем построчно, так как строки разделены \n
    while (fgets(buffer, sizeof(buffer), f)) {
        unsigned long value_kb = 0;

        // проверяем наличие подстроки в читаемой строке
        if (strstr(buffer, "MemTotal:") == buffer) {
            if (sscanf(buffer, "MemTotal: %lu", &value_kb) == 1) {
                // Переводим в байты
                out->total = (uint64_t)value_kb * 1024; 
                matched++;
            }
        } 
        else if (strstr(buffer, "MemFree:") == buffer) {
            if (sscanf(buffer, "MemFree: %lu", &value_kb) == 1) {
                out->free = (uint64_t)value_kb * 1024;
                matched++;
            }
        } 
        else if (strstr(buffer, "MemAvailable:") == buffer) {
            if (sscanf(buffer, "MemAvailable: %lu", &value_kb) == 1) {
                out->available = (uint64_t)value_kb * 1024;
                matched++;
            }
        } 
        else if (strstr(buffer, "Buffers:") == buffer) {
            if (sscanf(buffer, "Buffers: %lu", &value_kb) == 1) {
                out->buffers = (uint64_t)value_kb * 1024;
                matched++;
            }
        } 
        else if (strstr(buffer, "Cached:") == buffer) {
            if (sscanf(buffer, "Cached: %lu", &value_kb) == 1) {
                out->cached = (uint64_t)value_kb * 1024;
                matched++;
            }
        }

        if (matched == 5) {
            break;
        }
    }

    fclose(f);

    // Если структура повреждена, возвращаем ошибку
    if (matched < 5) return -1;

    return 0;
}