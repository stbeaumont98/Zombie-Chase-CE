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


/* Colors */
#define COLOR_RED 0x00
#define COLOR_GREEN 0x01
#define COLOR_DARK_GREEN 0x02
#define COLOR_DARK_RED 0x03
#define COLOR_WHITE 0x05
#define COLOR_BLACK 0x06
#define COLOR_LIGHT_RED 0x07
#define COLOR_BEIGE 0x08
#define COLOR_GRAY 0x09
#define COLOR_STEAK 0x0A
#define COLOR_TURKEY 0x0B
#define COLOR_HORSE 0x0C


/* Item types */
#define TYPE_LURE 0
#define TYPE_EXPLOSIVE 1
#define TYPE_MELEE 3
#define TYPE_ARMOR 4
#define TYPE_BOOTS 5

/* Item IDs */
#define ID_MACHETE 0			// A machete can be swung all around and kill some zombies within its reach.
#define ID_KATANA 1				// A katana can also be swung around, but its range is bigger.
#define ID_GRENADE 2			// Grenades explode after an amount of time passes and then kills the zombies lured to it and that's about it.
#define ID_C4 3					// C4 explodes at the players command and kills the zombies lured to it and zombies within a medium-sized radius.
#define ID_LAND_MINE 4			// Land mines explode on contact killing anything within its medium-sized blast radius.
#define ID_THE_BIG_ONE 5		// The big one kills all the zombies currently on the screen.
#define ID_TBONE_STEAK 6		// Lures some zombies so the player may have some time to collect more things.
#define ID_WHOLE_TURKEY 7		// Lures more zombies than the small lure for a longer time period.
#define ID_DEAD_HORSE 8			// Lures more zombies than both the small and medium lures for an even longer time period.
#define ID_CARDBOARD_ARMOR 9	// Cardboard armor protects the player from a few bites but falls apart quickly.
#define ID_PLASTIC_ARMOR 10		// Plastic armor can take a bit more damage than cardboard armor.
#define ID_STEEL_ARMOR 11		// Steel armor protects the player from more bites but slows the player down.
#define ID_FORCEFIELD_ARMOR 12	// Forcefield armor protects the player from all bites for 15 seconds.
#define ID_CAMOUFLAGE_ARMOR 13	// Camouflage armor makes the player invisible to zombies for a period of time.
#define ID_LIGHTWEIGHT_BOOTS 14	// Lightweight boots make the player move faster but can be damaged by a few bites.
#define ID_HEAVYWEIGHT_BOOTS 15	// Heavyweight boots make the player move faster and can be damaged by more bites.

uint8_t inv_size = 0;

struct Drop {
	uint16_t x;
	uint8_t y;
	uint16_t value;
};

struct Item {
	uint8_t type;
	uint8_t id;
	char name[20];
	char description[0xFF];
	uint16_t price;
	uint8_t quantity;
	gfx_sprite_t *icon;
};

struct Node {
	struct Item *data;
	struct Node *next;
};

struct LinkedList {
	struct Node *head;
	struct Node *tail;
};

struct Target {
	uint8_t type;
	uint8_t id;
	uint16_t x;
	uint8_t y;
	uint8_t timer;
	uint8_t radius;
};

struct Player {
	uint16_t x;
	uint8_t y;
	int health;
	bool infected;
	uint16_t money;
	uint16_t points;
	struct LinkedList *inv;
	struct Node *equipped_weapon;
	struct Node *equipped_armor;
	struct Node *equipped_boots;
};

struct Zombie {
	uint16_t x;
	uint8_t y;
	struct Target *target;
	bool alive;
};

/* Function prototypes */

