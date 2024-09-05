#include <stdint.h>
#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include "gfx/gfx.h"
#include "types.h"
#include "draw.h"
#include "scores.h"

#include "menu.h"

int8_t main_menu() {
    uint8_t i;
    int8_t selected_item = 0;
    bool can_press = false;

    do {
		kb_Scan();

		// Black background.
		gfx_FillScreen(COLOR_BLACK);

        // Title
        gfx_ScaledTransparentSprite_NoClip(title, 107, 25, 4, 4);

        gfx_ScaledSprite_NoClip(menu_player, 249, 141, 2, 2);
        gfx_ScaledSprite_NoClip(menu_zombie1, 180, 139, 2, 2);
        gfx_ScaledSprite_NoClip(menu_zombie2, 116, 159, 2, 2);

        for (i = 0; i < 3; i++)
            draw_custom_text(menu_options[i], COLOR_WHITE, 25, 124 + i * 24 - (i == selected_item ? 3 : 0), (i == selected_item ? 3 : 2));

	    draw_custom_text(quit_string, COLOR_WHITE, 79, 213, 2);

        if (can_press) {

            // Up and down controls the menu option.
			if (kb_Data[7] & kb_Down) {
				selected_item++;
				can_press = false;
			} else if (kb_Data[7] & kb_Up) {
				selected_item--;
				can_press = false;
            }
            
			if (kb_Data[1] & kb_2nd || kb_Data[6] & kb_Enter) {
                switch (selected_item) {
                    case CLASSIC:
                    case ENHANCED:
                        return selected_item;
                    case SCORES:
                        draw_scores();
                        break;
                    default:
                        break;
                }
                can_press = false;
            }
        }

        if (selected_item > 2)
            selected_item = 2;
        else if (selected_item < 0)
            selected_item = 0;

		if (!kb_AnyKey()) can_press = true;
						
		gfx_SwapDraw();

    } while (!(can_press && kb_Data[6] & kb_Clear));

    return -1;
}