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
#define COLOR_BEIGE 0x08
#define COLOR_GRAY 0x09

/* Target types */
#define TYPE_GRENADE 0
#define TYPE_C4 1
#define TYPE_LAND_MINE 2
#define TYPE_LURE 3

/* Item types */
#define ID_MACHETE 0			// A machete can be swung all around and kill some zombies within its reach.
#define ID_KATANA 1				// A katana can also be swung around, but its range is bigger.
#define ID_GRENADE 2			// Grenades explode after an amount of time passes and then kills the zombies lured to it and that's about it.
#define ID_C4 3					// C4 explodes at the players command and kills the zombies lured to it and zombies within a medium-sized radius.
#define ID_LAND_MINE 4			// Land mines explode on contact killing anything within its medium-sized blast radius.
#define ID_THE_BIG_ONE 5		// The big one kills all the zombies currently on the screen.
#define ID_TBONE_STEAK 6			// Lures some zombies so the player may have some time to collect more things.
#define ID_MEDIUM_LURE 7		// Lures more zombies than the small lure for a longer time period.
#define ID_LARGE_LURE 8			// Lures more zombies than both the small and medium lures for an even longer time period.
#define ID_CARDBOARD_ARMOR 9	// Cardboard armor protects the player from a few bites but falls apart quickly.
#define ID_PLASTIC_ARMOR 10		// Plastic armor can take a bit more damage than cardboard armor.
#define ID_STEEL_ARMOR 11		// Steel armor protects the player from more bites but slows the player down.
#define ID_FORCEFIELD_ARMOR 12	// Forcefield armor protects the player from all bites for 15 seconds.
#define ID_CAMOUFLAGE_ARMOR 13	// Camouflage armor makes the player invisible to zombies for a period of time.
#define ID_LIGHTWEIGHT_BOOTS 14	// Lightweight boots make the player move faster but can be damaged by a few bites.
#define ID_HEAVYWEIGHT_BOOTS 15	// Heavyweight boots make the player move faster and can be damaged by more bites.

struct Item {
	uint16_t id;
	char name[20];
	char description[0xFF];
	uint16_t price;
	gfx_sprite_t *icon;
};

struct Target {
	uint16_t x;
	uint8_t y;
	uint8_t timer;
	uint8_t radius;
	uint8_t type;
	bool alive;
};

struct Player {
	uint16_t x;
	uint8_t y;
	int health;
	struct Item inventory[25];
	struct Item *equipped_weapon;
	struct Item *equipped_armor;
	struct Item *equipped_boots;
};

struct Zombie {
	uint16_t x;
	uint8_t y;
	struct Target *target;
	bool alive;
};

/* Function prototypes */
void draw_player(uint16_t x, uint8_t y);
void draw_health_pack(uint16_t x, uint8_t y);
void draw_zombie(uint16_t x, uint8_t y);
void draw_custom_text(char* text, uint8_t color, int x, int y, int scale);
void draw_custom_int(int i, uint8_t color, int x, int y, int scale);
void draw_fail(void);
void draw_store(bool can_press);
bool is_in_radius(struct Target *t, struct Zombie z);

char fail_string[] = "PRESS [MODE] TO PLAY AGAIN";
char status_string[] = "";

static struct Item store_inv[16] = {		// Items that can be bought in the store.
	{ID_MACHETE, "Machete", "A machete can be swung all around and kill some zombies within its reach.", 20, machete},
	{ID_KATANA, "Katana", "A katana can also be swung around, but its range is bigger.", 50, katana},
	{ID_GRENADE, "Grenade", "Grenades explode after an amount of time passes and then kills the zombies lured to it and that's about it.", 15, grenade},
	{ID_C4, "C4", "C4 explodes at the players command and kills the zombies lured to it and zombies within a medium-sized radius.", 25, c4},
	{ID_LAND_MINE, "Land Mine", "Land mines explode on contact killing anything within its medium-sized blast radius.", 40, land_mine},
	{ID_THE_BIG_ONE, "The Big One", "The big one kills all the zombies currently on the screen.", 1000, the_big_one},
	{ID_TBONE_STEAK, "T-Bone Steak", "Lures some zombies so the player may have some time to collect more things.", 20, t_bone},
	{ID_MEDIUM_LURE, "Medium Lure", "Lures more zombies than the small lure for a longer time period.", 50, NULL},
	{ID_LARGE_LURE, "Large Lure", "Lures more zombies than both the small and medium lures for an even longer time period.", 100, NULL},
	{ID_CARDBOARD_ARMOR, "Cardboard Armor", "Cardboard armor protects the player from a few bites but falls apart quickly.", 20, NULL},
	{ID_PLASTIC_ARMOR, "Plastic Armor", "Plastic armor can take a bit more damage than cardboard armor.", 50, NULL},
	{ID_STEEL_ARMOR, "Steel Armor", "Steel armor protects the player from more bites but slows the player down.", 100, NULL},
	{ID_FORCEFIELD_ARMOR, "Forcefield Armor", "Forcefield armor protects the player from all bites for 15 seconds.", 100, NULL},
	{ID_CAMOUFLAGE_ARMOR, "Camouflage Armor", "Camouflage armor makes the player invisible to zombies for a period of time.", 100, NULL},
	{ID_LIGHTWEIGHT_BOOTS, "Lightweight Boots", "Lightweight boots make the player move faster but can be damaged by a few bites.", 20, NULL},
	{ID_HEAVYWEIGHT_BOOTS, "Heavyweight Boots", "Heavyweight boots make the player move faster and can be damaged by more bites.", 50, NULL},
};

