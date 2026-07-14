#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#include "../include/sched_mt/sched_mt.h"

static int passed = 0;
static int total  = 0;

#define TEST(name) printf("\n=== %s ===\n", name)

#define CHECK(cond, msg) do {               \
    total++;                                \
    if (cond) {                             \
        passed++;                           \
        printf("  [OK] %s\n", msg);        \
    } else {                                \
        printf("  [!!] ПРОВАЛ: %s\n", msg);\
    }                                       \
} while(0)

#define TOTAL() printf("\nИтог: %d/%d тестов пройдено\n", passed, total)

static long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

void test_fifo(void) {
    TEST("Проверка FIFO планировщика");
    scheduler_mt_t s;
    sched_mt_init(&s, FIFO, 0);
    sched_mt_add_proc(&s, 1, 1, 20, 0);
    sched_mt_add_proc(&s, 2, 1, 35, 0);
    sched_mt_add_proc(&s, 3, 1, 10, 0);
    sched_mt_run(&s);
    sched_mt_destroy(&s);
}

void test_priority(void) {
    TEST("Проверка PRIORITY планировщика");
    scheduler_mt_t s;
    sched_mt_init(&s, PRIORITY, 0); 
    sched_mt_add_proc(&s, 1, 3, 20, 0);
    sched_mt_add_proc(&s, 2, 1, 35, 0);
    sched_mt_add_proc(&s, 3, 2, 10, 0);
    sched_mt_run(&s);
    sched_mt_destroy(&s);
}

void test_rr(void) {
    TEST("Проверка RR планировщика");
    scheduler_mt_t s;
    sched_mt_init(&s, RR, 5);
    sched_mt_add_proc(&s, 1, 1, 20, 0);
    sched_mt_add_proc(&s, 2, 1, 35, 0);
    sched_mt_add_proc(&s, 3, 1, 10, 0);
    sched_mt_run(&s);
    sched_mt_destroy(&s);
}

int main(void) {
    
    test_fifo();
    test_priority();
    test_rr();

    return 0;
}