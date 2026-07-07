#include "../../include/sheduler/sheduler.h"

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
