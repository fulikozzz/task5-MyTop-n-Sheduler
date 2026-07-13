#ifndef SORT_MERGE_SORT_H
#define SORT_MERGE_SORT_H

// структура аргументов для сортировки
typedef struct {
    int *arr;  
    int  left; 
    int  right; 
} sort_args_t;

// структура аргументов для многопоточного слияния
typedef struct {
    int *src; // исходный массив
    // границы первого подмассива
    int  l1;  
    int  r1; 
    // границы второго подмассива
    int  l2;  
    int  r2;  

    int *result; // результирующий массив
    int  l3;   
} merge_args_t;

// однопоточная сортировка
void merge_sort(int *arr, int left, int right);

// многопоточная сортировка с однопоточным слиянием
void parallel_merge_sort(int *arr, int left, int right);

// многопоточная сортировка с многопоточным слиянием
void parallel_merge_sort_mt(int *arr, int left, int right);

#endif