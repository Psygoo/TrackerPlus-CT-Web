/* Logical Chance order, separate from persisted firmware IDs. */
#ifndef H11_CHANCE_ORDER_H
#define H11_CHANCE_ORDER_H
#include <stdint.h>
uint32_t h11_chance_rank(uint32_t value);  /* valid values 0..141 */
uint32_t h11_chance_value(uint32_t rank);  /* valid ranks 0..141 */
uint32_t h11_chance_edit(uint32_t value, int32_t delta);
#endif
