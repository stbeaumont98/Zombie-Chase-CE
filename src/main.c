/* Standard libraries */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tice.h>

/* Shared libraries */
#include <graphx.h>
#include <keypadc.h>

#include "gfx\gfx.h"
#include "gfx\zombie_font.h"

#define COLOR_RED 0x00
#define COLOR_GREEN 0x01
#define COLOR_DARK_GREEN 0x02
#define COLOR_DARK_RED 0x03
#define COLOR_WHITE 0x05
#define COLOR_BLACK 0x06
#define COLOR_LIGHT_RED 0x07

/* Function prototypes */
void drawPlayer(uint16_t x, uint8_t y);
void drawHealthPack(uint16_t x, uint8_t y);
void drawZombie(uint16_t x, uint8_t y);
void drawCustomText(char* text, uint8_t color, int x, int y, int scale);
void drawFail();

typedef struct Player
{
	uint16_t x;
	uint8_t y;
	int health;
} Player;

typedef struct Zombie {
	uint16_t x;
	uint8_t y;
	Player *target;
} Zombie;

char fail_string[] = "PRESS [MODE] TO PLAY AGAIN";

int main() {
	
	Zombie z[0xFF];					// Up to 255 zombies can be on screen at once.
	
    kb_key_t key;					// Variable to store keypad input.
    uint8_t zombie_spawn_timer;		// Timer for zombies to spawn.
    uint16_t hp_x;					// Health pack x and y.
    uint8_t hp_y;
    uint8_t zombie_count = 1;		// Number of zombies currently spawned.
    uint16_t points = 0;			// Points obtained by the player.

	/* Initialize the player */
	Player p;
    p.x = 156;
    p.y = 232;
    p.health = 200;
	
	uint8_t i;
	bool infected = false;
	
	int old_time, time, one_second;

    srand(rtc_Time());

	/* Set up graphics */
    gfx_Begin(gfx_8bpp);
	gfx_SetPalette(zombie_palette, sizeof(zombie_palette), 0);
	gfx_SetDrawBuffer();

	/* Set up the font */
    gfx_SetFontData(font);
    gfx_SetFontSpacing(font_spacing);
	gfx_SetFontHeight(6);
    
    zombie_spawn_timer = rand() % 2 + 3;
    hp_x = rand() % 310 + 2;
    hp_y = rand() % 230 + 2;
	
	for (i = 0; i < 0xFF; i++) {
		z[i].x = rand() % 310 + 2;
		z[i].y = rand() % 230 + 2;
		z[i].target = NULL;
	}
	
	z[0].target = &p; // The first zombie goes after the player.
    
	time = rtc_Time();
	
    do { // Game loop

		kb_Scan();
		
		old_time = time;
		time = rtc_Time();
		one_second = time - old_time;
		
		/* Black background */
        gfx_FillScreen(COLOR_BLACK);

        /* Draw the health bar */
		gfx_SetColor(COLOR_WHITE);				// White
		gfx_Rectangle_NoClip(58, 3, 204, 9);	// Draw the outline of the health bar.
        gfx_SetColor(COLOR_RED);				// Health color red.

        if (p.health >= 1) {
            gfx_FillRectangle_NoClip(60, 5, p.health, 5);
			gfx_SetColor(COLOR_LIGHT_RED);
			gfx_HorizLine_NoClip(60, 5, p.health);
			gfx_SetColor(COLOR_DARK_RED);
			gfx_FillRectangle_NoClip(60, 8, p.health, 2);

			if (infected && one_second) {
				if (p.health >= 4)
					p.health-=4;
				else
					p.health = 0;
			}
        }
        
        drawPlayer(p.x, p.y);
		drawHealthPack(hp_x, hp_y);
        gfx_SetTextFGColor(COLOR_WHITE);
        gfx_SetTextBGColor(COLOR_BLACK);
        gfx_SetTextTransparentColor(COLOR_BLACK);
        gfx_SetTextXY(2, 226);
		gfx_SetTextScale(2, 2);
        gfx_PrintInt(points, 3);
		
        for (i = 0; i < zombie_count; i++) {
			if (z[i].target != NULL) {
				drawZombie(z[i].x, z[i].y);

				// Zombie moves toward its target.
				if (z[i].x < z[i].target->x && rand() & 1)
					z[i].x += 2;
				if (z[i].x > z[i].target->x && rand() & 1)
					z[i].x -= 2;
				if (z[i].y < z[i].target->y && rand() & 1)
					z[i].y += 2;
				if (z[i].y > z[i].target->y && rand() & 1)
					z[i].y -= 2;

				// Zombie bounds; They only go as far as the player, but this code is just in case.
				if (z[i].x < 2)
					z[i].x = 2;
				if (z[i].x > 312)
					z[i].x = 312;
				if (z[i].y < 2)
					z[i].y = 2;
				if (z[i].y > 232)
					z[i].y = 232;

				/* Zombie/Player collisions */
				if ((p.x < z[i].x + 6) && (p.x + 6 > z[i].x) && (p.y < z[i].y + 6) && (6 + p.y > z[i].y)) {
					p.health--;
					if (!infected)
						infected = 1;
				}

			}
        }
		
		
        if (zombie_spawn_timer == 0 && zombie_count < 0xFF) {
			z[zombie_count % 0xFF].x = rand() % 310 + 2;
			z[zombie_count % 0xFF].y = rand() % 230 + 2;
			z[zombie_count % 0xFF].target = &p;
			zombie_count++;
            zombie_spawn_timer = rand() % 2 + 3;
        }
        
        /* Process key input */

		if (p.health > 0) {

       		key = kb_Data[7];

			if (key & kb_Left)
				p.x -= 2;
			if (key & kb_Right)
				p.x += 2;
			if (key & kb_Up)
				p.y -= 2;
			if (key & kb_Down)
				p.y += 2;
			
			if (key & kb_Up & kb_Left) {
				p.y += 1;
				p.x -= 1;
			}
			if (key & kb_Up & kb_Right) {
				p.y += 1;
				p.x += 1;
			}
			if (key & kb_Down & kb_Left) {
				p.y -= 1;
				p.x -= 1;
			}
			if (key & kb_Down & kb_Right) {
				p.y -= 1;
				p.x += 1;
			}
		}

		// Player bounds.
		if (p.x < 2)
			p.x = 2;
		if (p.x > 312)
			p.x = 312;
		if (p.y < 2)
			p.y = 2;
		if (p.y > 232)
			p.y = 232;

        /* Health pack collisions */
        if ((p.x < hp_x + 6) && (p.x + 5 > hp_x) && (p.y < hp_y + 6) && (5 + p.y > hp_y)) {
			if (infected)
				p.health += 10;
			else 
				p.health += 5;
			hp_x = rand() % 310 + 2;
			hp_y = rand() % 230 + 2;
			points++;
        }

		/* Health cannot exceed 200 */
		if (p.health > 200)
			p.health = 200;
        
        /* Check if the player has died. */
        if (p.health <= 0) {
			p.health = 0;
            drawFail();
			if (kb_Data[1] & kb_Mode) {
				for (i = 0; i < zombie_count; i++) {
					z[i].x = 0;
					z[i].y = 0;
					z[i].target = &p;
				}
				z[0].x = rand() % 310 + 2;
				z[0].y = rand() % 230 + 2;
				z[0].target = &p;
				points = 0;
				zombie_count = 1;
				zombie_spawn_timer = rand() % 2 + 3;
				p.x = 156;
				p.y = 232;
				hp_x = rand() % 310 + 2;
				hp_y = rand() % 230 + 2;
				p.health = 200;
				infected = false;
            }
        }
        
		// Decrement the zombie spawner every second.
		if (one_second)
        	zombie_spawn_timer--;

        gfx_SwapDraw();

    } while (!(kb_Data[6] & kb_Clear));

    gfx_End();
    pgrm_CleanUp();
	return 0;
}

