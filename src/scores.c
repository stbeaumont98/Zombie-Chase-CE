#include <stdbool.h>
#include <stdint.h>

#include "graphx.h"
#include "keypadc.h"
#include "draw.h"
#include "types.h"

#include "scores.h"

void draw_scores() {
    bool can_press = false;

    do {
        kb_Scan();
        
		// Black background.
		gfx_FillScreen(COLOR_BLACK);

		draw_custom_text("SCORES", COLOR_WHITE, 200, 3, 4);

		if (!kb_AnyKey()) can_press = true;
		
		gfx_SwapDraw();

    } while (!(can_press && kb_Data[6] & kb_Clear));
}