int main() {
	
	struct Zombie z[0xFF];			// Up to 255 zombies can be on screen at once.
	
    kb_key_t key;					// Variable to store keypad input.
    uint8_t zombie_spawn_timer;		// Timer for zombies to spawn.
    uint16_t hp_x;					// Health pack x and y.
    uint8_t hp_y;
	uint8_t zombie_rand = 0;
    uint8_t zombie_count = 1;		// Number of zombies currently spawned.
    uint16_t money = 0;				// Money obtained by the player.
	uint16_t points = 0;			// Points obtained by the player.

	/* Define the player */
	struct Player p;
    p.x = 156;
    p.y = 232;
    p.health = 200;
	p.equipped_weapon = &store_inv[3];
	
	int i;
	bool infected = false;
	
	int old_time, time, one_second;
	
	bool can_press = false;

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
		z[i].alive = false;
	}
    
	z[0].alive = true;

	time = rtc_Time();
	
    do { // Game loop

		kb_Scan();
		
		old_time = time;
		time = rtc_Time();
		one_second = time - old_time;
		
		/* Black background */
        gfx_FillScreen(COLOR_BLACK);
        
		draw_custom_text(status_string, COLOR_WHITE, 0, 0, 1);

        /* Draw the health bar */
		gfx_SetColor(COLOR_WHITE);
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
					p.health -= 4;
				else
					p.health = 0;
			}
        }
        
        draw_player(p.x, p.y);
		draw_health_pack(hp_x, hp_y);
		/* Draw the players money */
        draw_custom_text("$", COLOR_WHITE, 2, 2, 2);
		draw_custom_int(money, COLOR_WHITE, 10, 1, 2);
		draw_custom_int(points, COLOR_WHITE, 2, 226, 2);
		
        for (i = zombie_count; i >= 0; i--) {
			if (z[i].alive) {
				draw_zombie(z[i].x, z[i].y);
				if (z[i].target == NULL) {
					// Zombie chases the player.
					if (z[i].x < p.x && rand() & 1)
						z[i].x += 2;
					if (z[i].x > p.x && rand() & 1)
						z[i].x -= 2;
					if (z[i].y < p.y && rand() & 1)
						z[i].y += 2;
					if (z[i].y > p.y && rand() & 1)
						z[i].y -= 2;
				} else {
					// Zombie chases the target.
					if (z[i].x < z[i].target->x && rand() & 1)
						z[i].x += 2;
					if (z[i].x > z[i].target->x && rand() & 1)
						z[i].x -= 2;
					if (z[i].y < z[i].target->y && rand() & 1)
						z[i].y += 2;
					if (z[i].y > z[i].target->y && rand() & 1)
						z[i].y -= 2;
					
					switch (z[i].target->type) {
						case TYPE_GRENADE:
							gfx_SetColor(COLOR_DARK_GREEN);
							gfx_FillRectangle_NoClip(z[i].target->x, z[i].target->y, 4, 4);
							draw_custom_int(z[i].target->timer, COLOR_WHITE, z[i].target->x + 4, z[i].target->y - 7, 1);
							if (one_second && z[i].target->timer > 0) {
								z[i].target->timer--;
							}
							break;
						case TYPE_LURE:
							gfx_SetColor(COLOR_DARK_RED);
							gfx_FillRectangle_NoClip(z[i].target->x, z[i].target->y, 4, 4);
							draw_custom_int(z[i].target->timer, COLOR_WHITE, z[i].target->x + 4, z[i].target->y - 7, 1);
							if (one_second && z[i].target->timer > 0) {
								z[i].target->timer--;
							}
							break;
						case TYPE_C4:
							gfx_SetColor(COLOR_BEIGE);
							gfx_FillRectangle_NoClip(z[i].target->x, z[i].target->y, 4, 4);
							draw_custom_text("[2nd]", COLOR_WHITE, z[i].target->x, z[i].target->y - 7, 1);
							if (can_press && kb_Data[2] & kb_Alpha)
								z[i].target->timer--;
							break;
						case TYPE_LAND_MINE:
							gfx_SetColor(COLOR_GRAY);
							gfx_FillRectangle_NoClip(z[i].target->x, z[i].target->y, 4, 4);
							if ((z[i].target->x < z[i].x + 6) && (z[i].target->x + 6 > z[i].x) && (z[i].target->y < z[i].y + 6) && (6 + z[i].target->y > z[i].y))
								z[i].target->timer--;
							break;
					}

					if (z[i].target->timer <= 0) {
						if (z[i].target->type == TYPE_GRENADE || z[i].target->type == TYPE_C4 || z[i].target->type == TYPE_LAND_MINE) {
							zombie_count--;
							z[i].alive = false;
						}
						free(z[i].target);
						z[i].target = NULL;
					}
				}

				// Zombie bounds.
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
			z[zombie_count % 0xFF].alive = true;
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

			if (can_press) {
				if (kb_Data[1] & kb_Mode) {
					draw_store(can_press);
					can_press = false;
				}
				if (kb_Data[1] & kb_2nd) {
					switch (p.equipped_weapon->id) {
						case ID_GRENADE:
							zombie_rand = rand() % zombie_count;
							for (i = zombie_rand; i < zombie_count; i++) {
								if (z[i].target == NULL) {
									z[i].target = (struct Target *) malloc(sizeof(struct Target));
									z[i].target->type = TYPE_GRENADE;
									z[i].target->x = p.x;
									z[i].target->y = p.y;
									z[i].target->timer = 5;
									z[i].target->radius = 15;
								}
							}
							break;
						case ID_C4:
							zombie_rand = rand() % zombie_count;
							for (i = zombie_rand; i < zombie_count; i++) {
								if (z[i].target == NULL) {
									z[i].target = (struct Target *) malloc(sizeof(struct Target));
									z[i].target->type = TYPE_C4;
									z[i].target->x = p.x;
									z[i].target->y = p.y;
									z[i].target->timer = 5;
									z[i].target->radius = 30;
								}
							}
							break;
						case ID_LAND_MINE:
							zombie_rand = rand() % zombie_count;
							for (i = zombie_rand; i < zombie_count; i++) {
								if (z[i].target == NULL) {
									z[i].target = (struct Target *) malloc(sizeof(struct Target));
									z[i].target->type = TYPE_LAND_MINE;
									z[i].target->x = p.x;
									z[i].target->y = p.y;
									z[i].target->timer = 5;
									z[i].target->radius = 60;
								}
							}
							break;
						case ID_THE_BIG_ONE:
							for (i = 0; i < zombie_count; i++) {
								z[i].alive = false;
								z[i].x = rand() % 310 + 2;
								z[i].y = rand() % 230 + 2;
							}
							zombie_spawn_timer = rand() % 5 + 5;
							break;
						case ID_TBONE_STEAK:
							zombie_rand = rand() % zombie_count;
							for (i = zombie_rand; i < zombie_count; i++) {
								if (z[i].target == NULL) {
									z[i].target = (struct Target *) malloc(sizeof(struct Target));
									z[i].target->type = TYPE_LURE;
									z[i].target->x = p.x;
									z[i].target->y = p.y;
									z[i].target->timer = 5;
								}
							}
							break;
						case ID_MEDIUM_LURE:
							zombie_rand = rand() % zombie_count;
							for (i = zombie_rand; i < zombie_count; i++) {
								if (z[i].target == NULL) {
									z[i].target = (struct Target *) malloc(sizeof(struct Target));
									z[i].target->type = TYPE_LURE;
									z[i].target->x = p.x;
									z[i].target->y = p.y;
									z[i].target->timer = 10;
								}
							}
							break;
						case ID_LARGE_LURE:
							zombie_rand = rand() % zombie_count;
							for (i = zombie_rand; i < zombie_count; i++) {
								if (z[i].target == NULL) {
									z[i].target = (struct Target *) malloc(sizeof(struct Target));
									z[i].target->type = TYPE_LURE;
									z[i].target->x = p.x;
									z[i].target->y = p.y;
									z[i].target->timer = 20;
								}
							}
							break;
						case ID_MACHETE:
							break;
						case ID_KATANA:
							break;
					}
					can_press = false;
				}
			}
		}

		if (!kb_AnyKey()) can_press = true;

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
			money++;
        }

		if (p.health > 200)
			p.health = 200;
        
        /* Check if the player has died. */
        if (p.health <= 0) {
			p.health = 0;
            draw_fail();
			if (can_press && kb_Data[1] & kb_Mode) {
				for (i = 0; i < zombie_count; i++) {
					z[i].x = 0;
					z[i].y = 0;
					z[i].target = NULL;
				}
				z[0].x = rand() % 310 + 2;
				z[0].y = rand() % 230 + 2;
				money = 0;
				zombie_count = 1;
				zombie_spawn_timer = rand() % 2 + 3;
				p.x = 156;
				p.y = 232;
				hp_x = rand() % 310 + 2;
				hp_y = rand() % 230 + 2;
				p.health = 200;
				infected = false;
				can_press = false;
            }
        } else {
			// Add points for how long the player has survived.
			if (one_second)
				points++;
		}
        
		// Decrement the zombie spawner every second.
		if (one_second)
        	zombie_spawn_timer--;

        gfx_SwapDraw();

    } while (!(can_press && kb_Data[6] & kb_Clear));
	
    gfx_End();
    pgrm_CleanUp();
	return 0;
}