struct Item *newItem(uint8_t type, uint8_t id, char name[], char desc[], uint8_t quantity, gfx_sprite_t *icon);
int8_t getNodeIndex(struct LinkedList *list, struct Node *item);
int8_t getItemIndex(struct LinkedList *list, uint8_t id);
void removeItem(struct LinkedList *list, struct Node *item);
void addItem(struct LinkedList *list, struct Item *item);
void removeAllItems(struct LinkedList *list);
void incItemQuantity(struct LinkedList *list, uint8_t index, uint8_t quantity);
void decItemQuantity(struct LinkedList *list, uint8_t id);

void draw_player(uint16_t x, uint8_t y);
void draw_health_pack(uint16_t x, uint8_t y);
void draw_custom_text(char* text, uint8_t color, uint16_t x, uint8_t y, int scale);
void draw_custom_int(int i, uint8_t length, uint8_t color, uint16_t x, uint8_t y, int scale);
void draw_inventory(bool from_game);
void draw_store(bool from_game);
void draw_fail(void);
void new_object(uint8_t id);

char fail_string[] = "PRESS [MODE] TO PLAY AGAIN";
char status_string[0xFF];

static struct Item store_inv[16] = {		// Items that can be bought in the store.
	{TYPE_MELEE, ID_MACHETE, "Machete", "This one-handed weapon can be|swung all around to kill zombies|within its reach.", 20, 5, machete},
	{TYPE_MELEE, ID_KATANA, "Katana", "Like the machete, this two-handed|weapon can be swung around to|kill zombies, but its range is wider.", 50, 3, katana},
	{TYPE_EXPLOSIVE, ID_GRENADE, "Grenade", "This time-sensitive explosive can|lure and kill a handful of|zombies.", 15, 25, grenade},
	{TYPE_EXPLOSIVE, ID_C4, "C4", "Once placed, this explosive is set|off by remote detonation. Just|make sure you're out of range.", 25, 10, c4},
	{TYPE_EXPLOSIVE, ID_LAND_MINE, "Land Mine", "Land mines explode on contact,|killing anything within its blast|radius. That includes you.", 40, 10, land_mine},
	{TYPE_EXPLOSIVE, ID_THE_BIG_ONE, "The Big One", "We're not quite sure what this|one does, but it sure sounds|fancy.", 1000, 1, the_big_one},
	{TYPE_LURE, ID_TBONE_STEAK, "T-Bone Steak", "A T-bone steak lures some zombies|so the player has some time to|collect more things.", 20, 50, t_bone},
	{TYPE_LURE, ID_WHOLE_TURKEY, "Whole Turkey", "A whole turkey should lure more|zombies and last a little bit|longer than a steak.", 50, 20, turkey},
	{TYPE_LURE, ID_DEAD_HORSE, "Dead Horse", "A dead horse lures the most|zombies for even longer. Don't|ask where we get them.", 100, 10, horse},
	{TYPE_ARMOR, ID_CARDBOARD_ARMOR, "Cardboard Armor", "Cardboard armor protects the|player from a few bites but falls|apart quickly.", 10, 30, cb_armor},
	{TYPE_ARMOR, ID_PLASTIC_ARMOR, "Plastic Armor", "Plastic armor can take a bit more|damage than cardboard armor.", 50, 10, p_armor},
	{TYPE_ARMOR, ID_STEEL_ARMOR, "Steel Armor", "Steel armor protects the player|from more bites but slows the|player down.", 100, 5, s_armor},
	{TYPE_ARMOR, ID_FORCEFIELD_ARMOR, "Forcefield Armor", "Forcefield armor protects the|player from all bites for 15|seconds.", 100, 2, ff_armor},
	{TYPE_ARMOR, ID_CAMOUFLAGE_ARMOR, "Camo Armor", "Camouflage armor makes the player|invisible to zombies for a period|of time.", 100, 5, camo_armor},
	{TYPE_BOOTS, ID_LIGHTWEIGHT_BOOTS, "Lightweight Boots", "Lightweight boots make the player|move faster but can be damaged by|a few bites.", 20, 0, lw_boots},
	{TYPE_BOOTS, ID_HEAVYWEIGHT_BOOTS, "Heavyweight Boots", "Heavyweight boots make the player|move faster and can be damaged by|more bites.", 50, 0, hw_boots},
};

