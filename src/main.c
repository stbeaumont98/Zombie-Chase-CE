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
#define ID_EMPTY 0
#define ID_MACHETE 1			// A machete can be swung all around and kill some zombies within its reach.
#define ID_KATANA 2				// A katana can also be swung around, but its range is bigger.
#define ID_GRENADE 3			// Grenades explode after an amount of time passes and then kills the zombies lured to it and that's about it.
#define ID_C4 4					// C4 explodes at the players command and kills the zombies lured to it and zombies within a medium-sized radius.
#define ID_LAND_MINE 5			// Land mines explode on contact killing anything within its medium-sized blast radius.
#define ID_THE_BIG_ONE 6		// The big one kills all the zombies currently on the screen.
#define ID_TBONE_STEAK 7		// Lures some zombies so the player may have some time to collect more things.
#define ID_WHOLE_TURKEY 8		// Lures more zombies than the small lure for a longer time period.
#define ID_DEAD_HORSE 9			// Lures more zombies than both the small and medium lures for an even longer time period.
#define ID_CARDBOARD_ARMOR 10	// Cardboard armor protects the player from a few bites but falls apart quickly.
#define ID_PLASTIC_ARMOR 11		// Plastic armor can take a bit more damage than cardboard armor.
#define ID_STEEL_ARMOR 12		// Steel armor protects the player from more bites but slows the player down.
#define ID_FORCEFIELD_ARMOR 13	// Forcefield armor protects the player from all bites for 15 seconds.
#define ID_CAMOUFLAGE_ARMOR 14	// Camouflage armor makes the player invisible to zombies for a period of time.
#define ID_LIGHTWEIGHT_BOOTS 15	// Lightweight boots make the player move faster but can be damaged by a few bites.
#define ID_HEAVYWEIGHT_BOOTS 16	// Heavyweight boots make the player move faster and can be damaged by more bites.

struct Item {
	uint16_t id;
	char name[20];
	char description[0xFF];
	uint16_t price;
	uint8_t quantity;
	gfx_sprite_t *icon;
};

struct Target {
	uint16_t x;
	uint8_t y;
	uint8_t timer;
	uint8_t radius;
	uint8_t type;
};

