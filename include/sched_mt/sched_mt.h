#ifndef SCHEDULER_MT_H
#define SCHEDULER_MT_H

#include <pthread.h>
#include "../core/types.h"

#define MAX_PROCESSES 16

typedef enum {
    FIFO,
    RR,
    PRIORITY
} sched_type_t;

// Описание одного потока-процесса
typedef struct {
    int id;           
    int priority;     
    int burst;      // время работы
    int arrival;    // задержка перед появлением в очереди
    int quantum;    // квант времени для RR

    proc_state_t state;
    int waited;     // время в ожидании
    int remaining;  // оставшееся время работы для RR

    pthread_cond_t  cond;
    pthread_mutex_t mutex;
} sim_proc_t;

typedef struct {
    sched_type_t type;
    int quantum; // для RR

    sim_proc_t procs[MAX_PROCESSES];
    int proc_count;

    // Очередь готовых процессов 
    int queue[MAX_PROCESSES];
    int queue_len;

    pthread_mutex_t queue_mutex;
    pthread_cond_t  queue_cond;
    pthread_cond_t  cpu_cond;

    int active_count; 
} scheduler_mt_t;

void sched_mt_init(scheduler_mt_t *s, sched_type_t type, int quantum_ms);
void sched_mt_destroy(scheduler_mt_t *s);

void sched_mt_add_proc(scheduler_mt_t *s, int id, int priority, int burst_ms, int arrival_ms);

void sched_mt_run(scheduler_mt_t *s);

#endif 