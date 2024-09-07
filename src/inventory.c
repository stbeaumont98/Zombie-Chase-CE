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

		// Draw inventory cursor.
		gfx_Rectangle_NoClip(163 + (cursor_pos % 3) * 50, 64 + (cursor_pos / 3) * 50, 44, 44);

		// Draw the inventory.
		for (i = 0; i < 9; i++) {
			gfx_SetColor(COLOR_WHITE);
			gfx_Rectangle_NoClip(166 + (i % 3) * 50, 67 + (i / 3) * 50, 38, 38);
			gfx_Rectangle_NoClip(167 + (i % 3) * 50, 68 + (i / 3) * 50, 36, 36);
			if (p->inv[i].id != ID_NONE) {
				gfx_ScaledTransparentSprite_NoClip(p->inv[i].icon, 170 + (i % 3) * 50, 71 + (i / 3) * 50, 2, 2);
				if (p->inv[i].quantity > 1) {
					int offset = 0;
					if (p->inv[i].quantity > 9 && p->inv[i].quantity < 100)
						offset = 8;
					else if (p->inv[i].quantity > 99)
						offset = 16;
					gfx_SetColor(COLOR_BLACK);
					gfx_FillRectangle_NoClip((192 - offset) + (i % 3) * 50, 97 + (i / 3) * 50, 18 + offset, 14);
					gfx_SetColor(COLOR_WHITE);
					gfx_Rectangle_NoClip((191 - offset) + (i % 3) * 50, 96 + (i / 3) * 50, 20 + offset, 16);

					draw_custom_text("x", COLOR_WHITE, (194 - offset) + (i % 3) * 50, 97 + (i / 3) * 50, 2);
					draw_custom_int(p->inv[i].quantity, 1, COLOR_WHITE, (202 - offset) + (i % 3) * 50, 97 + (i / 3) * 50, 2);
				}
			}
		}

		// Draw the player's equipped items
		gfx_Rectangle_NoClip(23, 77, 55, 55);
		gfx_Rectangle_NoClip(24, 78, 53, 53);
		gfx_Rectangle_NoClip(87, 77, 55, 55);
		gfx_Rectangle_NoClip(88, 78, 53, 53);
		gfx_Rectangle_NoClip(23, 141, 55, 55);
		gfx_Rectangle_NoClip(24, 142, 53, 53);

		gfx_ScaledTransparentSprite_NoClip(p->equipped_armor != NULL ? p->equipped_armor->icon : b_frame, 28, 82, 3, 3);
		gfx_ScaledTransparentSprite_NoClip(p->equipped_boots != NULL ? p->equipped_boots->icon : f_frame, 28, 146, 3, 3);
		gfx_ScaledTransparentSprite_NoClip(p->equipped_weapon != NULL ? p->equipped_weapon->icon : h1_frame, 92, 82, 3, 3);

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
			if (kb_Data[7] & kb_Down && cursor_pos < 6) {
				cursor_pos += 3;
				can_press = false;
			} else if (kb_Data[7] & kb_Up && cursor_pos > 2) {
				cursor_pos -= 3;
				can_press = false;
			} else if (kb_Data[7] & kb_Left && cursor_pos > 0) {
				cursor_pos--;
				can_press = false;
			} else if (kb_Data[7] & kb_Right && cursor_pos < 8) {
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