void drawPlayer(uint16_t x, uint8_t y) {
	gfx_SetColor(COLOR_WHITE);
    gfx_FillRectangle_NoClip(x + 1, y, 3, 5);
    gfx_FillRectangle_NoClip(x, y + 1, 5, 3);
}

void drawHealthPack(uint16_t x, uint8_t y) {
    gfx_SetColor(COLOR_RED);
    gfx_FillRectangle_NoClip(x + 2, y, 2, 6);
    gfx_FillRectangle_NoClip(x, y + 2, 6, 2);
}

void drawZombie(uint16_t x, uint8_t y) {
    gfx_SetColor(COLOR_DARK_GREEN);
	gfx_FillRectangle_NoClip(x, y, 4, 4);
	gfx_FillRectangle_NoClip(x + 2, y + 2, 4, 4);
	gfx_SetColor(COLOR_GREEN);
	gfx_FillRectangle_NoClip(x + rand() % 4, y + rand() % 4, 2, 2);
	gfx_SetColor(COLOR_DARK_RED);
	gfx_FillRectangle_NoClip(x + rand() % 4, y + rand() % 4, 2, 2);
}

void drawCustomText(char* text, uint8_t color, int x, int y, int scale) {
	gfx_SetTextFGColor(color);
    gfx_SetTextBGColor(COLOR_RED);
    gfx_SetTextTransparentColor(COLOR_RED);
    gfx_SetTextXY(x, y);
	gfx_SetTextScale(scale, scale);
	gfx_PrintString(text);
}

void drawFail() {
    gfx_ScaledTransparentSprite_NoClip(fail, 73, 76, 6, 6);
	drawCustomText(fail_string, COLOR_WHITE, 57, 148, 2);
}