static gfx_sprite_t *zombie_sprites[8] = {z_0, z_1, z_2, z_3, z_4, z_5, z_6, z_7};

struct Player p;
struct Target *objects[16];		// Up to 16 objects can be on screen at once.
uint8_t obj_count;				// The number of objects currently on screen.

bool can_press;

int main() {
	uint8_t i, j;
	int old_time, time, one_second;

	uint8_t z_dir = 0;
	
	struct Zombie z[0xFF];			// Up to 255 zombies can be on screen at once.
	
    kb_key_t key;					// Variable to store keypad input.
	struct Drop hp;					// Health pack x and y.
    uint8_t zombie_count = 1;		// Number of zombies currently spawned.
    uint8_t zombie_spawn_timer;		// Timer for zombies to spawn.
	uint8_t status_countdown = 0;

	/* Initialize the player. */
    p.x = 156;
    p.y = 232;
    p.health = 200;
	p.infected = false;
	p.money = 500;
	p.points = 0;

	p.inv = (struct LinkedList *) malloc(sizeof(struct LinkedList));
	removeAllItems(p.inv);

	inv_size = 0;
	p.equipped_weapon = p.equipped_armor = p.equipped_boots = NULL;

	/* Initialize the objects array. */
	for (i = 0; i < 16; i++)
		objects[i] = NULL;
	
	obj_count = 0;

	/* Initialize the health pack coordinates */
	hp.x = rand() % 310 + 2;
    hp.y = rand() % 230 + 2;
	hp.value = rand() % 3 + 2;

	/* Initialize the array of zombies. */
	for (i = 0; i < 0xFF; i++) {
		z[i].x = 0;
		z[i].y = 0;
		z[i].target = NULL;
		z[i].alive = false;
	}
    
	/* Initialize the first zombie */
	z[0].x = rand() % 310 + 2;
	z[0].y = rand() % 230 + 2;
	z[0].alive = true;
	z[0].target = NULL;

    zombie_spawn_timer = rand() % 5 + 4;

    srand(rtc_Time());

	/* Set up graphics */
    gfx_Begin(gfx_8bpp);
	gfx_SetPalette(zombie_palette, sizeof(zombie_palette), 0);
	gfx_SetDrawBuffer();

	/* Set up the font */
    gfx_SetFontData(font);
    gfx_SetFontSpacing(font_spacing);
	gfx_SetFontHeight(6);

	time = rtc_Time();

	can_press = false;
	
    do { // Game loop

		kb_Scan();
		
		old_time = time;
		time = rtc_Time();
		one_second = time - old_time;
		
		/* Draw the black background */
        gfx_FillScreen(COLOR_BLACK);
        
		/* Display any status messages to the player.
		 * TODO: Determine if any visual improvements can be made. 
		 */
		if (status_countdown > 0) {
			draw_custom_text(status_string, COLOR_WHITE, 320 - strlen(status_string) * 4 * 2, 224, 2);
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

			if (p.infected && one_second) {
				if (p.health >= 4)
					p.health -= 4;
				else
					p.health = 0;
			}
        }
		
		/* Draw the players money and timer */
        draw_custom_text("$", COLOR_WHITE, 2, 2, 2);
		draw_custom_int(p.money, 1, COLOR_WHITE, 10, 1, 2);
		draw_custom_int(p.points / 60, 2, COLOR_WHITE, 2, 226, 2);
		draw_custom_text(":", COLOR_WHITE, 18, 226, 2);
		draw_custom_int(p.points % 60, 2, COLOR_WHITE, 22, 226, 2);
        
        draw_player(p.x, p.y);
		draw_health_pack(hp.x, hp.y);

		/* Draw the player's equipped items.
		 * TODO: Make improvements on visuals.
		 */
		/*
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
		*/

		/* Game logic for any objects on the screen. */
		for (i = 0; i < obj_count; i++) {
			if (objects[i] != NULL) {
				if (objects[i]->timer > 0) {
					/* Draw objects differently based on their ID */
					switch (objects[i]->id) {
						case ID_GRENADE:
							gfx_SetColor(COLOR_DARK_GREEN);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 3, 3);
							draw_custom_int(objects[i]->timer, 1, COLOR_WHITE, objects[i]->x + 4, objects[i]->y - 7, 1);
							if (one_second && objects[i]->timer > 0)
								objects[i]->timer--;
							break;
						case ID_C4:
							gfx_SetColor(COLOR_BEIGE);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 5, 3);
							strcpy(status_string, "Press [alpha] to detonate C4.");
							status_countdown = 1;
							if (can_press && kb_Data[2] & kb_Alpha)
								objects[i]->timer--;
							break;
						case ID_LAND_MINE:
							gfx_SetColor(COLOR_GRAY);
							gfx_FillCircle_NoClip(objects[i]->x, objects[i]->y, 2);
							for (j = 0; j < zombie_count; j++) {
								if ((objects[i]->x < z[j].x + 6) && (objects[i]->x + 6 > z[j].x) && (objects[i]->y < z[j].y + 6) && (6 + objects[i]->y > z[j].y))
									objects[i]->timer--;
							}
							break;
						case ID_TBONE_STEAK:
							gfx_SetColor(COLOR_STEAK);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 3, 3);
							draw_custom_int(objects[i]->timer, 1, COLOR_WHITE, objects[i]->x + 4, objects[i]->y - 7, 1);
							if (one_second && objects[i]->timer > 0)
								objects[i]->timer--;
							break;
						case ID_WHOLE_TURKEY:
							gfx_SetColor(COLOR_TURKEY);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 4, 4);
							draw_custom_int(objects[i]->timer, 1, COLOR_WHITE, objects[i]->x + 4, objects[i]->y - 7, 1);
							if (one_second && objects[i]->timer > 0)
								objects[i]->timer--;
							break;
						case ID_DEAD_HORSE:
							gfx_SetColor(COLOR_HORSE);
							gfx_FillRectangle_NoClip(objects[i]->x, objects[i]->y, 5, 3);
							draw_custom_int(objects[i]->timer, 1, COLOR_WHITE, objects[i]->x + 4, objects[i]->y - 7, 1);
							if (one_second && objects[i]->timer > 0)
								objects[i]->timer--;
							break;
					}
				} else {
					if (objects[i]->type == TYPE_EXPLOSIVE) {

						for (j = 0; j < zombie_count; j++)
							if (z[j].target == objects[i])
								z[j].target = NULL;

						/* Explosion "animation." */
						gfx_SetColor(COLOR_WHITE);
						gfx_FillCircle(objects[i]->x, objects[i]->y, objects[i]->radius);
						gfx_SwapDraw();

						dbg_printf("Test 1\n");

						int distance;

						/* Check if zombies are in the blast radius. */
						for (j = 0; j < zombie_count; j++) {
							distance = sqrt(pow(objects[i]->x - z[j].x + 2, 2) + pow(objects[i]->y - z[j].y + 2, 2));
							if (distance <= objects[i]->radius) {
								z[j].target = NULL;
								if (zombie_count > 1) {
									z[j] = z[--zombie_count];
									z[zombie_count].alive = false;
								} else
									z[j].alive = false;
							}
						}

						dbg_printf("Test 2\n");

						/* Check if the player is in the blast radius. */
						distance = sqrt(pow(objects[i]->x - p.x + 2, 2) + pow(objects[i]->y - p.y + 2, 2));
						if (distance <= objects[i]->radius)
							p.health -= (p.health / 2);

						
						dbg_printf("Test 3\n");
						
					}

					/* Take care of the dead object. */
					free(objects[i]); // Why does the game crash when freeing this object??
					dbg_printf("Test 4\n");
					if (obj_count <= 1)
						objects[i] = NULL;
					else {
						objects[i] = objects[--obj_count];
						objects[obj_count] = NULL;
					}
					
					dbg_printf("Test 5\n");
				}
			}
		}
		
		/* Game logic for any zombies on the screen. */
        for (i = 0; i < zombie_count; i++) {
			if (z[i].alive) {
				/* Draw the zombies */
				gfx_TransparentSprite_NoClip(zombie_sprites[z_dir], z[i].x, z[i].y);
				if (z[i].target == NULL) {
					/* If the zombie has no other target, it will chase the 
					 * player. 
					 */

					/* Change position of the zombie based on where it is
					 * relative to the player. 
					 */
					if (z[i].x < p.x && rand() & 1) // Zombie travels right
						z[i].x += 2;
					if (z[i].x > p.x && rand() & 1) // Zombie travels left
						z[i].x -= 2;
					if (z[i].y < p.y && rand() & 1) // Zombie travels downward
						z[i].y += 2;
					if (z[i].y > p.y && rand() & 1) // Zombie travels upward
						z[i].y -= 2;

					/* Change the direction the zombie is facing based on where
					 * it is relative to the player. 
					 */
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

					/* Change position of the zombie based on where it is
					 * relative to its target. 
					 */
					if (z[i].x < z[i].target->x && rand() & 1)
						z[i].x += 2;
					if (z[i].x > z[i].target->x && rand() & 1)
						z[i].x -= 2;
					if (z[i].y < z[i].target->y && rand() & 1)
						z[i].y += 2;
					if (z[i].y > z[i].target->y && rand() & 1)
						z[i].y -= 2;

					/* Change the direction the zombie is facing based on where
					 * it is relative to its target. 
					 */
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

				/* Zombie bounds. */
				if (z[i].x < 2)
					z[i].x = 2;
				if (z[i].x > 312)
					z[i].x = 312;
				if (z[i].y < 2)
					z[i].y = 2;
				if (z[i].y > 232)
					z[i].y = 232;

				/* Zombie/player collisions. */
				if ((p.x < z[i].x + 6) && (p.x + 5 > z[i].x) && (p.y < z[i].y + 6) && (p.y + 5 > z[i].y)) {
					p.health--;
					if (!p.infected) {
						p.infected = true;
						strcpy(status_string, "Infected!");
						status_countdown = 3;
					}
				}
			}
        }
		
		/* Spawn new zombies every so often. */
        if (zombie_spawn_timer == 0 && zombie_count < 0xFF) {
			z[zombie_count].x = rand() % 310 + 2;
			z[zombie_count].y = rand() % 230 + 2;
			z[zombie_count].alive = true;
			z[zombie_count].target = NULL;
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
					/* Do an action based on what weapon is equipped. */
					if (p.equipped_weapon->data->type == TYPE_MELEE) {
						uint8_t swing_radius;

						if (p.equipped_weapon->data->id == ID_MACHETE)
							swing_radius = 10;
						else
							swing_radius = 20;
						
						/* Swing "animation." */
						gfx_SetColor(COLOR_GRAY);
						gfx_FillCircle(p.x + 2, p.y + 2, swing_radius);
						gfx_SwapDraw();

						/* Check if zombies are in swing radius. */
						for (i = 0; i < zombie_count; i++) {
							uint16_t distance = sqrt(pow(p.x + 2 - z[i].x + 2, 2) + pow(p.y + 2 - z[i].y + 2, 2));
							if (distance <= swing_radius) {
								z[i].target = NULL;
								z[i] = z[--zombie_count];
								z[zombie_count].alive = false;
							}
						}
					} else {

						/* Only drop something if there's a free spot in the objects array. */
						if (objects[obj_count] == NULL) {

							/* Add a new object to the objects array. */
							new_object(p.equipped_weapon->data->id);

							/* Set this new object as the target for a random amount of zombies. */
							for (i = rand() % zombie_count; i < zombie_count; i++) {
								if (z[i].target == NULL) {
									z[i].target = objects[obj_count];
								}
							}
							
							obj_count++;

							if (obj_count > 15)
								obj_count = 15;

							/* Decrease the quantity of the players equipped weapon and check if they've run out. */
							if (--p.equipped_weapon->data->quantity == 0) {
								/* If they've run out of the item that's in their hands,
								 * remove that item from the inventory. 
								 */

								/* Find the item in inventory and remove it from player inventory. */
								struct Node *tmp = p.inv->head;
								while (tmp != NULL) {
									if (p.equipped_weapon->data == tmp->data) {
										removeItem(p.inv, tmp);
									}
									tmp = tmp->next;
								}

								/* No weapon is equipped anymore. */
								p.equipped_weapon = NULL;

								inv_size--;
							}
						}
					}
					can_press = false;
				}
			}
		}

		if (!kb_AnyKey()) can_press = true;

		/* Player bounds. */
		if (p.x < 2)
			p.x = 2;
		if (p.x > 312)
			p.x = 312;
		if (p.y < 2)
			p.y = 2;
		if (p.y > 232)
			p.y = 232;

        /* Health pack collisions */
        if ((p.x < hp.x + 6) && (p.x + 5 > hp.x) && (p.y < hp.y + 6) && (5 + p.y > hp.y)) {
			if (p.infected)
				p.health += 10;
			else 
				p.health += 5;
			p.money+=hp.value;
			hp.x = rand() % 310 + 2;
			hp.y = rand() % 230 + 2;
			hp.value = rand() % 3 + 2;
        }

		if (p.health > 200)
			p.health = 200;
        
        /* Check if the player has died. */
        if (p.health <= 0) {
			p.health = 0;
            draw_fail();
			if (can_press && kb_Data[1] & kb_Mode) {
				/* Initialize zombies array. */
				for (i = 0; i < zombie_count; i++) {
					z[i].x = 0;
					z[i].y = 0;
					z[i].target = NULL;
					z[i].alive = false;
				}

				/* Initialize objects array. */
				for (i = 0; i < obj_count; i++) {
					free(objects[i]);
					objects[i] = NULL;
				}

				/* Initialize the first zombie. */
				z[0].x = rand() % 310 + 2;
				z[0].y = rand() % 230 + 2;
				z[0].alive = true;
				z[0].target = NULL;
				zombie_spawn_timer = rand() % 5 + 4;
				zombie_count = 1;

				/* Initialize player variables. */
				p.x = 156;
				p.y = 232;
				p.health = 200;
				p.money = p.points = 0;

				removeAllItems(p.inv);
				inv_size = 0;

				p.equipped_weapon = NULL;
				p.equipped_armor = NULL;
				p.equipped_boots = NULL;

				/* Initialize health pack and health. */
				hp.x = rand() % 310 + 2;
				hp.y = rand() % 230 + 2;
				hp.value = rand() % 3 + 2;
				p.infected = false;
				can_press = false;
            }
        } else {
			/* Add points for how long the player has survived. */
			if (one_second)
				p.points++;
		}
        
		/* Decrement the zombie spawner every second. */
		if (one_second)
        	zombie_spawn_timer--;

        gfx_SwapDraw();

    } while (!(can_press && kb_Data[6] & kb_Clear));

	for (i = 0; i < obj_count; i++)
		free(objects[i]);

	removeAllItems(p.inv);
	free(p.inv);
	
    gfx_End();
    pgrm_CleanUp();
	return 0;
}

