#include "sort/merge_sort.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define MIN_PARALLEL_SIZE 10000

// Функция слияния
static void merge(int *arr, int left, int mid, int right) 
{
    int n = right - left + 1;
    int *tmp = malloc(n * sizeof(int));
    if (!tmp) return;

    int i = left;      
    int j = mid + 1;   
    int k = 0;         

    while (i <= mid && j <= right) 
    {
        if (arr[i] <= arr[j])
            tmp[k++] = arr[i++];
        else
            tmp[k++] = arr[j++];
    }
    while (i <= mid)   tmp[k++] = arr[i++];
    while (j <= right) tmp[k++] = arr[j++];

    memcpy(arr + left, tmp, n * sizeof(int));
    free(tmp);
}

// Функция сортировки
void merge_sort(int *arr, int left, int right) 
{
    if (left >= right) return;

    int mid = (left + right) / 2;
    merge_sort(arr, left, mid);
    merge_sort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// функция адаптер для соответствия аргументам pthread_create
static void *thread_sort(void *arg) 
{
    sort_args_t *a = (sort_args_t *)arg; // явно приводим аргументы
    parallel_merge_sort(a->arr, a->left, a->right); // сортируем
    free(a); // очищаем аргументы
    return NULL;
}

// многопоточная сортировка с однопоточным слиянием
void parallel_merge_sort(int *arr, int left, int right) 
{
    if (left >= right) return;

    if (right - left + 1 < MIN_PARALLEL_SIZE)
    {
        merge_sort(arr, left, right);
        return;
    }

    int mid = (left + right) / 2;

    pthread_t t;
    
    sort_args_t *args = malloc(sizeof(sort_args_t));
    // В случае ошибки сортируем в однопоточке
    if (!args) {
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
        return;
    }
    args->arr = arr;
    args->left = left;
    args->right = mid;

    int err = pthread_create(&t, NULL, thread_sort, args);
    // В случае ошибки сортируем в однопоточке
    if (err != 0) 
    {
        free(args);
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
        return;
    }

    parallel_merge_sort(arr, mid + 1, right);

    pthread_join(t, NULL);

    merge(arr, left, mid, right);
}