#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <stdint.h>   // Для uint64_t
#include <sys/types.h> // Для pid_t 

#define PROCESS_NAME_MAX 256

/* Состояние процесса:
 * 'R' running, 'S' sleeping, 'D' disk sleep, 'Z' zombie,
 * 'T' stopped, 't' tracing stop, 'X' dead
 */
typedef char proc_state_t;

// Процесс (из /proc/[pid]/stat)
typedef struct {
    pid_t pid;                       // PID
    char name[PROCESS_NAME_MAX];
    proc_state_t state;

    pid_t ppid;                      // PID родителя

    // Время в тиках для расчета CPU%
    unsigned long utime;             // время в пользовательском режиме
    unsigned long stime;             // время в системном режиме (ядро)
    unsigned long long starttime;    // время старта

    uint64_t vsize;                  // виртуальная память в байтах (VIRT)
    uint64_t rss;                    // резидентная память в страницах (RES)

    long priority;                   // приоритет ядра (PR)
    long nice;                       // вежливость (NI)
    long threads;                    // число потоков
} process_t;

// Статистика по использованию процессора
typedef struct {
    unsigned long user;    // время работы в пространстве пользователя
    unsigned long nice;    // время работы процессов с измененным приоритетом
    unsigned long system;  // время работы в системном режиме
    unsigned long idle;    // время простоя
    unsigned long iowait;  // время ожидания завершения I/O операций
    unsigned long irq;     // время обработки аппаратных прерываний
    unsigned long softirq; // время обработки программных прерываний
    int cpu_count;         // количество ядер процессора
} cpu_stats_t;

// Статистика по использованию RAM
typedef struct {
    uint64_t total;        // общий объём 
    uint64_t free;         // свободная память
    uint64_t available;    // доступно для новых процессов без свапа
    uint64_t buffers;      // память под буферы ядра
    uint64_t cached;       // память под страничный кэш
} mem_stats_t;

// Снапшот состояния системы
typedef struct {
    process_t *processes;         // массив процессов
    int process_count;            // количество процессов 

    cpu_stats_t cpu;              // общая статистика CPU
    mem_stats_t mem;              // общая статистика RAM

    uint64_t timestamp;           // момент снятия снапшота в мс
} system_snapshot_t;

#endif