#include "../../include/ui/ui.h"
#include "../../include/engine/engine.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BAR_WIDTH 30

// структура для шапки с системной инфой
typedef struct {
    double uptime_sec;
    double load1, load5, load15;
    int total, running, sleeping, zombie;
    char cpu_model[64];
    int cpu_cores;
} sysinfo_t;

static void collect_sysinfo(sysinfo_t *s, const system_snapshot_t *snap) 
{
    memset(s, 0, sizeof(*s));

    // получаем uptime
    FILE *f = fopen("/proc/uptime", "r");
    if (f) 
    {
        fscanf(f, "%lf", &s->uptime_sec);
        fclose(f);
    }

    // получаем load average
    f = fopen("/proc/loadavg", "r");
    if (f) 
    {
        fscanf(f, "%lf %lf %lf", &s->load1, &s->load5, &s->load15);
        fclose(f);
    }

    // получаем модель CPU и количество ядер
    f = fopen("/proc/cpuinfo", "r");
    if (f) 
    {
        char buf[256];
        while (fgets(buf, sizeof(buf), f)) 
        {
            if (strstr(buf, "model name") && s->cpu_model[0] == '\0') 
            {
                char *colon = strchr(buf, ':');
                if (colon) 
                {
                    char *p = colon + 2;
                    buf[strcspn(buf, "\n")] = '\0';
                    strncpy(s->cpu_model, p, sizeof(s->cpu_model) - 1);
                }
            }
            if (strstr(buf, "cpu cores")) {
                sscanf(buf, "cpu cores : %d", &s->cpu_cores);
            }
        }
        fclose(f);
        if (s->cpu_cores == 0) s->cpu_cores = snap->cpu.cpu_count;
    }

    // статистика по процессам из снапшота
    s->total = snap->process_count;
    for (int i = 0; i < snap->process_count; i++) 
    {
        char st = snap->processes[i].state;
        if (st == 'R') s->running++;
        else if (st == 'Z') s->zombie++;
        else s->sleeping++;
    }
}

// Фуенкция отрисовки прогресс-бара
static void draw_bar(int row, int col, const char *label, double percent) 
{
    if (percent < 0.0) percent = 0.0;
    if (percent > 100.0) percent = 100.0;

    int filled = (int)(percent / 100.0 * BAR_WIDTH);

    mvprintw(row, col, "%-4s [", label);
    attron(A_BOLD);
    for (int i = 0; i < filled; i++) addch(ACS_BLOCK);
    attroff(A_BOLD);
    for (int i = filled; i < BAR_WIDTH; i++) addch(' ');
    printw("] %5.1f%%", percent);
}

static const computed_snapshot_t *g_sort_result = NULL;

// функция сортировки по %CPU
static int cmp_by_cpu(const void *a, const void *b) 
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    double da = g_sort_result->processes[ia].cpu_percent;
    double db = g_sort_result->processes[ib].cpu_percent;
    if (db > da) return  1;
    if (db < da) return -1;
    return 0;
}

// функция инициализации текстового интерфейса ncurses
void ui_init(void) 
{
    
    initscr(); // включает режим ncurses
    cbreak(); // отключает буферизацию строк
    noecho(); // отключает эхо нажимаемых клавиш на экран
    curs_set(0); // скрывает мигающий курсор
    nodelay(stdscr, TRUE); // делает чтение клавиш неблокирующим
    keypad(stdscr, TRUE); // разрешает обработку спецклавиш
}

