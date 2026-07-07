#include "engine/engine.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h> // sysconf

// Структура для хранения предыдущих тиков процессов
typedef struct 
{
    pid_t pid;
    unsigned long utime;
    unsigned long stime;
} proc_ticks_t;

// Структура внутреннего состояния engine
typedef struct 
{
    cpu_stats_t prev_cpu; // тики процессора из предыдущего снапшота

    proc_ticks_t *prev_procs; // предыдущие тики процессо
    int prev_proc_count;

    int has_prev; // флаг наличия предыдущего снапшота
    computed_snapshot_t result; // результат последнего вычисления

    long page_size; // байт на страницу
} engine_state_t;

static engine_state_t *g_state = NULL;

// Суммарные тики процессора - все время
static unsigned long cpu_total(const cpu_stats_t *c) 
{
    return c->user + c->nice + c->system + c->idle
         + c->iowait + c->irq + c->softirq;
}

// Время простоя процессора
static unsigned long cpu_idle(const cpu_stats_t *c) 
{
    return c->idle + c->iowait;
}

// Функция поиска предыдущих тиков процесса по pid
static const proc_ticks_t *find_prev(const engine_state_t *s, pid_t pid) 
{
    for (int i = 0; i < s->prev_proc_count; i++) 
        if (s->prev_procs[i].pid == pid) 
                return &s->prev_procs[i];
    return NULL;
}

// Функцкция инициализации 
void engine_init(void) 
{
    g_state = calloc(1, sizeof(engine_state_t));
    if (!g_state) return;

    g_state->page_size = sysconf(_SC_PAGESIZE);
    // Если sysconf вернул ошибку, используем стандартное значение 4096 байт
    if (g_state->page_size <= 0) g_state->page_size = 4096;
}

// Функция обновления engine с новым снапшотом
void engine_update(const system_snapshot_t *snap) 
{
    if (!g_state || !snap) return;

    engine_state_t *s = g_state;

    // выделяем массив результатов под текущее количество процессов 
    free(s->result.processes);
    s->result.processes = calloc(snap->process_count, sizeof(computed_process_t));
    s->result.process_count = snap->process_count;
    s->result.cpu_total_percent = 0.0;
    s->result.mem_total_percent = 0.0;

    // процент CPU общий
    // если есть предыдущий снапшот
    if (s->has_prev) {
        // вычисляем разницу тиков процессора между снапшотами
        unsigned long total_delta = cpu_total(&snap->cpu) - cpu_total(&s->prev_cpu);
        // и разницу простоя
        unsigned long idle_delta  = cpu_idle(&snap->cpu)  - cpu_idle(&s->prev_cpu);

        // Загрузка = (время работы / общее время) * 100
        s->result.cpu_total_percent = (double)(total_delta - idle_delta) / total_delta * 100.0;
    }

    // процент RAM общий
    if (snap->mem.total > 0) 
    {
        uint64_t used = snap->mem.total - snap->mem.available;
        s->result.mem_total_percent = (double)used / snap->mem.total * 100.0;
    }

    // загрузка процессора и памяти каждым процессом
    unsigned long total_delta = 0;
    if (s->has_prev) total_delta = cpu_total(&snap->cpu) - cpu_total(&s->prev_cpu);

    // проходим по всем процессам в текущем снапшоте
    for (int i = 0; i < snap->process_count; i++) 
    {
        const process_t *p = &snap->processes[i];
        computed_process_t *out = &s->result.processes[i];

        out->pid = p->pid;

        // %CPU процесса
        if (s->has_prev) {
            const proc_ticks_t *prev = find_prev(s, p->pid);
            if (prev) {
                unsigned long proc_delta = (p->utime + p->stime) - (prev->utime + prev->stime);
                // процент = (разница тиков процесса / разница тиков процессора) * 100
                out->cpu_percent = (double)proc_delta / total_delta * 100.0;
            }
        }

        // %MEM процесса
        if (snap->mem.total > 0) {
            uint64_t rss_bytes = p->rss * (uint64_t)s->page_size;
            // процент = (объем резидентной памяти процесса / общая память) * 100
            out->mem_percent = (double)rss_bytes / snap->mem.total * 100.0;
        }
    }

    // предыдущий снапшот становится текущим
    s->prev_cpu = snap->cpu;
    // освобождаем старый массив предыдущих тиков процессов и выделяем новый
    free(s->prev_procs);
    s->prev_procs = malloc(snap->process_count * sizeof(proc_ticks_t));
    if (s->prev_procs) {
        // если есть предыдущий снапшот, копируем тики процессов в массив prev_procs
        for (int i = 0; i < snap->process_count; i++) {
            s->prev_procs[i].pid   = snap->processes[i].pid;
            s->prev_procs[i].utime = snap->processes[i].utime;
            s->prev_procs[i].stime = snap->processes[i].stime;
        }
        s->prev_proc_count = snap->process_count;
    }

    s->has_prev = 1;
}

// возвращает указатель на последний вычисленный результат
const computed_snapshot_t *engine_get(void) {
    if (!g_state) return NULL;
    return &g_state->result;
}

// функция высвобождения 
void engine_destroy(void) {
    if (!g_state) return;
    free(g_state->result.processes);
    free(g_state->prev_procs);
    free(g_state);
    g_state = NULL;
}