void draw_player(uint16_t x, uint8_t y) {
	gfx_SetColor(COLOR_WHITE);
    gfx_FillCircle_NoClip(x + 2, y + 2, 2);
}

void draw_health_pack(uint16_t x, uint8_t y) {
    gfx_SetColor(COLOR_RED);
    gfx_FillRectangle_NoClip(x + 2, y, 2, 6);
    gfx_FillRectangle_NoClip(x, y + 2, 6, 2);
}

void draw_zombie(uint16_t x, uint8_t y) {
    gfx_SetColor(COLOR_DARK_GREEN);
	gfx_FillRectangle_NoClip(x, y, 4, 4);
	gfx_FillRectangle_NoClip(x + 2, y + 2, 4, 4);
	gfx_SetColor(COLOR_GREEN);
	gfx_FillRectangle_NoClip(x + rand() % 4, y + rand() % 4, 2, 2);
	gfx_SetColor(COLOR_DARK_RED);
	gfx_FillRectangle_NoClip(x + rand() % 4, y + rand() % 4, 2, 2);
}

void draw_custom_text(char* text, uint8_t color, int x, int y, int scale) {
	gfx_SetTextFGColor(color);
    gfx_SetTextBGColor(COLOR_RED);
    gfx_SetTextTransparentColor(COLOR_RED);
    gfx_SetTextXY(x, y);
	gfx_SetTextScale(scale, scale);
	gfx_PrintString(text);
}

