#ifndef SCHEDULER_SCHEDULER_H
#define SCHEDULER_SCHEDULER_H

#define MAX_PROCESSES 16
#define GANTT_WIDTH 60
#define MAX_NAME_LENGHT 16

// входные данные процесса 
typedef struct 
{
    int pid;
    char name[MAX_NAME_LENGHT];
    int arrival;   // тик появления в очереди
    int burst;     // требуемое время процессора в тиках
    int priority;  // приоритет процесса
} sim_process_t;

// результат симуляции одного процесса
typedef struct 
{
    int pid;
    char name[MAX_NAME_LENGHT];
    int finish_time;  // момент завершения
    int waiting_time; // время ожидания в очереди
    int turnaround;   // finish_time - arrival (полное время от прихода до конца)
} sim_result_t;

// общий результат симуляции
typedef struct 
{
    sim_result_t results[MAX_PROCESSES];
    int count;

    // Для диаграммы 
    // индекс массива - тик времени, значение - PID выполняемого процесса 
    int gantt[GANTT_WIDTH];
    int gantt_len; // общее время реализации

    double avg_waiting;    // среднее время ожидания
    double avg_turnaround; // среднее время от появления до завершения
} sim_snapshot_t;

// Алгоритм FIFO
void sched_fifo(sim_process_t *procs, int count, sim_snapshot_t *out);

// Алгоритм Round Robin
void sched_rr(sim_process_t *procs, int count, int quantum, sim_snapshot_t *out);
 
// Алгоритм риоритетного планирования
void sched_priority(sim_process_t *procs, int count, sim_snapshot_t *out);


// Функция печати диаграммы Ганта
void sim_print(const char *algo_name, sim_process_t *procs, int proc_count, const sim_snapshot_t *s);

#endif