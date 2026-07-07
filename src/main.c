#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/core/types.h"
#include "../include/collectors/proc_collector.h"
#include "../include/collectors/mem_collector.h"
#include "../include/collectors/cpu_collector.h"
#include "../include/engine/engine.h"

static system_snapshot_t collect() {
    system_snapshot_t snap = {0};

    int count = 0;
    pid_t *pids = proc_collect_pids(&count);
    if (!pids) return snap;

    snap.processes = calloc(count, sizeof(process_t));
    if (!snap.processes) { free(pids); return snap; }

    snap.process_count = 0;
    for (int i = 0; i < count; i++) {
        if (proc_collect_process(pids[i], &snap.processes[snap.process_count]) == 0)
            snap.process_count++;
    }
    free(pids);

    cpu_collect_stats(&snap.cpu);
    meminfo_collect_stats(&snap.mem);
    return snap;
}

int main(void) {
    engine_init();

    while(1){
        system_snapshot_t snap1 = collect();
        engine_update(&snap1);
        free(snap1.processes);
        
        sleep(1);
        system("clear");

        system_snapshot_t snap2 = collect();
        engine_update(&snap2);

        const computed_snapshot_t *result = engine_get();

        printf("\nCPU: %.1f%%", result->cpu_total_percent);
        printf("\nMEM: %.1f%%\n", result->mem_total_percent);
        printf("\n");
        printf("%-6s  %-16s  %6s  %6s\n", "PID", "NAME", "CPU%", "MEM%");

        for (int i = 0; i < result->process_count; i++) {
            const computed_process_t *cp = &result->processes[i];
            if (cp->cpu_percent < 0.01 && cp->mem_percent < 0.01) continue;

            // ищем имя процесса в snap2
            const char *name = "";
            for (int j = 0; j < snap2.process_count; j++) {
                if (snap2.processes[j].pid == cp->pid) {
                    name = snap2.processes[j].name;
                    break;
                }
            }
            printf("%-6d  %-16s  %6.1f  %6.1f\n",
                cp->pid, name, cp->cpu_percent, cp->mem_percent);
        }
        
        free(snap2.processes);
    }
    
    engine_destroy();
    return 0;
}