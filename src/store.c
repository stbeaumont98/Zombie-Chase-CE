
#include <stdint.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>

#include "types.h"
#include "draw.h"
#include "inventory.h"

#include "store.h"

void draw_store(bool from_game, struct Player *p) {
	// Draw the store.
	int i, i_offset = 0, selected_item = 0, quantity = 1, selling_price;
	bool can_press = false;
	bool in_loop = true;
	while (in_loop) {
		kb_Scan();
		// Black background.
		gfx_FillScreen(COLOR_BLACK);
		draw_custom_text("$", COLOR_WHITE, 10, 3, 4);
		draw_custom_int(p->money, 1, COLOR_WHITE, 26, 2, 4);
		draw_custom_text("STORE", COLOR_WHITE, 229, 3, 4);
		gfx_SetColor(COLOR_WHITE);
		gfx_FillRectangle_NoClip(0, 30, 320, 3);

		for (i = 0; i < 6; i++)
			draw_custom_text(store_inv[i + i_offset].name, COLOR_WHITE, 15, 40 + i * 24, (i == selected_item ? 3 : 2));
		
		// Draw the box with the icon inside.
		gfx_SetColor(COLOR_WHITE);
		gfx_Rectangle_NoClip(205, 53, 59, 59);
		gfx_Rectangle_NoClip(206, 54, 57, 57);
		gfx_ScaledTransparentSprite_NoClip(store_inv[selected_item + i_offset].icon, 212, 60, 3, 3);

		// Draw the quantity 
		draw_custom_text("QTY: <   >", COLOR_WHITE, 198, 118, 2);
		draw_custom_int(quantity, 2, COLOR_WHITE, 248, 118, 2);
				
		// Calculate the selling price and display it underneath the quantity.
		selling_price = store_inv[selected_item + i_offset].price * quantity;
		draw_custom_text("$", p->money < selling_price ? COLOR_DARK_RED : COLOR_GREEN, 217, 137, 3);
		draw_custom_int(selling_price, 1, p->money < selling_price ? COLOR_DARK_RED : COLOR_GREEN, 229, 136, 3);

		gfx_Rectangle_NoClip(25, 189, 270, 40);
		draw_custom_text(store_inv[selected_item + i_offset].description, COLOR_WHITE, 28, 190, 2);

		// Check for key presses.
		if (can_press) {

			// Buttons to break the loop.
			if (kb_Data[1] & kb_Del || kb_Data[6] & kb_Clear)
				in_loop = false;

			// Action controls
			if (kb_Data[1] & kb_2nd || kb_Data[6] & kb_Enter) {
				if (p->money >= selling_price && p->inv_count < 10) {
					p->money -= selling_price;
					// Check if the user already has at least one of that item
					int item_index = player_has_item(p->inv, store_inv[selected_item + i_offset].id);
					if (item_index != -1) {
						// If so, add to the quantity owned by the player
						p->inv[item_index]->quantity += quantity;
					} else {
						// Otherwise, find the next non-empty slot and put the new item there.
						p->inv[p->inv_count] = (struct Item *) malloc(sizeof(struct Item));
						p->inv[p->inv_count]->type = store_inv[selected_item + i_offset].type;
						p->inv[p->inv_count]->id = store_inv[selected_item + i_offset].id;
						strcpy(p->inv[p->inv_count]->name, store_inv[selected_item + i_offset].name);
						strcpy(p->inv[p->inv_count]->description, store_inv[selected_item + i_offset].description);
						p->inv[p->inv_count]->quantity = quantity;
						p->inv[p->inv_count]->icon = store_inv[selected_item + i_offset].icon;

						p->equipped_weapon = p->inv[0];

						p->inv_count++;
					}
				}
				can_press = false;
			}

			// Up and down controls the menu option, left and right controls the quantity of the item.
			if (kb_Data[7] & kb_Down) {
				selected_item++;
				quantity = 1;
				can_press = false;
			} else if (kb_Data[7] & kb_Up) {
				selected_item--;
				quantity = 1;
				can_press = false;
			} else if (kb_Data[7] & kb_Left) {
				if (quantity > 1)
					quantity--;
				can_press = false;
			} else if (kb_Data[7] & kb_Right) {
				if (quantity < store_inv[selected_item + i_offset].quantity)
					quantity++;
				can_press = false;
			}

			// Open inventory from the store.
			if (kb_Data[1] & kb_Mode) {
				if (!from_game)
					in_loop = false;
				else {
					draw_inventory(false, p);
					can_press = false;
				}
			}
		}

		if (selected_item < 0 && i_offset > 0) {
			i_offset--;
			selected_item = 0;
		} else if (selected_item > 5 && i_offset < 10) {
			i_offset++;
			selected_item = 5;
		} else if (selected_item < 0) {
			selected_item = 0;
		} else if (selected_item > 5) {
			selected_item = 5;
		}

		if (!kb_AnyKey()) can_press = true;
						
		gfx_SwapDraw();
	}
}