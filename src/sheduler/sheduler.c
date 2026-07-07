#include "../../include/scheduler/scheduler.h"

#include <string.h>
 
 // функция инициализации снапшота симуляции
static void init_snapshot(sim_snapshot_t *out) 
{
    memset(out, 0, sizeof(*out));
    // -1 - простой
    for (int i = 0; i < GANTT_WIDTH; i++) out->gantt[i] = -1;
}
 
// записывает pid в диаграмму на тик t
static void gantt_set(sim_snapshot_t *out, int t, int pid) 
{
    // если влезает в массив
    if (t < GANTT_WIDTH) 
    {
        // записывем pid и увеличиваем gantt_len по необходимости
        out->gantt[t] = pid;
        if (t + 1 > out->gantt_len) out->gantt_len = t + 1;
    }
}
 
// функция посчета среднего времени ожидания и цикла жизни
static void calc_averages(sim_snapshot_t *out) 
{
    // если процессов нет
    if (out->count == 0) return;

    double sum_wait = 0, sum_turnaround = 0;
    // суммируем показатели всех процессов
    for (int i = 0; i < out->count; i++) 
    {
        sum_wait += out->results[i].waiting_time;
        sum_turnaround += out->results[i].turnaround;
    }
    // вычисляем avg
    out->avg_waiting    = sum_wait / out->count;
    out->avg_turnaround = sum_turnaround / out->count;
}
 
// Алгоритм FIFO
void sched_fifo(sim_process_t *procs, int count, sim_snapshot_t *out) 
{
    // Все процессы делаем ожидающими
    init_snapshot(out);
    if (count <= 0 || count > MAX_PROCESSES) return;
 
    // Сортируем по времени поступления
    sim_process_t sorted[MAX_PROCESSES];
    memcpy(sorted, procs, count * sizeof(sim_process_t));
    for (int i = 1; i < count; i++) 
    {
        sim_process_t key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j].arrival > key.arrival) 
        {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }
  
    int time = 0;
    // Проходим по процессам
    for (int i = 0; i < count; i++) {
        // Если процессор простаивал до прихода процесса
        if (time < sorted[i].arrival) 
            time = sorted[i].arrival;
 
        int waiting = time - sorted[i].arrival;
 
        // Выполняем процесс до конца
        for (int t = 0; t < sorted[i].burst; t++)
        {
            gantt_set(out, time + t, sorted[i].pid);
        }
        time += sorted[i].burst;
 
        // Сохраняем результаты
        out->results[i].pid = sorted[i].pid;
        strncpy(out->results[i].name, sorted[i].name, 15);
        out->results[i].finish_time  = time;
        out->results[i].waiting_time = waiting;
        out->results[i].turnaround   = time - sorted[i].arrival;
    }
 
    out->count = count;
    calc_averages(out);
}

#define MAX_PRIORITY_LEVELS 8 // максимальное количество уровней приоритетов

// Структура FIFO-очереди
typedef struct 
{
    sim_process_t items[MAX_PROCESSES];
    int head;
    int tail;
    int size;
} fifo_queue_t;

// Структура очередей
typedef struct 
{
    fifo_queue_t queues[MAX_PRIORITY_LEVELS];
} multilevel_ready_queues_t;

// Инициализация всех очередей
static void multilevel_queues_init(multilevel_ready_queues_t *mq)
{
    for (int i = 0; i < MAX_PRIORITY_LEVELS; i++)
    {
        mq->queues[i].head = 0;
        mq->queues[i].tail = 0;
        mq->queues[i].size = 0;
    }
}

// Функция добавления приоритета в нужную очередь
static void multilevel_enqueue(multilevel_ready_queues_t *mq, sim_process_t p)
{
    int pr = p.priority;
    if (pr < 0 || pr >= MAX_PRIORITY_LEVELS) return;

    fifo_queue_t *q = &mq->queues[pr];
    q->items[q->tail++] = p;
    q->size++;
}

// Функция получения процесса из приоритетной очереди
static sim_process_t multilevel_dequeue(multilevel_ready_queues_t *mq)
{
    for (int i = 0; i < MAX_PRIORITY_LEVELS; i++)
    {
        if (mq->queues[i].size > 0)
        {
            fifo_queue_t *q = &mq->queues[i];
            q->size--;
            return q->items[q->head++];
        }
    }
    // В случае пустоты очередей возвращаем пустую структуру
    sim_process_t empty = {0};
    return empty;
}

// Функция проверяет наличие процессов в очередях
static int multilevel_has_ready(multilevel_ready_queues_t *mq)
{
    for (int i = 0; i < MAX_PRIORITY_LEVELS; i++)
    {
        if (mq->queues[i].size > 0) return 1;
    }
    return 0;
}

// Алгоритм приоритетного планирования с очередями
void sched_priority(sim_process_t *procs, int count, sim_snapshot_t *out)
{
    // Все процессы делаем ожидающими
    init_snapshot(out);
    if (count <= 0 || count > MAX_PROCESSES) return;

    multilevel_ready_queues_t ready_system;
    multilevel_queues_init(&ready_system);

    int added[MAX_PROCESSES];
    memset(added, 0, sizeof(added));

    int time = 0;
    int done = 0;

    while (done < count)
    {
        // Добавляем во все соответствующие очереди прибывшие процессы
        for (int i = 0; i < count; i++)
        {
            if (!added[i] && procs[i].arrival <= time)
            {
                multilevel_enqueue(&ready_system, procs[i]);
                added[i] = 1;
            }
        }

        // Перемещаем системное время к ближайшему по прибытию процессу если все очереди пусты
        if (!multilevel_has_ready(&ready_system))
        {
            int next = -1;

            for (int i = 0; i < count; i++)
            {
                if (!added[i] && (next < 0 || procs[i].arrival < procs[next].arrival))
                {
                    next = i;
                }
            }

            if (next >= 0)
                time = procs[next].arrival;

            continue;
        }

        // Извлекаем процесс из самой приоритетной очереди
        sim_process_t current = multilevel_dequeue(&ready_system);

        int waiting = time - current.arrival;

        // Выбранный процесс выполняется до полного завершения
        for (int t = 0; t < current.burst; t++)
            gantt_set(out, time + t, current.pid);

        time += current.burst;

        // Сохраняем результаты выполнения       
        out->results[done].pid = current.pid;
        strncpy(out->results[done].name, current.name, 15);
        out->results[done].finish_time = time;
        out->results[done].waiting_time = waiting;
        out->results[done].turnaround = time - current.arrival;

        done++;
    }

    out->count = count;
    calc_averages(out);
}
