#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../include/core/types.h"
#include "../include/collectors/proc_collector.h"
#include "../include/collectors/cpu_collector.h"
#include "../include/collectors/mem_collector.h"
#include "../include/engine/engine.h"
#include "../include/ui/ui.h"
#include "../include/scheduler/scheduler.h"

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

static void run_monitor(void)
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
}

static void run_sim(void) 
{
    sim_process_t procs[] = {
        { .pid=1, .name="P1", .arrival=0, .burst=8,  .priority=2 },
        { .pid=2, .name="P2", .arrival=1, .burst=4,  .priority=1 },
        { .pid=3, .name="P3", .arrival=2, .burst=9,  .priority=3 },
        { .pid=4, .name="P4", .arrival=3, .burst=5,  .priority=1 },
    };

    int count = 4;
 
    printf("processes:\n");
    printf("%-8s  %8s  %8s  %8s\n", "Name", "Arrival", "Burst", "Priority");
    printf("%-8s  %8s  %8s  %8s\n", "----", "-------", "-----", "--------");
    for (int i = 0; i < count; i++) {
        printf("%-10s  %8d  %8d  %8d\n", procs[i].name, procs[i].arrival,
            procs[i].burst, procs[i].priority);
    }
 
    sim_snapshot_t result;
 
    sched_fifo(procs, count, &result);
    sim_print("FIFO", procs, count, &result);
 
}

int main(int argc, char *argv[]) 
{
    if (argc > 1 && strcmp(argv[1], "--sim") == 0) 
        run_sim();
    else 
        run_monitor();
}