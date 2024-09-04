#include <stdint.h>
#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include "gfx/gfx.h"
#include "types.h"
#include "draw.h"

#include "menu.h"

void draw_menu() {
    uint8_t i;
    int8_t selected_item = 0;
	int old_time, time, one_second;
    bool can_press = false;
    bool in_loop = true;
    bool text_flash = true;
    
	time = rtc_Time();

    while (in_loop) {
		kb_Scan();
		
		old_time = time;
		time = rtc_Time();
		one_second = time - old_time;

        if (one_second)
            text_flash = !text_flash;


		// Black background.
		gfx_FillScreen(COLOR_BLACK);

        // Title
        gfx_ScaledTransparentSprite_NoClip(title, 107, 25, 4, 4);

        for (i = 0; i < 3; i++)
            draw_custom_text(menu_options[i], COLOR_WHITE, 25, 120 + i * 24 - (i == selected_item ? 3 : 0), (i == selected_item ? 3 : 2));

        if (text_flash)
	        draw_custom_text(quit_string, COLOR_WHITE, 79, 213, 2);

        if (can_press) {
			// Buttons to break the loop.
			if (kb_Data[6] & kb_Clear)
				in_loop = false;

            // Up and down controls the menu option.
			if (kb_Data[7] & kb_Down) {
				selected_item++;
				can_press = false;
			} else if (kb_Data[7] & kb_Up) {
				selected_item--;
				can_press = false;
            }
        }

        if (selected_item > 2)
            selected_item = 2;
        else if (selected_item < 0)
            selected_item = 0;

		if (!kb_AnyKey()) can_press = true;
						
		gfx_SwapDraw();
    }
}