#include <stdio.h>
#include <stdlib.h>
#include "../include/core/types.h"
#include "../include/collectors/proc_collector.h"
#include "../include/collectors/mem_collector.h"

int main() 
{
    int count = 0; 
    pid_t* pids = proc_collect_pids(&count); 
    
    for(int i = 0; i < count; i++){
        printf(" %ld", pids[i]);
    }

    if (pids == NULL || count == 0) {
        fprintf(stderr, "Не удалось собрать PID или процессов нет.\n");
        return 1;
    }

    process_t proc1;
    for (int i = 0; i < count && i < 1000; i++) {
        if (proc_collect_process(pids[i], &proc1) == 0) {
            printf("Успешно распарсен процесс! PID: %d, Имя: %s, Состояние: %c\n", 
                   proc1.pid, proc1.name, proc1.state);
        } else {
            fprintf(stderr, "Ошибка парсинга для PID %d\n", pids[i]);
        }
    }
    
    mem_stats_t stats1;
    if (meminfo_collect_stats(&stats1) == 0) {
        printf("Память:\n Free: %llu\n Available: %llu\n Total: %llu\n Cached/Buffer: %llu\n",
               stats1.free, stats1.available, 
               stats1.total, stats1.buffers + stats1.cached);
    } else {
        fprintf(stderr, "Ошибка сбора метрик памяти!\n");
    }

    free(pids); 

    return 0;
}