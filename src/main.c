/* Standard libraries */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tice.h>

#include <debug.h>

/* Shared libraries */
#include <graphx.h>
#include <keypadc.h>

#include "gfx\gfx.h"
#include "gfx\zombie_font.h"

#include "types.h"
#include "draw.h"
#include "menu.h"
#include "game.h"

int main() {

	/* Set up graphics */
    gfx_Begin(gfx_8bpp);
	gfx_SetPalette(zombie_palette, sizeof(zombie_palette), 0);
	gfx_SetDrawBuffer();

	/* Set up the font */
    gfx_SetFontData(font);
    gfx_SetFontSpacing(font_spacing);
	gfx_SetFontHeight(6);

	int8_t mode = 0;
	
	while (mode != -1) {
		mode = main_menu();
		if (mode != -1)
			game(mode);
	}
	
    gfx_End();
    pgrm_CleanUp();
	return 0;
}