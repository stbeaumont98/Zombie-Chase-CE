
#include <stdint.h>

#include <graphx.h>
#include <keypadc.h>

#include "types.h"
#include "draw.h"
#include "store.h"

#include "inventory.h"

void draw_inventory(bool from_game, struct Player *p) {
	// Draw the player's inventory.
	uint8_t i;
	bool can_press = false;
	bool in_loop = true;

	uint8_t cursor_pos = 0;

	while (in_loop) {
		kb_Scan();
		// Black background.
		gfx_FillScreen(COLOR_BLACK);
		draw_custom_text("$", COLOR_WHITE, 10, 3, 4);
		draw_custom_int(p->money, 1, COLOR_WHITE, 26, 2, 4);
		draw_custom_text("INVENTORY", COLOR_WHITE, 165, 3, 4);
		gfx_SetColor(COLOR_WHITE);
		gfx_FillRectangle_NoClip(0, 30, 320, 3);

		// Draw the inventory.
		for (i = 0; i < 10; i++) {
			gfx_SetColor(COLOR_WHITE);
			gfx_Rectangle_NoClip(41 + (i % 5) * 50, 45 + (i / 5) * 50, 38, 38);
			gfx_Rectangle_NoClip(42 + (i % 5) * 50, 46 + (i / 5) * 50, 36, 36);
			if (p->inv[i] != NULL)
				gfx_ScaledTransparentSprite_NoClip(p->inv[i]->icon, 45 + (i % 5) * 50, 49 + (i / 5) * 50, 2, 2);
		}

		// Draw inventory cursor
		gfx_Rectangle_NoClip(38 + (cursor_pos % 5) * 50, 42 + (cursor_pos / 5) * 50, 44, 44);

		// Check for key presses.
		if (can_press) {

			// Buttons to break the loop.
			if (kb_Data[1] & kb_Mode || kb_Data[6] & kb_Clear)
				in_loop = false;

			// Action controls.
			if (kb_Data[1] & kb_2nd || kb_Data[6] & kb_Enter) {
				// Equip an item
				can_press = false;
			}

			// The arrows control the player's selection.
			if (kb_Data[7] & kb_Down && cursor_pos < 5) {
				cursor_pos += 5;
				can_press = false;
			} else if (kb_Data[7] & kb_Up && cursor_pos > 4) {
				cursor_pos -= 5;
				can_press = false;
			} else if (kb_Data[7] & kb_Left && cursor_pos > 0) {
				cursor_pos--;
				can_press = false;
			} else if (kb_Data[7] & kb_Right && cursor_pos < 9) {
				cursor_pos++;
				can_press = false;
			}

			// Open the store from inventory.
			if (kb_Data[1] & kb_Del) {
				if (!from_game)
					in_loop = false;
				else {
					draw_store(false, p);
					can_press = false;
				}
			}
		}

		if (!kb_AnyKey()) can_press = true;
						
		gfx_SwapDraw();
	}
}

int player_has_item(struct Item *inventory[10], uint8_t id) {
	int i;
	for (i = 0; i < 10; i++) {
		if (inventory[i]->id == id)
			return i;
	}
	return -1;
}
