#include <stdint.h>
#include <string.h>

#include <graphx.h>

#include "gfx\gfx.h"
#include "types.h"

#include "draw.h"

void draw_player(uint16_t x, uint8_t y) {
	gfx_SetColor(COLOR_WHITE);
    gfx_FillCircle_NoClip(x + 2, y + 2, 2);
}

void draw_health_pack(uint16_t x, uint8_t y) {
    gfx_SetColor(COLOR_RED);
    gfx_FillRectangle_NoClip(x + 2, y, 2, 6);
    gfx_FillRectangle_NoClip(x, y + 2, 6, 2);
}

void draw_custom_text(char* text, uint8_t color, uint16_t x, uint8_t y, int scale) {
	gfx_SetTextFGColor(color);
    gfx_SetTextBGColor(COLOR_RED);
    gfx_SetTextTransparentColor(COLOR_RED);
    gfx_SetTextXY(x, y);
	gfx_SetTextScale(scale, scale);

	if (x + strlen(text) * 4 * scale > GFX_LCD_WIDTH) {
		char tmp[0xFF];
		strcpy(tmp, text);
		char *token = strtok(tmp, "|");
		while (token != NULL) {
			gfx_PrintString(token);
			token = strtok(NULL, "|");
			y += (6 * scale);
			gfx_SetTextXY(x, y);
		}
	} else 
		gfx_PrintString(text);
}

void draw_custom_int(int i, uint8_t length, uint8_t color, uint16_t x, uint8_t y, int scale) {
	gfx_SetTextFGColor(color);
    gfx_SetTextBGColor(COLOR_RED);
    gfx_SetTextTransparentColor(COLOR_RED);
    gfx_SetTextXY(x, y);
	gfx_SetTextScale(scale, scale);
	gfx_PrintInt(i, length);
}

void draw_fail(void) {
    gfx_ScaledTransparentSprite_NoClip(fail, 73, 76, 6, 6);
	draw_custom_text(fail_string, COLOR_WHITE, 57, 148, 2);
}