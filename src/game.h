#ifndef GAME_H_
#define GAME_H_

#include <graphx.h>
#include "gfx/gfx.h"

static gfx_sprite_t *zombie_sprites[8] = {z_0, z_1, z_2, z_3, z_4, z_5, z_6, z_7};

static char status_string[0xFF];

static struct Player p;
static struct Target objects[16];		// Up to 16 objects can be on screen at once.

void game(int8_t game_mode);

#endif