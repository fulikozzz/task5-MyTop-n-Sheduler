#ifndef COLLECTORS_PROC_COLLECTOR_H
#define COLLECTORS_PROC_COLLECTOR_H

#include "../core/types.h"

// Функция получения массива pid процессов, найденных в /proc в данный момент.
// Возвращает указатель на массив процессов, либо NULL при ошибке
// Освобождение памяти - задача вызывающего
pid_t *proc_collect_pids(int *count);

// Функция парсинга процессов по pid
// Читает /proc/[pid]/stat для процесса, возвращает заполненную структуру process_t через параметр.
// Возвращает 0 при успехе, -1 при ошибке или зваершении процесса
int proc_collect_process(pid_t pid, process_t *out);

#endif