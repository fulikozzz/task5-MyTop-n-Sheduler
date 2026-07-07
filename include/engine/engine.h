#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "../core/types.h"
#include "calc_types.h"

/*
 * engine вычисляет метрики из двух последовательных снапшотов
 */

// Инициализация 
void engine_init(void);

// Принимает новый снапшот, вычисляет метрики относительно предыдущего
void engine_update(const system_snapshot_t *snap);

// Возвращает указатель на последний вычисленный результат
const computed_snapshot_t *engine_get(void);

// Освобождает внутреннее состояние engine
void engine_destroy(void);

#endif