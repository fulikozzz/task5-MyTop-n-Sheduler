#include "../../include/scheduler/scheduler.h"

#include <stdio.h>
#include <string.h>

#define SYM_BUSY '#'
#define SYM_IDLE '.'
#define SYM_WAIT ' '

// Печатает диаграмму Ганта и таблицу метрик для одного алгоритма
void sim_print(const char *algo_name, sim_process_t *procs, int proc_count, const sim_snapshot_t *s) 
{
    printf("\n");
    printf("!--- %s \n", algo_name);
    printf("\n");

    // Шкала времени
    printf("%-10s ", "");
    for (int t = 0; t < s->gantt_len; t++)
        printf("%d", t % 10);
    printf("\n");

    printf("%-10s |", "");
    for (int t = 0; t < s->gantt_len; t++) printf("-");
    printf("|\n");

    // Строка для каждого процесса
    for (int i = 0; i < proc_count; i++) 
    {
        printf("%-10s |", procs[i].name);
        for (int t = 0; t < s->gantt_len; t++) 
        {
            if (s->gantt[t] == procs[i].pid)
                putchar(SYM_BUSY);         // процесс работает
            else if (t >= procs[i].arrival)
                putchar(SYM_IDLE);         // процесс ждёт
            else
                putchar(SYM_WAIT);         // ещё не пришёл
        }
        printf("|\n");
    }

    printf("%-10s |", "");
    for (int t = 0; t < s->gantt_len; t++) printf("-");
    printf("|\n");

    // таблица метрик
    printf("\n");
    printf("%-10s  %8s  %8s  %10s\n", "Process", "Finish", "Waiting", "Turnaround");
    printf("%-10s  %8s  %8s  %10s\n", "-------", "------", "-------", "----------");

    for (int i = 0; i < s->count; i++) 
    {
        printf("%-10s  %8d  %8d  %10d\n", s->results[i].name, s->results[i].finish_time,
            s->results[i].waiting_time, s->results[i].turnaround);
    }

    printf("\n");
    printf("Avg waiting:    %.2f\n", s->avg_waiting);
    printf("Avg turnaround: %.2f\n", s->avg_turnaround);
}