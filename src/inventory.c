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

	uint8_t cursor_pos = 0;

	do {
		kb_Scan();
		// Black background.
		gfx_FillScreen(COLOR_BLACK);
		draw_custom_text("$", COLOR_WHITE, 10, 3, 4);
		draw_custom_int(p->money, 1, COLOR_WHITE, 26, 2, 4);
		draw_custom_text("INVENTORY", COLOR_WHITE, 165, 3, 4);
		gfx_SetColor(COLOR_WHITE);
		gfx_FillRectangle_NoClip(0, 30, 320, 3);

		// Draw the inventory.
		for (i = 0; i < 12; i++) {
			gfx_SetColor(COLOR_WHITE);
			gfx_Rectangle_NoClip(166 + (i % 3) * 50, 42 + (i / 3) * 50, 38, 38);
			gfx_Rectangle_NoClip(167 + (i % 3) * 50, 43 + (i / 3) * 50, 36, 36);
			if (p->inv[i].id != ID_NONE)
				gfx_ScaledTransparentSprite_NoClip(p->inv[i].icon, 170 + (i % 3) * 50, 46 + (i / 3) * 50, 2, 2);
		}

		// Draw inventory cursor.
		gfx_Rectangle_NoClip(163 + (cursor_pos % 3) * 50, 39 + (cursor_pos / 3) * 50, 44, 44);

		// Draw the player's equipped items
		gfx_Rectangle_NoClip(22, 60, 55, 55);
		gfx_Rectangle_NoClip(23, 61, 53, 53);
		gfx_Rectangle_NoClip(86, 60, 55, 55);
		gfx_Rectangle_NoClip(87, 61, 53, 53);
		gfx_Rectangle_NoClip(22, 124, 55, 55);
		gfx_Rectangle_NoClip(23, 125, 53, 53);

		gfx_ScaledTransparentSprite_NoClip(p->equipped_armor != NULL ? p->equipped_armor->icon : b_frame, 27, 65, 3, 3);
		gfx_ScaledTransparentSprite_NoClip(p->equipped_boots != NULL ? p->equipped_boots->icon : f_frame, 27, 129, 3, 3);
		gfx_ScaledTransparentSprite_NoClip(h1_frame, 91, 65, 3, 3);
		if (p->equipped_weapon != NULL)
			gfx_ScaledTransparentSprite_NoClip(p->equipped_weapon->icon, 91, 65, 3, 3);

		// Check for key presses.
		if (can_press) {

			// Action controls.
			if (kb_Data[1] & kb_2nd || kb_Data[6] & kb_Enter) {
				// Equip an item
				switch (p->inv[cursor_pos].type) {
					case TYPE_NONE:
						break;
					case TYPE_ARMOR:
						p->equipped_armor = &p->inv[cursor_pos];
						break;
					case TYPE_BOOTS:
						p->equipped_boots = &p->inv[cursor_pos];
						break;
					default:
						p->equipped_weapon = &p->inv[cursor_pos];
						break;
				}
				can_press = false;
			}

			// The arrows control the player's selection.
			if (kb_Data[7] & kb_Down && cursor_pos < 9) {
				cursor_pos += 3;
				can_press = false;
			} else if (kb_Data[7] & kb_Up && cursor_pos > 2) {
				cursor_pos -= 3;
				can_press = false;
			} else if (kb_Data[7] & kb_Left && cursor_pos > 0) {
				cursor_pos--;
				can_press = false;
			} else if (kb_Data[7] & kb_Right && cursor_pos < 11) {
				cursor_pos++;
				can_press = false;
			}

			// Open the store from inventory.
			if (kb_Data[1] & kb_Del) {
				if (!from_game)
					break;
				else {
					draw_store(false, p);
					can_press = false;
				}
			}
		}

		if (!kb_AnyKey()) can_press = true;
						
		gfx_SwapDraw();

	} while (!(can_press && (kb_Data[1] & kb_Mode ||  kb_Data[6] & kb_Clear)));
}
