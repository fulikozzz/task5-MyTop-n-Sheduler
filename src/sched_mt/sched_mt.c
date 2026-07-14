#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "../../include/sched_mt/sched_mt.h"

static int queue_pop(scheduler_mt_t *s) 
{
    if (s->queue_len == 0) return -1;

    int best = 0; // индекс элемента в очереди

    if (s->type == PRIORITY) 
    {
        // выбираем процесс с наименьшим приоритетом
        for (int i = 1; i < s->queue_len; i++) {
            if (s->procs[s->queue[i]].priority < s->procs[s->queue[best]].priority)
                best = i;
        }
    }

    // для FIFO и RR берем первый в очереди
    int idx = s->queue[best];

    // сдвигаем оставшиеся элементы
    for (int i = best; i < s->queue_len - 1; i++)
        s->queue[i] = s->queue[i + 1];
    s->queue_len--;

    return idx;
}

typedef struct {
    scheduler_mt_t *sched;
    int idx;   
} proc_thread_args_t;

static long now_ms(void) 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void *proc_thread(void *arg) 
{
    proc_thread_args_t *a = (proc_thread_args_t*)arg;
    scheduler_mt_t* s = a->sched;
    int idx = a->idx;
    sim_proc_t* p = &s->procs[idx];
    free(a); 

    // Имитируем задержку появления процесса
    if (p->arrival > 0)
        usleep(p->arrival * 5000);

    printf("[proc %2d] появился в очереди (burst=%d мс, priority=%d)\n", p->id, p->burst, p->priority);

    while (p->remaining > 0) {
        // встаем в очередь планировщика
        pthread_mutex_lock(&s->queue_mutex);
        p->state = 'S'; 
        // сохраняем время
        long enqueue_time = now_ms(); 
        s->queue[s->queue_len++] = idx;
        pthread_cond_signal(&s->queue_cond); // будим планировщик
        pthread_mutex_unlock(&s->queue_mutex); 

        // ждем, пока планировщик переведет процесс в running
        pthread_mutex_lock(&p->mutex);
        while (p->state == 'S')
            pthread_cond_wait(&p->cond, &p->mutex);
        pthread_mutex_unlock(&p->mutex);

        // Считаем время ожидания
        p->waited += (long)((now_ms() - enqueue_time));

        if (p->state == 'Z') break;

        // вычисляем квант работы
        int run_time;
        if (s->type == RR) {
            run_time = (p->remaining < s->quantum) ? p->remaining : s->quantum;
        } else {
            run_time = p->remaining;
        }

        printf("[proc %2d] работает %d мс (осталось %d мс)\n", p->id, run_time, p->remaining - run_time);

        usleep(run_time * 5000); // Симулируем работу
        p->remaining -= run_time;

        pthread_mutex_lock(&s->queue_mutex);
        pthread_cond_signal(&s->queue_cond);
        pthread_mutex_unlock(&s->queue_mutex);
        
}

    // завершение процесса
    pthread_mutex_lock(&s->queue_mutex);
    p->state = 'Z';
    s->active_count--;
    printf("[proc %2d] завершён (ждал %d мс)\n", p->id, p->waited);
    pthread_cond_signal(&s->queue_cond); // Будим планировщик, чтобы обновить счетчик активных
    pthread_mutex_unlock(&s->queue_mutex);

    return NULL;
}

static void *scheduler_thread(void *arg) {
    scheduler_mt_t *s = (scheduler_mt_t *)arg;

    while (1) {
        pthread_mutex_lock(&s->queue_mutex);

        // ждем пока появится процесс или все завершатся
        while (s->queue_len == 0 && s->active_count > 0)
            pthread_cond_wait(&s->queue_cond, &s->queue_mutex);

        if (s->active_count == 0 && s->queue_len == 0) {
            pthread_mutex_unlock(&s->queue_mutex);
            break;
        }

        int idx = queue_pop(s);
        pthread_mutex_unlock(&s->queue_mutex);

        if (idx < 0) continue;

        sim_proc_t *p = &s->procs[idx];

        // будим выбранный процесс
        pthread_mutex_lock(&p->mutex);
        p->state = 'R';
        pthread_cond_signal(&p->cond);
        pthread_mutex_unlock(&p->mutex);

        // Ждем пока процесс отработает квант
        pthread_mutex_lock(&s->queue_mutex);
        pthread_cond_wait(&s->queue_cond, &s->queue_mutex);
        pthread_mutex_unlock(&s->queue_mutex);
    }

    return NULL;
}

void sched_mt_init(scheduler_mt_t *s, sched_type_t type, int quantum_ms) 
{
    // обнуляем структуру
    memset(s, 0, sizeof(*s));
    s->type = type;
    s->quantum = quantum_ms; // для RR
    // Инициализируем глобальные мьютекс и конд для защиты очереди
    pthread_mutex_init(&s->queue_mutex, NULL);
    pthread_cond_init(&s->queue_cond, NULL);
}

void sched_mt_destroy(scheduler_mt_t *s) 
{
    pthread_mutex_destroy(&s->queue_mutex);
    pthread_cond_destroy(&s->queue_cond);
    for (int i = 0; i < s->proc_count; i++) 
    {
        pthread_mutex_destroy(&s->procs[i].mutex);
        pthread_cond_destroy(&s->procs[i].cond);
    }
}

void sched_mt_add_proc(scheduler_mt_t *s, int id, int priority, int burst_ms, int arrival_ms) 
{
    if (s->proc_count >= MAX_PROCESSES) return;

    int idx = s->proc_count++;
    sim_proc_t *p = &s->procs[idx];

    p->id = id;
    p->priority = priority;
    p->burst = burst_ms;
    p->arrival = arrival_ms;
    p->remaining = burst_ms;
    p->quantum = s->quantum;
    p->state = 'S'; // изначально процесс спит
    p->waited = 0;

    pthread_mutex_init(&p->mutex, NULL);
    pthread_cond_init(&p->cond, NULL);
}

void sched_mt_run(scheduler_mt_t *s) 
{
    s->active_count = s->proc_count;

    // запускаем поток планировщик
    pthread_t sched_t;
    pthread_create(&sched_t, NULL, scheduler_thread, s);

    // запускаем потоки процессы
    pthread_t threads[MAX_PROCESSES];
    for (int i = 0; i < s->proc_count; i++) 
    {
        proc_thread_args_t *args = malloc(sizeof(proc_thread_args_t));
        args->sched = s;
        args->idx = i;
        pthread_create(&threads[i], NULL, proc_thread, args);
    }

    // ждём завершения всех процессов
    for (int i = 0; i < s->proc_count; i++)
        pthread_join(threads[i], NULL);

    pthread_join(sched_t, NULL);

    // табличка для вывода
    const char *type_name[] = { "FIFO", "RR", "Priority" };
    printf("\nИтог [%s] \n", type_name[s->type]);
    if (s->type == RR) printf("(Квант времени=%d мс) \n", s->quantum);
    printf("%-6s  %8s  %8s\n", "ProcID", "Burst мс", "Wait мс");
    for (int i = 0; i < s->proc_count; i++) 
    {
        sim_proc_t *p = &s->procs[i];
        printf("%-4d  %6d  %6d\n", p->id, p->burst, p->waited);
    }
}
