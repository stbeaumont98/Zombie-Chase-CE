
#include <stdint.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>

#include <debug.h>

#include "types.h"
#include "draw.h"
#include "inventory.h"

#include "store.h"

void draw_store(bool from_game, struct Player *p) {
	// Draw the store.
    uint8_t i, quantity = 1;
	int8_t i_offset = 0, selected_item = 0;
	uint16_t selling_price;
	bool can_press = false;
	do {
		kb_Scan();
		// Black background.
		gfx_FillScreen(COLOR_BLACK);
		draw_custom_text("$", COLOR_WHITE, 10, 3, 4);
		draw_custom_int(p->money, 1, COLOR_WHITE, 26, 2, 4);
		draw_custom_text("STORE", COLOR_WHITE, 229, 3, 4);
		gfx_SetColor(COLOR_WHITE);
		gfx_FillRectangle_NoClip(0, 30, 320, 3);

		for (i = 0; i < 6; i++)
			draw_custom_text(store_inv[i + i_offset].name, COLOR_WHITE, 25, 40 + i * 24 - (i == selected_item ? 3 : 0), (i == selected_item ? 3 : 2));
		
		// Draw the box with the icon inside.
		gfx_SetColor(COLOR_WHITE);
		gfx_Rectangle_NoClip(226, 53, 59, 59);
		gfx_Rectangle_NoClip(227, 54, 57, 57);
		gfx_ScaledTransparentSprite_NoClip(store_inv[selected_item + i_offset].icon, 233, 60, 3, 3);

		// Draw the quantity 
		draw_custom_text("QTY: <   >", COLOR_WHITE, 219, 118, 2);
		draw_custom_int(quantity, 2, COLOR_WHITE, 269, 118, 2);
				
		// Calculate the selling price and display it underneath the quantity.
		selling_price = store_inv[selected_item + i_offset].price * quantity;
		draw_custom_text("$", p->money < selling_price ? COLOR_DARK_RED : COLOR_GREEN, 238, 137, 3);
		draw_custom_int(selling_price, 1, p->money < selling_price ? COLOR_DARK_RED : COLOR_GREEN, 250, 136, 3);

		gfx_Rectangle_NoClip(20, 184, 280, 50);
		draw_custom_text(store_inv[selected_item + i_offset].description, COLOR_WHITE, 28, 190, 2);

		// Check for key presses.
		if (can_press) {

			// Action controls
			if (kb_Data[1] & kb_2nd || kb_Data[6] & kb_Enter) {
				if (p->money >= selling_price && get_item_index(p->inv, ID_NONE) != -1) {
					p->money -= selling_price;
					// Check if the user already has at least one of that item
					int item_index = get_item_index(p->inv, store_inv[selected_item + i_offset].id);
					dbg_printf("Item index found!\nIndex: %d\n", item_index);
					if (item_index != -1) {
						// If so, add to the quantity owned by the player
						p->inv[item_index].quantity += quantity;
					} else {

						// Find the next empty slot
						int new_index = get_item_index(p->inv, ID_NONE);

						p->inv[new_index] = new_item(
								store_inv[selected_item + i_offset].type,
								store_inv[selected_item + i_offset].id,
								store_inv[selected_item + i_offset].name,
								store_inv[selected_item + i_offset].description,
								quantity,
								store_inv[selected_item + i_offset].icon
							);
						
						dbg_printf("Item added!\n");

						switch (p->inv[new_index].type) {
							case TYPE_NONE:
								break;
							case TYPE_ARMOR:
								if (p->equipped_armor == NULL)
									p->equipped_armor = &p->inv[new_index];
								break;
							case TYPE_BOOTS:
								if (p->equipped_boots == NULL)
									p->equipped_boots = &p->inv[new_index];
								break;
							default:
								if (p->equipped_weapon == NULL)
									p->equipped_weapon = &p->inv[new_index];
								break;
						}
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
					break;
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
	} while (!(can_press && (kb_Data[1] & kb_Del || kb_Data[6] & kb_Clear)));
}