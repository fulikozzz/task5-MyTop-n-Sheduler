#ifndef UI_UI_H
#define UI_UI_H

#include "../core/types.h"
#include "../engine/calc_types.h"

/*
 * ui - терминальный интерфейс на ncurses
 */

// Инициализация ncurses и настройка терминала
void ui_init(void);

// Отрисовка одного кадра
// шапка: %CPU и %MEM
// таблица процессов, отсортированных по убыванию %CPU
void ui_render(const system_snapshot_t *snap,
               const computed_snapshot_t *result);

// Обработка нажатий клавиш 
int ui_handle_input(void);

// Завершение работы ncurses, восстановление терминала
void ui_destroy(void);

#endif 