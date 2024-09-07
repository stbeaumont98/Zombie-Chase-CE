#include <stdint.h>
#include <string.h>

#include <debug.h>

#include "inventory.h"

#include "types.h"

struct Item new_item(int8_t type, int8_t id, char name[], char desc[], uint8_t quantity, gfx_sprite_t *icon) {
	static struct Item i;

	i.type = type;
	i.id = id;
	strcpy(i.name, name);
	strcpy(i.description, desc);
	i.quantity = quantity;
	i.icon = icon;

	return i;
}

int8_t get_item_index(struct Item list[], int8_t id) {
	int8_t i;
	for (i = 0; i < 12; i++) {
		if (list[i].id == id)
			return i;
	}
	return -1;
}

struct Target new_object(int8_t type, int8_t id, uint16_t x, uint8_t y, uint8_t timer, uint8_t radius) {
	static struct Target t;

	t.type = type;
	t.id = id;
	t.x = x;
	t.y = y;
	t.timer = timer;
	t.radius = radius;

	return t;
}

int8_t get_object_index(struct Target list[], int8_t id) {
	int8_t i;
	for (i = 0; i < 12; i++) {
		if (list[i].id == id)
			return i;
	}
	return -1;
}

struct Target drop_object(uint8_t id, uint16_t x, uint8_t y) {
	static struct Target t;
	t.type = (id == ID_GRENADE || id == ID_C4 || id == ID_LAND_MINE);
	t.id = id;
	t.x = x;
	t.y = y;
	switch (id) {
		case ID_GRENADE:
			t.timer = 5;
			t.radius = 15;
			break;
		case ID_C4:
			t.timer = 1;
			t.radius = 30;
			break;
		case ID_LAND_MINE:
			t.timer = 1;
			t.radius = 60;
			break;
		case ID_TBONE_STEAK:
			t.timer = 5;
			t.radius = 15;
			break;
		case ID_WHOLE_TURKEY:
			t.timer = 10;
			t.radius = 20;
			break;
		case ID_DEAD_HORSE:
			t.timer = 20;
			t.radius = 25;
			break;
	}

	return t;
}