struct Item *newItem(uint8_t type, uint8_t id, char name[], char desc[], uint8_t quantity, gfx_sprite_t *icon) {
	struct Item *i = (struct Item *) malloc(sizeof(struct Item));

	i->type = type;
	i->id = id;
	strcpy(i->name, name);
	strcpy(i->description, desc);
	i->quantity = quantity;
	i->icon = icon;

	return i;
}

int8_t getNodeIndex(struct LinkedList *list, struct Node *item) {
	struct Node *temp = list->head;
	int8_t index = 0;
	while (temp != item && temp != NULL && index < inv_size) {
		temp = temp->next;
		index++;
	}
	if (temp != NULL && index < 10)
		return index;
	else
		return -1;
}

int8_t getItemIndex(struct LinkedList *list, uint8_t id) {
	struct Node *temp = list->head;
	int8_t index = 0;
	while (temp->data->id != id && temp != NULL && index < inv_size) {
		temp = temp->next;
		index++;
	}
	if (temp != NULL && index < inv_size)
		return index;
	else
		return -1;
}

void removeItem(struct LinkedList *list, struct Node *item) {
	struct Node *temp;

	if (list->head != NULL && list->tail != NULL) {
		if (list->head == item && list->head != list->tail) {
			temp = list->head;
			list->head = list->head->next;
			free(temp);
		} else if (list->head == item && list->head == list->tail) {
			free(list->head);
			free(list->tail);
			list->head = list->tail = NULL;
		} else {
			uint8_t index = getNodeIndex(list, item);
			temp = list->head;
			while (index > 1) {
				temp = temp->next;
				index--;
			}
			struct Node *t = temp->next;
			temp->next = temp->next->next;
			free(t);
		}
	}
}

