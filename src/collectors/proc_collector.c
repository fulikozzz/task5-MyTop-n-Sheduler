#include "../../include/collectors/proc_collector.h"

#include <ctype.h>  // Для функций типов
#include <dirent.h> // Для работы с каталогами
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h> // Обязательно для макросов SCNu64 и SCNd64

#define PROC_STAT_BUF_SIZE 4096
#define PIDS_INITIAL_CAPACITY 128

// Функция проверки имени на числовой формат
static int is_all_digits(const char *s) 
{
    if (*s == '\0') return 0;

    for (; *s; s++) 
    {
        if (!isdigit((unsigned char)*s)) return 0;
    }
    return 1;
}

// Функция получения массива pid процессов
pid_t *proc_collect_pids(int *count) 
{
    DIR *dir = opendir("/proc");
    if (!dir) 
    {
        if (count) *count = 0;
        return NULL;
    }

    size_t capacity = PIDS_INITIAL_CAPACITY;
    size_t n = 0;
    pid_t *pids = malloc(capacity * sizeof(pid_t));
    if (!pids) 
    {
        closedir(dir);
        if (count) *count = 0;
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        // Читаем только каталоги с числовым именем
        if (!is_all_digits(entry->d_name)) 
        {
            continue;
        }

        // Расширяем вместимость массива при необходимости
        if (n == capacity) 
        {
            capacity *= 2;
            pid_t *tmp = realloc(pids, capacity * sizeof(pid_t));
            if (!tmp) 
            {
                free(pids);
                closedir(dir);
                if (count) *count = 0;
                return NULL;
            }
            pids = tmp;
        }

        // сохраняем имя в массив
        pids[n++] = (pid_t)atoi(entry->d_name);
    }

    closedir(dir);
    if (count) *count = (int)n;
    return pids;
}

// Функция парсинга процессов по pid
int proc_collect_process(pid_t pid, process_t *out) 
{
    if (!out) return -1;

    // формируем путь к каталогу stat 
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", (int)pid);

    FILE *f = fopen(path, "r");
    if (!f) return -1; // Случай, если процесс завершился к моменту парсинга

    // Буффер для чтения stat
    char buf[PROC_STAT_BUF_SIZE];
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    // Парсим имя процесса
    // Имя в stat в скобках и может содержать пробелы и другие скобки
    char *name_start = strchr(buf, '('); // получаем левую скобку
    char *name_end = strrchr(buf, ')'); // получаем правую скобку
    if (!name_start || !name_end || name_end < name_start) return -1;

    // Очищаем переданную структуру
    memset(out, 0, sizeof(*out));
    out->pid = pid;

    size_t name_len = (size_t)(name_end - name_start - 1);
    if (name_len >= PROCESS_NAME_MAX) {
        name_len = PROCESS_NAME_MAX - 1;
    }
    
    memcpy(out->name, name_start + 1, name_len);
    out->name[name_len] = '\0';

    // переменные парсинга
    int matched = sscanf(name_end + 1,
        " %c %d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu" 
        " %lu %lu %*ld %*ld"                           
        " %ld %ld %ld %*ld"                            
        " %llu %" SCNu64 " %" SCNd64,                  
        &out->state,
        &out->ppid,       
        &out->utime, 
        &out->stime,
        &out->priority, 
        &out->nice, 
        &out->threads,
        &out->starttime, 
        &out->vsize,      
        &out->rss);       

    if (matched != 10) {
        return -1; 
    }

    return 0;
}