void draw_custom_int(int i, uint8_t color, int x, int y, int scale) {
	gfx_SetTextFGColor(color);
    gfx_SetTextBGColor(COLOR_RED);
    gfx_SetTextTransparentColor(COLOR_RED);
    gfx_SetTextXY(x, y);
	gfx_SetTextScale(scale, scale);
	gfx_PrintInt(i, 1);
}

void draw_fail(void) {
    gfx_ScaledTransparentSprite_NoClip(fail, 73, 76, 6, 6);
	draw_custom_text(fail_string, COLOR_WHITE, 57, 148, 2);
}

void draw_store(bool can_press) {

	int i, i_offset = 0, selected_item = 0;
	can_press = false;
	while (!(can_press && (kb_Data[1] & kb_Mode || kb_Data[6] & kb_Clear))) {
		kb_Scan();
		gfx_FillScreen(COLOR_BLACK); // Black background
		draw_custom_text("STORE", COLOR_WHITE, 131, 3, 3);
		for (i = 0; i < 6; i++)
			draw_custom_text(store_inv[i + i_offset].name, COLOR_WHITE, 20, 55 + i * 24, (i == selected_item ? 3 : 2));
		
		gfx_SetColor(COLOR_WHITE);
		gfx_Rectangle_NoClip(205, 40, 60, 60);
		gfx_ScaledTransparentSprite_NoClip(store_inv[selected_item + i_offset].icon, 212, 47, 3, 3);
		draw_custom_text("$", COLOR_WHITE, 205, 120, 2);
		draw_custom_int(store_inv[selected_item + i_offset].price, COLOR_WHITE, 213, 119, 2);
		//draw_custom_text(store_inv[selected_item + i_offset].description, COLOR_WHITE, 150, 105, 1);

		if (can_press) {
			if (kb_Data[7] & kb_Down) {
				selected_item++;
				can_press = false;
			} else if (kb_Data[7] & kb_Up) {
				selected_item--;
				can_press = false;
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

bool is_in_radius(struct Target *t, struct Zombie z) {
	int distance = sqrt(pow(t->x - z.x, 2) + pow(t->y - z.y, 2));
	return distance <= t->radius;
}