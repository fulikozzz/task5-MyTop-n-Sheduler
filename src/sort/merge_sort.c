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
 
// бинарный поиск
// если right <= left или x <= T[left] - возвращает left
// иначе возвращает наибольший индекс i из отрезка [left,right] такой, что array[i−1]<x
static int binary_search(int x, int *arr, int left, int right) 
{
    while (left < right) 
    {
        int mid = (left + right) / 2;
        if (arr[mid] < x)
            left = mid + 1;
        else
            right = mid;
    }
    return left;
}
 
// Предопределение тк взаимная рекурсия
static void merge_mt(int *src, int l1, int r1, int l2, int r2, int *result, int l3);
 
static void *thread_merge(void *arg) 
{
    merge_args_t *a = (merge_args_t *)arg;
    merge_mt(a->src, a->l1, a->r1, a->l2, a->r2, a->result, a->l3);
    free(a);
    return NULL;
}
 
// однопоточное слияние при ошибке args
static void one_thread_merge(int *src, int l1, int r1, int l2, int r2, int *result, int l3) 
{
    int i = l1;
    int j = l2;
    int k = l3;
    while (i <= r1 && j <= r2) 
    {
        if (src[i] <= src[j])
            result[k++] = src[i++];
        else
            result[k++] = src[j++];
    }

    while (i <= r1) result[k++] = src[i++];
    while (j <= r2) result[k++] = src[j++];
}

// параллельное слияние
static void merge_mt(int *src, int l1, int r1, int l2, int r2, int *result, int l3) 
{
    int n1 = r1 - l1 + 1;
    int n2 = r2 - l2 + 1;
    if (n1 < n2) {
        int tmp = l1; l1 = l2; l2 = tmp;
        tmp = r1; r1 = r2; r2 = tmp;
        tmp = n1; n1 = n2; n2 = tmp;
    }
    if (n1 == 0) return;

    if (n1 + n2 < MIN_PARALLEL_SIZE) 
    {
        one_thread_merge(src, l1, r1, l2, r2, result, l3);
        return;
    }

    int mid1 = (l1 + r1) / 2;
    int mid2 = binary_search(src[mid1], src, l2, r2);
    int mid3 = l3 + (mid1 - l1) + (mid2 - l2);
    result[mid3] = src[mid1];

    pthread_t t;
    merge_args_t *args = malloc(sizeof(merge_args_t));
    if (!args) 
    {
        one_thread_merge(src, l1, r1, l2, r2, result, l3);
        return;
    }
    args->src = src;
    args->l1 = l1;
    args->r1 = mid1 - 1;
    args->l2 = l2;
    args->r2 = mid2 - 1;
    args->result = result;
    args->l3 = l3;

    int err = pthread_create(&t, NULL, thread_merge, args); 
    if (err != 0) 
    {
        free(args);
        one_thread_merge(src, l1, r1, l2, r2, result, l3);
        return;
    }

    merge_mt(src, mid1 + 1, r1, mid2, r2, result, mid3 + 1);
    pthread_join(t, NULL);
}
 
// функция адаптер для соответствия аргументам pthread_create
static void *thread_sort_mt(void *arg) 
{
    sort_args_t *a = (sort_args_t *)arg;
    parallel_merge_sort_mt(a->arr, a->left, a->right);
    //free(a);
    return NULL;
}
 
// многопоточная сортировка с многопоточным слиянием 
void parallel_merge_sort_mt(int *arr, int left, int right) 
{
    if (left >= right) return;
 
    if (right - left + 1 < MIN_PARALLEL_SIZE)
    {
        merge_sort(arr, left, right);
        return;
    }

    int n = right - left + 1;
    int mid = (left + right) / 2;
    int new_mid = mid - left; // граница в tmp между левой и правой частями
 
    int *tmp = malloc(n * sizeof(int));
    if (!tmp) return;
 
    // сортируем левую половину A в tmp[0..new_mid]
    pthread_t t;
    sort_args_t *args = malloc(sizeof(sort_args_t));
    // В случае ошибки сортируем в однопоточке
    if (!args) {
        free(tmp);
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
        return;
    }
    args->arr = arr;
    args->left = left;
    args->right = mid;

    int err = pthread_create(&t, NULL, thread_sort_mt, args);
    if (err != 0) 
    {
        free(args);
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
        return;
    }
 
    // сортируем правую половину
    parallel_merge_sort_mt(arr, mid + 1, right);
 
    pthread_join(t, NULL);
 
    // копируем обе отсортированные половины в tmp
    memcpy(tmp, arr + left, n * sizeof(int));
 
    // параллельное слияние tmp в arr
    merge_mt(tmp, 0, new_mid, new_mid + 1, n - 1, arr, left);
 
    free(tmp);
}
 
