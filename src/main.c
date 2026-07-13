#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../include/sort/merge_sort.h"

#define TEST(name)   printf("\n=== %s ===\n", name)
#define ОК(msg)    printf("  [OK] %s\n", msg)
#define TOTAL() \
    printf("\nИтог: %d/%d тестов пройдено\n", passed, total)

static int passed = 0;
static int total = 0;

#define CHECK(ifelse, msg)   \
    do {                                \
        total++;                        \
        if (ifelse) {                  \
            passed++;                   \
            printf("  [OK] %s\n", msg); \
        } else {                        \
            printf("  [!!] ПРОВАЛ: %s\n", msg); \
        }                               \
    } while(0)

// Компаратор для qsort
static int cmp_ints(const void* a, const void* b) 
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

static int cp_arr(int* src, int* dest, int size)
{
    if (!src || !dest) return -1; 
    memcpy(dest, src, size * sizeof(int));
    return 0;
}

static bool cmp_arr_ref(int* arr1, int size)
{  
    if (!arr1 || size <= 0) return false; 
    
    int* ref_arr = malloc(size * sizeof(int)); 
    if (!ref_arr) return false; 
    
    cp_arr(arr1, ref_arr, size);
    qsort(ref_arr, size, sizeof(int), cmp_ints); 
    
    bool is_equal = true;
    for (int i = 0; i < size; i++)
    {
        if (arr1[i] != ref_arr[i]) 
        {
            is_equal = false;
            break;
        }
    }
    
    free(ref_arr);
    return is_equal;
}

void merge_sort_basic()
{
    int arr1[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    printf("Исходный массив: ");    
    for (int i = 0; i < 9; i++) printf("%d ", arr1[i]);
    printf("\n");
    merge_sort(arr1, 0, 8);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 9; i++) printf("%d ", arr1[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr1, 9) == true, "случайный массив прошел");

    int arr2[] = {1, 2, 3, 4, 5};
    printf("Исходный массив: ");    
    for (int i = 0; i < 5; i++) printf("%d ", arr2[i]);
    printf("\n");
    merge_sort(arr2, 0, 4);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 5; i++) printf("%d ", arr2[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr2, 5) == true, "уже отсортированный массив прошел");
    
    int arr3[] = {5, 4, 3, 2, 1};
    printf("Исходный массив: ");    
    for (int i = 0; i < 5; i++) printf("%d ", arr3[i]);
    printf("\n");
    merge_sort(arr3, 0, 4);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 5; i++) printf("%d ", arr3[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr3, 5) == true, "массив в обратном порядке прошел");
    
    int arr4[] = {42};
    printf("Исходный массив: ");    
    for (int i = 0; i < 1; i++) printf("%d ", arr4[i]);
    printf("\n");
    merge_sort(arr4, 0, 0);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 1; i++) printf("%d ", arr4[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr4, 1) == true, "массив из одного элемента прошел");
 
    int arr5[] = {2, 2, 2, 2};
    printf("Исходный массив: ");    
    for (int i = 0; i < 4; i++) printf("%d ", arr5[i]);
    printf("\n");
    merge_sort(arr5, 0, 3);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 4; i++) printf("%d ", arr5[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr5, 4) == true, "массив из одинаковых элементов прошел");
}

void parallel_merge_sort_basic()
{
    int arr1[] = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    printf("Исходный массив: ");    
    for (int i = 0; i < 9; i++) printf("%d ", arr1[i]);
    printf("\n");
    parallel_merge_sort(arr1, 0, 8);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 9; i++) printf("%d ", arr1[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr1, 9) == true, "случайный массив прошел");

    int arr2[] = {1, 2, 3, 4, 5};
    printf("Исходный массив: ");    
    for (int i = 0; i < 5; i++) printf("%d ", arr2[i]);
    printf("\n");
    parallel_merge_sort(arr2, 0, 4);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 5; i++) printf("%d ", arr2[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr2, 5) == true, "уже отсортированный массив прошел");
    
    int arr3[] = {5, 4, 3, 2, 1};
    printf("Исходный массив: ");    
    for (int i = 0; i < 5; i++) printf("%d ", arr3[i]);
    printf("\n");
    parallel_merge_sort(arr3, 0, 4);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 5; i++) printf("%d ", arr3[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr3, 5) == true, "массив в обратном порядке прошел");
    
    int arr4[] = {42};
    printf("Исходный массив: ");    
    for (int i = 0; i < 1; i++) printf("%d ", arr4[i]);
    printf("\n");
    parallel_merge_sort(arr4, 0, 0);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 1; i++) printf("%d ", arr4[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr4, 1) == true, "массив из одного элемента прошел");
 
    int arr5[] = {2, 2, 2, 2};
    printf("Исходный массив: ");    
    for (int i = 0; i < 4; i++) printf("%d ", arr5[i]);
    printf("\n");
    parallel_merge_sort(arr5, 0, 3);
    printf("Отсортированный массив: ");        
    for (int i = 0; i < 4; i++) printf("%d ", arr5[i]);
    printf("\n");
    CHECK(cmp_arr_ref(arr5, 4) == true, "массив из одинаковых элементов прошел");
}


