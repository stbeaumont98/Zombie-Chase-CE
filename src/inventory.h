#ifndef INVENTORY_H_
#define INVENTORY_H_

#include "types.h"

void draw_inventory(bool from_game, struct Player *p);
int player_has_item(struct Item *inventory[10], uint8_t id);

#endif