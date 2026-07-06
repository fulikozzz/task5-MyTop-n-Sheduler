#include <stdio.h>
#include <stdlib.h>
#include "../include/core/types.h"
#include "../include/collectors/proc_collector.h"

int main() 
{
    int count = 0; 
    pid_t* pids = proc_collect_pids(&count); 
    
    if (pids == NULL || count == 0) {
        fprintf(stderr, "Не удалось собрать PID или процессов нет.\n");
        return 1;
    }

    process_t proc1;
    for (int i = 0; i < count && i < 10; i++) {
        if (proc_collect_process(pids[i], &proc1) == 0) {
            printf("Успешно распарсен процесс! PID: %d, Имя: %s, Состояние: %c\n", 
                   proc1.pid, proc1.name, proc1.state);
        } else {
            fprintf(stderr, "Ошибка парсинга для PID %d\n", pids[i]);
        }
    }

    free(pids); 

    return 0;
}