#include "../../include/collectors/cpu_collector.h"

#include <stdio.h>
#include <string.h>

#define CPU_STAT_BUF_SIZE 256

int cpu_collect_stats(cpu_stats_t *out) {
    if (!out) return -1;

    FILE *f = fopen("/proc/stat", "r");
    if (!f) return -1;

    memset(out, 0, sizeof(cpu_stats_t));

    char buffer[CPU_STAT_BUF_SIZE];
    int common_line_parsed = 0;

    while (fgets(buffer, sizeof(buffer), f)) {

        // Первая строка это общая статистика по всем ядрам в формате:
        // cpu user nice system idle iowait irq softirq
        if (!common_line_parsed && strstr(buffer, "cpu ") == buffer) {
            int matched = sscanf(buffer,
                "cpu %lu %lu %lu %lu %lu %lu %lu",
                &out->user, &out->nice, &out->system, &out->idle,
                &out->iowait, &out->irq, &out->softirq);

            if (matched != 7) {
                fclose(f);
                return -1;
            }
            common_line_parsed = 1;
        }
        // Остальные строки это статистика по отдльным ялрам
        else if (strstr(buffer, "cpu") == buffer && buffer[3] >= '0' && buffer[3] <= '9') {
            out->cpu_count++;
        }
        
        // Остальные строки на данном этапе не нужны
        else if (common_line_parsed && strstr(buffer, "cpu") != buffer) {
            break;
        }
    }

    fclose(f);
    
    if (!common_line_parsed) return -1;
    else return 0;
}