#include <stdint.h>
#include <string.h>

#include <debug.h>

#include "inventory.h"

#include "types.h"

struct Item newItem(int8_t type, int8_t id, char name[], char desc[], uint8_t quantity, gfx_sprite_t *icon) {
	static struct Item i;

	i.type = type;
	i.id = id;
	strcpy(i.name, name);
	strcpy(i.description, desc);
	i.quantity = quantity;
	i.icon = icon;

	return i;
}

int8_t getItemIndex(struct Item list[], int8_t id) {
	int8_t i;
	for (i = 0; i < 12; i++) {
		if (list[i].id == id)
			return i;
	}
	return -1;
}