// функция отрисовки
void ui_render(const system_snapshot_t *snap, const computed_snapshot_t *result) 
{
    if (!snap || !result) return;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // очищаем экран
    erase();

    // данные для шапки
    sysinfo_t sys;
    collect_sysinfo(&sys, snap);

    int row = 0;
    int uptime_h = (int)(sys.uptime_sec / 3600);
    int uptime_m = (int)(sys.uptime_sec / 60) % 60;
    int uptime_s = (int)(sys.uptime_sec) % 60;
    mvprintw(row++, 0, "mini-monitor  uptime: %02d:%02d:%02d  (q - exit)", uptime_h, uptime_m, uptime_s);

    mvprintw(row++, 0, "CPU: %.*s (%d core%s)", cols - 20, sys.cpu_model,  sys.cpu_cores, sys.cpu_cores > 1 ? "s" : "");

    mvprintw(row++, 0, "Load avg: %.2f  %.2f  %.2f  (1/5/15 min)", sys.load1, sys.load5, sys.load15);

    mvprintw(row++, 0, "Tasks: %d total  %d running  %d sleeping  %d zombie", sys.total, sys.running, sys.sleeping, sys.zombie);

    row++; 

    // рисуем бар по процессору
    draw_bar(row, 0, "CPU", result->cpu_total_percent);

    unsigned long total = snap->cpu.user + snap->cpu.nice + snap->cpu.system
                        + snap->cpu.idle + snap->cpu.iowait
                        + snap->cpu.irq  + snap->cpu.softirq;
    if (total > 0) 
    {
        mvprintw(row++, 42, "  us:%.1f sy:%.1f id:%.1f wa:%.1f",
                 100.0 * snap->cpu.user   / total,
                 100.0 * snap->cpu.system / total,
                 100.0 * snap->cpu.idle   / total,
                 100.0 * snap->cpu.iowait / total);
    } 
    else 
    {
        row++;
    }

    // рисуем бар по памяти
    draw_bar(row, 0, "MEM", result->mem_total_percent);

    uint64_t used  = snap->mem.total - snap->mem.available;
    uint64_t total_mb  = snap->mem.total / 1024 / 1024;
    uint64_t used_mb   = used / 1024 / 1024;
    uint64_t avail_mb  = snap->mem.available / 1024 / 1024;
    uint64_t cached_mb = (snap->mem.cached + snap->mem.buffers) / 1024 / 1024;
    mvprintw(row++, 42, "  %lluMB/%lluMB  avail:%lluMB  cache:%lluMB",
             (unsigned long long)used_mb,
             (unsigned long long)total_mb,
             (unsigned long long)avail_mb,
             (unsigned long long)cached_mb);

    row++; 

    // рисуем таблицу процессов
    attron(A_REVERSE);
    mvprintw(row++, 0, "%6s  %5s  %-15s  %5s  %5s  %6s  %7s  %7s  %4s  %3s  %s",
             "PID", "PPID", "NAME", "PR", "NI",
             "THR", "VIRT MB", "RES MB", "CPU%", "MEM%", "S");
    attroff(A_REVERSE);

    // сортировка
    int count = result->process_count;
    int *idx = malloc(count * sizeof(int));
    if (!idx) return;
    for (int i = 0; i < count; i++) idx[i] = i;
    g_sort_result = result;
    qsort(idx, count, sizeof(int), cmp_by_cpu);

    // строки процессов
    int max_proc_rows = rows - row - 1;
    for (int r = 0; r < count && r < max_proc_rows; r++) 
    {
        int i = idx[r];
        const computed_process_t *cp = &result->processes[i];

        // ищем raw данные в snap
        const process_t *p = NULL;
        for (int j = 0; j < snap->process_count; j++) 
        {
            if (snap->processes[j].pid == cp->pid) 
            {
                p = &snap->processes[j];
                break;
            }
        }
        if (!p) continue;

        uint64_t virt_mb = p->vsize / 1024 / 1024;
        uint64_t res_mb  = p->rss * 4096 / 1024 / 1024; // пока размер страницы захардкоден

        char short_name[16];
        strncpy(short_name, p->name, 15);
        short_name[15] = '\0';

        mvprintw(row + r, 0,
                 "%6d  %5d  %-15s  %5ld  %5ld  %6ld  %7llu  %7llu  %4.1f  %3.1f  %c",
                 p->pid, p->ppid, short_name,
                 p->priority, p->nice, p->threads,
                 (unsigned long long)virt_mb,
                 (unsigned long long)res_mb,
                 cp->cpu_percent, cp->mem_percent,
                 p->state);
    }

    free(idx);
    refresh();
}

// функция обработки ввода
int ui_handle_input(void) 
{
    int ch = getch();
    if (ch == 'q' || ch == 'Q') return 0;
    return 1;
}

void ui_destroy(void) 
{
    endwin();
}