int main()
{

    merge_sort_basic();
    parallel_merge_sort_basic();
    TOTAL();
    return 0;
}

/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../include/core/types.h"
#include "../include/collectors/proc_collector.h"
#include "../include/collectors/cpu_collector.h"
#include "../include/collectors/mem_collector.h"
#include "../include/engine/engine.h"
#include "../include/ui/ui.h"
#include "../include/scheduler/scheduler.h"

#define UPDATE_INTERVAL 1 // секунд между обновлениями

static system_snapshot_t collect(void) {
    system_snapshot_t snap = {0};

    int count = 0;
    pid_t *pids = proc_collect_pids(&count);
    if (!pids) return snap;

    snap.processes = calloc(count, sizeof(process_t));
    if (!snap.processes) 
    { 
        free(pids); 
        return snap; 
    }

    snap.process_count = 0;
    for (int i = 0; i < count; i++) 
    {
        if (proc_collect_process(pids[i], &snap.processes[snap.process_count]) == 0)
            snap.process_count++;
    }
    free(pids);

    cpu_collect_stats(&snap.cpu);
    meminfo_collect_stats(&snap.mem);
    return snap;
}

static void run_monitor(void)
{
    engine_init();
    ui_init();
    
    system_snapshot_t snap = collect();
    engine_update(&snap);
    free(snap.processes);

    sleep(UPDATE_INTERVAL);

    int running = 1;
    while (running) 
    {
        snap = collect();
        engine_update(&snap);

        const computed_snapshot_t *result = engine_get();
        ui_render(&snap, result);

        free(snap.processes);

        for (int i = 0; i < UPDATE_INTERVAL * 10 && running; i++) 
        {
            running = ui_handle_input();
            usleep(100000); 
        }
    }

    ui_destroy();
    engine_destroy();
}

static void run_sim(void) 
{
    sim_process_t procs[] = {
        { .pid=1, .name="P1", .arrival=0, .burst=8,  .priority=2 },
        { .pid=2, .name="P2", .arrival=0, .burst=4,  .priority=1 },
        { .pid=3, .name="P3", .arrival=0, .burst=9,  .priority=3 },
        { .pid=4, .name="P4", .arrival=0, .burst=5,  .priority=1 },
        { .pid=5, .name="P5", .arrival=1, .burst=8,  .priority=2 },
        { .pid=6, .name="P6", .arrival=2, .burst=4,  .priority=3 },
        { .pid=7, .name="P7", .arrival=3, .burst=9,  .priority=3 },
        { .pid=8, .name="P8", .arrival=3, .burst=5,  .priority=1 },
    };

    int count = 8;
    int quantum = 3;
 
    printf("processes:\n");
    printf("%-8s  %8s  %8s  %8s\n", "Name", "Arrival", "Burst", "Priority");
    printf("%-8s  %8s  %8s  %8s\n", "----", "-------", "-----", "--------");
    for (int i = 0; i < count; i++) {
        printf("%-10s  %8d  %8d  %8d\n", procs[i].name, procs[i].arrival,
            procs[i].burst, procs[i].priority);
    }
 
    sim_snapshot_t result;
 
    sched_fifo(procs, count, &result);
    sim_print("FIFO", procs, count, &result);

    sched_priority(procs, count, &result);
    sim_print("Priority", procs, count, &result);

    sched_rr(procs, count, quantum, &result);
    sim_print("Round Robin", procs, count, &result);
}

int main(int argc, char *argv[]) 
{
    if (argc > 1 && strcmp(argv[1], "--sim") == 0) 
        run_sim();
    else 
        run_monitor();
}*/