struct Player {
	uint16_t x;
	uint8_t y;
	int health;
	uint16_t money;
	uint16_t points;
	struct Item inventory[10];
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
void draw_custom_text(char* text, uint8_t color, uint16_t x, uint8_t y, int scale);
void draw_custom_int(int i, uint8_t length, uint8_t color, uint16_t x, uint8_t y, int scale);
void draw_inventory(bool from_game);
void draw_store(bool from_game);
void draw_fail(void);
int player_has_item(struct Item inventory[10], uint8_t id);

gfx_sprite_t *z1, *z2, *z3, *z4, *z5, *z6;

char fail_string[] = "PRESS [MODE] TO PLAY AGAIN";
char status_string[0xFF];

static struct Item store_inv[16] = {		// Items that can be bought in the store.
	{ID_MACHETE, "Machete", "This one-handed weapon can be|swung all around to kill zombies|within its reach.", 20, 5, machete},
	{ID_KATANA, "Katana", "Like the machete, this two-handed|weapon can be swung around to|kill zombies, but its range is wider.", 50, 3, katana},
	{ID_GRENADE, "Grenade", "This time-sensitive explosive can|lure and kill a handful of|zombies.", 15, 25, grenade},
	{ID_C4, "C4", "Once placed, this explosive is set|off by remote detonation. Just|make sure you're out of range.", 25, 10, c4},
	{ID_LAND_MINE, "Land Mine", "Land mines explode on contact,|killing anything within its blast|radius. That includes you.", 40, 10, land_mine},
	{ID_THE_BIG_ONE, "The Big One", "We're not quite sure what this|one does, but it sure sounds|fancy.", 1000, 1, the_big_one},
	{ID_TBONE_STEAK, "T-Bone Steak", "A T-bone steak lures some zombies|so the player has some time to|collect more things.", 20, 50, t_bone},
	{ID_WHOLE_TURKEY, "Whole Turkey", "A whole turkey should lure more|zombies and last a little bit|longer than a steak.", 50, 20, turkey},
	{ID_DEAD_HORSE, "Dead Horse", "A dead horse lures the most|zombies for even longer. Don't|ask where we get them.", 100, 10, horse},
	{ID_CARDBOARD_ARMOR, "Cardboard Armor", "Cardboard armor protects the|player from a few bites but falls|apart quickly.", 10, 30, unknown},
	{ID_PLASTIC_ARMOR, "Plastic Armor", "Plastic armor can take a bit more|damage than cardboard armor.", 50, 10, unknown},
	{ID_STEEL_ARMOR, "Steel Armor", "Steel armor protects the player|from more bites but slows the|player down.", 100, 5, unknown},
	{ID_FORCEFIELD_ARMOR, "Forcefield Armor", "Forcefield armor protects the|player from all bites for 15|seconds.", 100, 2, unknown},
	{ID_CAMOUFLAGE_ARMOR, "Camo Armor", "Camouflage armor makes the player|invisible to zombies for a period|of time.", 100, 5, unknown},
	{ID_LIGHTWEIGHT_BOOTS, "Lightweight Boots", "Lightweight boots make the player|move faster but can be damaged by|a few bites.", 20, 0, unknown},
	{ID_HEAVYWEIGHT_BOOTS, "Heavyweight Boots", "Heavyweight boots make the player|move faster and can be damaged by|more bites.", 50, 0, unknown},
};

static gfx_sprite_t *zombie_sprites[8] = {z_0, z_1, z_2, z_3, z_4, z_5, z_6, z_7};

struct Player p;
bool can_press;

int main() {

	int z_dir = 0;
	
	struct Zombie z[0xFF];			// Up to 255 zombies can be on screen at once.
	struct Target *objects[16];		// Up to 16 objects can be on screen at once.

	uint8_t obj_count = 0;			// The number of objects currently on screen.
	
    kb_key_t key;					// Variable to store keypad input.
    uint16_t hp_x;					// Health pack x and y.
    uint8_t hp_y;
    uint8_t zombie_count = 1;		// Number of zombies currently spawned.
    uint8_t zombie_spawn_timer;		// Timer for zombies to spawn.
	uint8_t status_countdown = 0;

	/* Define the player */
    p.x = 156;
    p.y = 232;
    p.health = 200;
	p.money = 0;
	p.points = 0;
	
	int i, j;
	bool infected = false;
	
	int old_time, time, one_second;

	can_press = false;

    srand(rtc_Time());

	/* Set up graphics */
    gfx_Begin(gfx_8bpp);
	gfx_SetPalette(zombie_palette, sizeof(zombie_palette), 0);
	gfx_SetDrawBuffer();

	/* Set up the font */
    gfx_SetFontData(font);
    gfx_SetFontSpacing(font_spacing);
	gfx_SetFontHeight(6);
    
    hp_x = rand() % 310 + 2;
    hp_y = rand() % 230 + 2;
	
	for (i = 0; i < 10; i++) {
		p.inventory[i].id = ID_EMPTY;
		strcpy(p.inventory[i].name,"");
		strcpy(p.inventory[i].description, "");
		p.inventory[i].price = 0;
		p.inventory[i].quantity = 0;
		p.inventory[i].icon = NULL;
	}

	p.equipped_weapon = NULL;
	p.equipped_armor = NULL;
	p.equipped_boots = NULL;

	for (i = 0; i < 0xFF; i++) {
		z[i].x = rand() % 310 + 2;
		z[i].y = rand() % 230 + 2;
		z[i].target = NULL;
		z[i].alive = false;
	}
    
	z[0].alive = true;
    zombie_spawn_timer = rand() % 5 + 4;

	for (i = 0; i < 16; i++) {
		objects[i] = NULL;
	}

	time = rtc_Time();
	
    do { // Game loop

		kb_Scan();
		
		old_time = time;
		time = rtc_Time();
		one_second = time - old_time;
		
		/* Black background */
        gfx_FillScreen(COLOR_BLACK);
        
		if (status_countdown > 0) {
			draw_custom_text(status_string, COLOR_WHITE, 320 - strlen(status_string) * 4, 232, 1);
			if (one_second)
				status_countdown--;
		}

        /* Draw the health bar */
		gfx_SetColor(COLOR_WHITE);
		gfx_Rectangle_NoClip(58, 2, 204, 9);	// Draw the outline of the health bar.
        gfx_SetColor(COLOR_RED);				// Health color red.

        if (p.health >= 1) {
            gfx_FillRectangle_NoClip(60, 4, p.health, 5);
			gfx_SetColor(COLOR_LIGHT_RED);
			gfx_HorizLine_NoClip(60, 4, p.health);
			gfx_SetColor(COLOR_DARK_RED);
			gfx_FillRectangle_NoClip(60, 7, p.health, 2);

			if (infected && one_second) {
				if (p.health >= 4)
					p.health -= 4;
				else
					p.health = 0;
			}
        }
        
        draw_player(p.x, p.y);
		draw_health_pack(hp_x, hp_y);

		/* Draw the players money and timer */
        draw_custom_text("$", COLOR_WHITE, 2, 2, 2);
		draw_custom_int(p.money, 1, COLOR_WHITE, 10, 1, 2);
		draw_custom_int(p.points / 60, 2, COLOR_WHITE, 2, 226, 2);
		draw_custom_text(":", COLOR_WHITE, 18, 226, 2);
		draw_custom_int(p.points % 60, 2, COLOR_WHITE, 22, 226, 2);

		/* Draw the pleyer's equipped items. */
		for (i = 0; i < 3; i++) {
			gfx_SetColor(COLOR_WHITE);
			gfx_Rectangle_NoClip(263 + i * 19, 2, 17, 17);
		}
		if (p.equipped_weapon != NULL)
			gfx_TransparentSprite_NoClip(p.equipped_weapon->icon, 264, 3);
		if (p.equipped_armor != NULL)
			gfx_TransparentSprite_NoClip(p.equipped_armor->icon, 283, 3);
		if (p.equipped_boots != NULL)
			gfx_TransparentSprite_NoClip(p.equipped_boots->icon, 302, 3);

		for (i = 0; i < obj_count; i++) {
			if (objects[i] != NULL) {
				if (objects[i]->timer > 0) {
					switch (objects[i]->type) {
						case TYPE_GRENADE:
							gfx_SetColor(COLOR_DARK_GREEN);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 4, 4);
							draw_custom_int(objects[i]->timer, 1, COLOR_WHITE, objects[i]->x + 4, objects[i]->y - 7, 1);
							if (one_second && objects[i]->timer > 0) {
								objects[i]->timer--;
							}
							break;
						case TYPE_LURE:
							gfx_SetColor(COLOR_DARK_RED);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 4, 4);
							draw_custom_int(objects[i]->timer, 1, COLOR_WHITE, objects[i]->x + 4, objects[i]->y - 7, 1);
							if (one_second && objects[i]->timer > 0) {
								objects[i]->timer--;
							}
							break;
						case TYPE_C4:
							gfx_SetColor(COLOR_BEIGE);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 4, 4);
							strcpy(status_string, "Press [alpha] to detonate C4.");
							status_countdown = 1;
							if (can_press && kb_Data[2] & kb_Alpha)
								objects[i]->timer--;
							break;
						case TYPE_LAND_MINE:
							gfx_SetColor(COLOR_GRAY);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 4, 4);
							for (j = zombie_count; j >= 0; j--) {
								if ((objects[i]->x < z[j].x + 6) && (objects[i]->x + 6 > z[j].x) && (objects[i]->y < z[j].y + 6) && (6 + objects[i]->y > z[j].y))
									objects[i]->timer--;
							}
							break;
					}
				} else {
					for (j = zombie_count; j >= 0; j--)
						if (z[j].target == objects[i])
							z[j].target = NULL;

					if (objects[i]->type == TYPE_GRENADE || objects[i]->type == TYPE_C4 || objects[i]->type == TYPE_LAND_MINE) {

						// Explosion
						gfx_SetColor(COLOR_WHITE);
						for (j = 20; j >= 1; j--)
							gfx_FillCircle(objects[i]->x, objects[i]->y, objects[i]->radius / j);

						int distance;

						// Check if zombies are in the blast radius.
						for (j = zombie_count; j >= 0; j--) {
							distance = sqrt(pow(objects[i]->x - z[j].x, 2) + pow(objects[i]->y - z[j].y, 2));
							if (distance <= objects[i]->radius) {
								z[j].target = NULL;
								z[j] = z[--zombie_count];
								z[zombie_count].alive = false;
							}
						}

						// Check if the player is in the blast radius.
						distance = sqrt(pow(objects[i]->x - p.x, 2) + pow(objects[i]->y - p.y, 2));
						if (distance <= objects[i]->radius)
							p.health -= (p.health / 2);
						
					}

					free(objects[i]);
					if (obj_count == 1)
						objects[i] = NULL;
					else {
						objects[i] = objects[obj_count - 1];
						objects[obj_count - 1] = NULL;
					}
					obj_count--;
				}
			}
		}
		
        for (i = zombie_count; i >= 0; i--) {
			if (z[i].alive) {
				// Draw the zombies
				gfx_TransparentSprite_NoClip(zombie_sprites[z_dir], z[i].x, z[i].y);
				if (z[i].target == NULL) {
					// Zombie chases the player.
					if (z[i].x < p.x && rand() & 1) // Zombie travels right
						z[i].x += 2;
					if (z[i].x > p.x && rand() & 1) // Zombie travels left
						z[i].x -= 2;
					if (z[i].y < p.y && rand() & 1) // Zombie travels downward
						z[i].y += 2;
					if (z[i].y > p.y && rand() & 1) // Zombie travels upward
						z[i].y -= 2;

					// Change the direction the zombie is facing based on where it is relative to the player.
					if (z[i].x == p.x && z[i].y < p.y)
						z_dir = 0;
					else if (z[i].x > p.x && z[i].y < p.y)
						z_dir = 1;
					else if (z[i].x > p.x && z[i].y == p.y)
						z_dir = 2;
					else if (z[i].x > p.x && z[i].y > p.y)
						z_dir = 3;
					else if (z[i].x == p.x && z[i].y > p.y)
						z_dir = 4;
					else if (z[i].x < p.x && z[i].y > p.y)
						z_dir = 5;
					else if (z[i].x < p.x && z[i].y == p.y)
						z_dir = 6;
					else
						z_dir = 7;
					
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

					// Change the direction the zombie is facing based on where it is relative to it's target.
					if (z[i].x == z[i].target->x && z[i].y < z[i].target->y)
						z_dir = 0;
					else if (z[i].x > z[i].target->x && z[i].y < z[i].target->y)
						z_dir = 1;
					else if (z[i].x > z[i].target->x && z[i].y == z[i].target->y)
						z_dir = 2;
					else if (z[i].x > z[i].target->x && z[i].y > z[i].target->y)
						z_dir = 3;
					else if (z[i].x == z[i].target->x && z[i].y > z[i].target->y)
						z_dir = 4;
					else if (z[i].x < z[i].target->x && z[i].y > z[i].target->y)
						z_dir = 5;
					else if (z[i].x < z[i].target->x && z[i].y == z[i].target->y)
						z_dir = 6;
					else
						z_dir = 7;
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
					if (!infected) {
						infected = true;
						strcpy(status_string, "Infected!");
						status_countdown = 3;
					}
				}
			}
        }
		
        if (zombie_spawn_timer == 0 && zombie_count < 0xFF) {
			z[zombie_count % 0xFF].alive = true;
			zombie_count++;
			
            zombie_spawn_timer = rand() % 5 + 4;
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
				if (kb_Data[1] & kb_Del) {
					draw_store(true);
					can_press = false;
				}
				if (kb_Data[1] & kb_Mode) {
					draw_inventory(true);
					can_press = false;
				}
				if (kb_Data[1] & kb_2nd && p.equipped_weapon != NULL) {
					switch (p.equipped_weapon->id) {
						case ID_GRENADE:
							if (objects[obj_count] == NULL) {
								objects[obj_count] = (struct Target *) malloc(sizeof(struct Target));
								objects[obj_count]->type = TYPE_GRENADE;
								objects[obj_count]->x = p.x;
								objects[obj_count]->y = p.y;
								objects[obj_count]->timer = 5;
								objects[obj_count]->radius = 15;

								for (i = rand() % zombie_count; i < zombie_count; i++) {
									if (z[i].target == NULL) {
										z[i].target = objects[obj_count];
									}
								}
							}
							break;
						case ID_C4:
							if (objects[obj_count] == NULL) {
								objects[obj_count] = (struct Target *) malloc(sizeof(struct Target));
								objects[obj_count]->type = TYPE_C4;
								objects[obj_count]->x = p.x;
								objects[obj_count]->y = p.y;
								objects[obj_count]->timer = 1;
								objects[obj_count]->radius = 30;

								for (i = rand() % zombie_count; i < zombie_count; i++) {
									if (z[i].target == NULL) {
										z[i].target = objects[obj_count];
									}
								}
							}
							break;
						case ID_LAND_MINE:
							if (objects[obj_count] == NULL) {
								objects[obj_count] = (struct Target *) malloc(sizeof(struct Target));
								objects[obj_count]->type = TYPE_LAND_MINE;
								objects[obj_count]->x = p.x;
								objects[obj_count]->y = p.y;
								objects[obj_count]->timer = 1;
								objects[obj_count]->radius = 60;

								for (i = rand() % zombie_count; i < zombie_count; i++) {
									if (z[i].target == NULL) {
										z[i].target = objects[obj_count];
									}
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
							if (objects[obj_count] == NULL) {
								objects[obj_count] = (struct Target *) malloc(sizeof(struct Target));
								objects[obj_count]->type = TYPE_LURE;
								objects[obj_count]->x = p.x;
								objects[obj_count]->y = p.y;
								objects[obj_count]->timer = 5;
								objects[obj_count]->radius = 15;

								for (i = rand() % zombie_count; i < zombie_count; i++) {
									if (z[i].target == NULL) {
										z[i].target = objects[obj_count];
									}
								}
							}
							break;
						case ID_WHOLE_TURKEY:
							if (objects[obj_count] == NULL) {
								objects[obj_count] = (struct Target *) malloc(sizeof(struct Target));
								objects[obj_count]->type = TYPE_LURE;
								objects[obj_count]->x = p.x;
								objects[obj_count]->y = p.y;
								objects[obj_count]->timer = 10;
								objects[obj_count]->radius = 20;

								for (i = rand() % zombie_count; i < zombie_count; i++) {
									if (z[i].target == NULL) {
										z[i].target = objects[obj_count];
									}
								}
							}
							break;
						case ID_DEAD_HORSE:
							if (objects[obj_count] == NULL) {
								objects[obj_count] = (struct Target *) malloc(sizeof(struct Target));
								objects[obj_count]->type = TYPE_GRENADE;
								objects[obj_count]->x = p.x;
								objects[obj_count]->y = p.y;
								objects[obj_count]->timer = 20;
								objects[obj_count]->radius = 25;

								for (i = rand() % zombie_count; i < zombie_count; i++) {
									if (z[i].target == NULL) {
										z[i].target = objects[obj_count];
									}
								}
							}
							break;
						case ID_MACHETE:
							break;
						case ID_KATANA:
							break;
					}

					// Decrease the quantity of your equipped weapon and check if you've run out.
					if (--p.equipped_weapon->quantity == 0) {
						// Set all these values so they are correct in the inventory.
						p.equipped_weapon->id = ID_EMPTY;
						strcpy(p.equipped_weapon->name, "");
						strcpy(p.equipped_weapon->description, "");
						p.equipped_weapon->icon = NULL;

						// No weapon is equipped anymore.
						p.equipped_weapon = NULL;
					}
					
					obj_count++;

					if (obj_count > 15)
						obj_count = 15;
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
			p.money++;
        }

		if (p.health > 200)
			p.health = 200;
        
        /* Check if the player has died. */
        if (p.health <= 0) {
			p.health = 0;
            draw_fail();
			if (can_press && kb_Data[1] & kb_Mode) {
				for (i = 0; i < zombie_count; i++) {
					z[i].x = rand() % 310 + 2;
					z[i].y = rand() % 230 + 2;
					z[i].target = NULL;
					z[i].alive = false;
				}
				z[0].alive = true;
				p.money = p.points = 0;
				zombie_count = 1;
				zombie_spawn_timer = rand() % 5 + 4;
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
				p.points++;
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

void draw_custom_text(char* text, uint8_t color, uint16_t x, uint8_t y, int scale) {
	gfx_SetTextFGColor(color);
    gfx_SetTextBGColor(COLOR_RED);
    gfx_SetTextTransparentColor(COLOR_RED);
    gfx_SetTextXY(x, y);
	gfx_SetTextScale(scale, scale);

	if (x + strlen(text) * 4 * scale > gfx_lcdWidth) {
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

void draw_inventory(bool from_game) {
	// Draw the player's inventory.
	uint8_t i;
	can_press = false;
	bool in_loop = true;
	while (in_loop) {
		kb_Scan();
		// Black background.
		gfx_FillScreen(COLOR_BLACK);
		draw_custom_text("$", COLOR_WHITE, 10, 3, 4);
		draw_custom_int(p.money, 1, COLOR_WHITE, 26, 2, 4);
		draw_custom_text("INVENTORY", COLOR_WHITE, 165, 3, 4);
		gfx_SetColor(COLOR_WHITE);
		gfx_FillRectangle_NoClip(0, 30, 320, 3);

		// Draw the inventory.
		for (i = 0; i < 10; i++) {
			gfx_SetColor(COLOR_WHITE);
			gfx_Rectangle_NoClip(57 + (i % 5) * 42, 45 + (i / 5) * 42, 38, 38);
			gfx_Rectangle_NoClip(58 + (i % 5) * 42, 46 + (i / 5) * 42, 36, 36);
			if (p.inventory[i].id != ID_EMPTY)
				gfx_ScaledTransparentSprite_NoClip(p.inventory[i].icon, 61 + (i % 5) * 42, 49 + (i / 5) * 42, 2, 2);
		}

		// Check for key presses.
		if (can_press) {

			// Buttons to break the loop.
			if (kb_Data[1] & kb_Mode || kb_Data[6] & kb_Clear)
				in_loop = false;

			// Action controls.
			if (kb_Data[1] & kb_2nd || kb_Data[6] & kb_Enter) {
				// Equip an item
				can_press = false;
			}

			// The arrows control the player's selection.
			if (kb_Data[7] & kb_Down) {
				// Navigate inventory
				can_press = false;
			} else if (kb_Data[7] & kb_Up) {
				// Navigate
				can_press = false;
			} else if (kb_Data[7] & kb_Left) {
				// Navigate
				can_press = false;
			} else if (kb_Data[7] & kb_Right) {
				// Navigate
				can_press = false;
			}

			// Open the store from inventory.
			if (kb_Data[1] & kb_Del) {
				if (!from_game)
					in_loop = false;
				else {
					draw_store(false);
					can_press = false;
				}
			}
		}

		if (!kb_AnyKey()) can_press = true;
						
		gfx_SwapDraw();
	}
}

void draw_store(bool from_game) {
	// Draw the store.
	int i, i_offset = 0, selected_item = 0, quantity = 1, selling_price;
	can_press = false;
	bool in_loop = true;
	while (in_loop) {
		kb_Scan();
		// Black background.
		gfx_FillScreen(COLOR_BLACK);
		draw_custom_text("$", COLOR_WHITE, 10, 3, 4);
		draw_custom_int(p.money, 1, COLOR_WHITE, 26, 2, 4);
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
		draw_custom_text("$", p.money < selling_price ? COLOR_DARK_RED : COLOR_GREEN, 217, 137, 3);
		draw_custom_int(selling_price, 1, p.money < selling_price ? COLOR_DARK_RED : COLOR_GREEN, 229, 136, 3);

		gfx_Rectangle_NoClip(25, 189, 270, 40);
		draw_custom_text(store_inv[selected_item + i_offset].description, COLOR_WHITE, 28, 190, 2);

		// Check for key presses.
		if (can_press) {

			// Buttons to break the loop.
			if (kb_Data[1] & kb_Del || kb_Data[6] & kb_Clear)
				in_loop = false;

			// Action controls
			if (kb_Data[1] & kb_2nd || kb_Data[6] & kb_Enter) {
				if (p.money >= selling_price) {
					p.money -= selling_price;
					// Check if the user already has at least one of that item
					int item_index = player_has_item(p.inventory, store_inv[selected_item + i_offset].id);
					if (item_index != -1) {
						// If so, add to the quantity owned by the player
						p.inventory[item_index].quantity += quantity;
					} else {
						// Otherwise, find the next non-empty slot and put the new item there.
						strcpy(p.inventory[0].name, store_inv[selected_item + i_offset].name);
						strcpy(p.inventory[0].description, store_inv[selected_item + i_offset].description);
						p.inventory[0].id = store_inv[selected_item + i_offset].id;
						p.inventory[0].quantity = quantity;
						p.inventory[0].icon = store_inv[selected_item + i_offset].icon;

						p.equipped_weapon = &p.inventory[0];
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
					draw_inventory(false);
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

void draw_fail(void) {
    gfx_ScaledTransparentSprite_NoClip(fail, 73, 76, 6, 6);
	draw_custom_text(fail_string, COLOR_WHITE, 57, 148, 2);
}

int player_has_item(struct Item inventory[10], uint8_t id) {
	int i;
	for (i = 0; i < 10; i++) {
		if (inventory[i].id == id)
			return i;
	}
	return -1;
}