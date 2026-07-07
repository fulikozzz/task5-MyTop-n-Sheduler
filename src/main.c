#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/core/types.h"
#include "../include/collectors/proc_collector.h"
#include "../include/collectors/cpu_collector.h"
#include "../include/collectors/mem_collector.h"
#include "../include/engine/engine.h"
#include "../include/ui/ui.h"

#define UPDATE_INTERVAL 1 // секунд между обновлениями

static system_snapshot_t collect(void) {
    system_snapshot_t snap = {0};

    int count = 0;
    pid_t *pids = proc_collect_pids(&count);
    if (!pids) return snap;

    snap.processes = calloc(count, sizeof(process_t));
    if (!snap.processes) 
    { 
        free(pids); 
        return snap; 
    }

    snap.process_count = 0;
    for (int i = 0; i < count; i++) 
    {
        if (proc_collect_process(pids[i], &snap.processes[snap.process_count]) == 0)
            snap.process_count++;
    }
    free(pids);

    cpu_collect_stats(&snap.cpu);
    meminfo_collect_stats(&snap.mem);
    return snap;
}

int main(void) 
{
    engine_init();
    ui_init();
    
    system_snapshot_t snap = collect();
    engine_update(&snap);
    free(snap.processes);

    sleep(UPDATE_INTERVAL);

    int running = 1;
    while (running) 
    {
        snap = collect();
        engine_update(&snap);

        const computed_snapshot_t *result = engine_get();
        ui_render(&snap, result);

        free(snap.processes);

        for (int i = 0; i < UPDATE_INTERVAL * 10 && running; i++) 
        {
            running = ui_handle_input();
            usleep(100000); 
        }
    }

    ui_destroy();
    engine_destroy();
    return 0;
}