#ifndef ENGINE_COMPUTED_TYPES_H
#define ENGINE_COMPUTED_TYPES_H

#include <sys/types.h>
#include <stdint.h>

/*
 * Вычисляемые метрики - результат сравнения двух снапшотов, полученных от collectors
 */

// Вычисленные метрики одного процесса
typedef struct {
    pid_t pid;
    double cpu_percent; // CPU% = (proc_delta / total_delta) * 100
    double mem_percent; // MEM% = rss * page_size / mem_total * 100
} computed_process_t;

// Полный вычисленный снапшот
typedef struct {
    computed_process_t *processes; // массив вычисленных метрик процессов
    int process_count;

    double cpu_total_percent; // суммарная загрузка CPU по системе
    double mem_total_percent; // суммарная загрузка RAM по системе
} computed_snapshot_t;

#endif