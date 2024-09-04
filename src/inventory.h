#ifndef INVENTORY_H_
#define INVENTORY_H_

#include <stdint.h>
#include "types.h"

static uint8_t inv_size = 0;

void draw_inventory(bool from_game, struct Player *p);
int player_has_item(struct Item *inventory[10], uint8_t id);

#endif