void addItem(struct LinkedList *list, struct Item *item) {
	struct Node *n = (struct Node *) malloc(sizeof(struct Node));
	n->data = item;
	n->next = NULL;

	if (list->head == NULL && list->tail == NULL) {
		list->head = n;
		list->tail = n;
		dbg_printf("List is not empty anymore!\n");
	} else {
		list->tail->next = n;
		list->tail = list->tail->next;
		dbg_printf("Added to list!\n");
	}
}

void removeAllItems(struct LinkedList *list) {
	uint8_t index = 0;
	for (index = 0; index < inv_size; index++) {
		removeItem(list, list->head);
	}
	free(list->tail);
	list->head = list->tail = NULL;
}

void incItemQuantity(struct LinkedList *list, uint8_t index, uint8_t quantity) {
	struct Node *temp = list->head;
	while (index > 0) {
		temp = temp->next;
		index--;
	}
	temp->data->quantity += quantity;
}

void decItemQuantity(struct LinkedList *list, uint8_t id) {
	uint8_t index = getItemIndex(list, id);
	struct Node *temp = list->head;
	while (index > 0) {
		temp = temp->next;
		index--;
	}
	temp->data->quantity--;
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

void draw_inventory(bool from_game) {
	// Draw the player's inventory.
	uint8_t i;
	can_press = false;
	bool in_loop = true;

	uint8_t cursor_pos = 0;

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
		for (i = 0; i < 12; i++) {
			gfx_SetColor(COLOR_WHITE);
			gfx_Rectangle_NoClip(166 + (i % 3) * 50, 42 + (i / 3) * 50, 38, 38);
			gfx_Rectangle_NoClip(167 + (i % 3) * 50, 43 + (i / 3) * 50, 36, 36);
		}

		// Draw the inventory items.
		i = 0;
		struct Node *tmp = p.inv->head;
		while (tmp != NULL && i < inv_size) {
			gfx_ScaledTransparentSprite_NoClip(tmp->data->icon, 170 + (i % 3) * 50, 46 + (i / 3) * 50, 2, 2);
			tmp = tmp->next;
			i++;
		}


		// Draw inventory cursor.
		gfx_Rectangle_NoClip(163 + (cursor_pos % 3) * 50, 39 + (cursor_pos / 3) * 50, 44, 44);

		// Draw the player's equipped items
		gfx_Rectangle_NoClip(22, 60, 55, 55);
		gfx_Rectangle_NoClip(23, 61, 53, 53);
		gfx_Rectangle_NoClip(86, 60, 55, 55);
		gfx_Rectangle_NoClip(87, 61, 53, 53);
		gfx_Rectangle_NoClip(22, 124, 55, 55);
		gfx_Rectangle_NoClip(23, 125, 53, 53);

		gfx_ScaledTransparentSprite_NoClip(p.equipped_armor != NULL ? b_frame : p.equipped_armor->data->icon, 27, 65, 3, 3);
		gfx_ScaledTransparentSprite_NoClip(p.equipped_boots != NULL ? f_frame : p.equipped_boots->data->icon, 27, 129, 3, 3);
		gfx_ScaledTransparentSprite_NoClip(h1_frame, 91, 65, 3, 3);
		if (p.equipped_weapon != NULL)
			gfx_ScaledTransparentSprite_NoClip(p.equipped_weapon->data->icon, 91, 65, 3, 3);

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
			if (kb_Data[7] & kb_Down && cursor_pos < 9) {
				cursor_pos += 3;
				can_press = false;
			} else if (kb_Data[7] & kb_Up && cursor_pos > 2) {
				cursor_pos -= 3;
				can_press = false;
			} else if (kb_Data[7] & kb_Left && cursor_pos > 0) {
				cursor_pos--;
				can_press = false;
			} else if (kb_Data[7] & kb_Right && cursor_pos < 11) {
				cursor_pos++;
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
	uint8_t i, quantity = 1;
	int8_t i_offset = 0, selected_item = 0;
	uint16_t selling_price;
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
				if (p.money >= selling_price && inv_size < 10) {
					p.money -= selling_price;
					// Check if the user already has at least one of that item
					int item_index = getItemIndex(p.inv, store_inv[selected_item + i_offset].id);
					dbg_printf("Item index found!\nIndex: %d\n", item_index);
					if (item_index != -1) {
						// If so, add to the quantity owned by the player
						incItemQuantity(p.inv, item_index, quantity);
					} else {
						// Otherwise, allocate memory and put the new item there.

						addItem(p.inv, newItem(
							store_inv[selected_item + i_offset].type,
							store_inv[selected_item + i_offset].id,
							store_inv[selected_item + i_offset].name,
							store_inv[selected_item + i_offset].description,
							quantity,
							store_inv[selected_item + i_offset].icon
							));
						
						dbg_printf("Item added!\n");

						p.equipped_weapon = p.inv->head;

						inv_size++;
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

void new_object(uint8_t id) {
	objects[obj_count] = (struct Target *) malloc(sizeof(struct Target));
	objects[obj_count]->type = (id == ID_GRENADE || id == ID_C4 || id == ID_LAND_MINE);
	objects[obj_count]->id = id;
	objects[obj_count]->x = p.x;
	objects[obj_count]->y = p.y;
	switch (id) {
		case ID_GRENADE:
			objects[obj_count]->timer = 5;
			objects[obj_count]->radius = 15;
			break;
		case ID_C4:
			objects[obj_count]->timer = 1;
			objects[obj_count]->radius = 30;
			break;
		case ID_LAND_MINE:
			objects[obj_count]->timer = 1;
			objects[obj_count]->radius = 60;
			break;
		case ID_TBONE_STEAK:
			objects[obj_count]->timer = 5;
			objects[obj_count]->radius = 15;
			break;
		case ID_WHOLE_TURKEY:
			objects[obj_count]->timer = 10;
			objects[obj_count]->radius = 20;
			break;
		case ID_DEAD_HORSE:
			objects[obj_count]->timer = 20;
			objects[obj_count]->radius = 25;
			break;
	}
}