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

#include "types.h"
#include "draw.h"

#include "store.h"
#include "inventory.h"
#include "menu.h"

#include "game.h"

void game(int8_t game_mode) {

    bool can_press;

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
    
    for (i = 0; i < 12; i++)
	    p.inv[i] = newItem(TYPE_NONE, ID_NONE, "", "", 0, unknown);

	p.equipped_weapon = NULL;
    p.equipped_armor = NULL;
    p.equipped_boots = NULL;

	/* Initialize the objects array. */
	for (i = 0; i < 16; i++)
		objects[i] = NULL;
	
	obj_count = 0;

	/* Initialize the health pack coordinates */
	hp.x = rand() % 310 + 2;
    hp.y = rand() % 230 + 2;
	hp.value = rand() % 4;

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
        
        draw_player(p.x, p.y);
		draw_health_pack(hp.x, hp.y);

		if (game_mode == CLASSIC) {
			/* Draw the player's score */
			draw_custom_int(p.points, 3, COLOR_WHITE, 2, 226, 2);
		} else if (game_mode == ENHANCED) {
			/* Draw the player's money and timer */
			draw_custom_text("$", COLOR_WHITE, 2, 2, 2);
			draw_custom_int(p.money, 1, COLOR_WHITE, 10, 1, 2);
			draw_custom_int(p.points / 60, 2, COLOR_WHITE, 2, 226, 2);
			draw_custom_text(":", COLOR_WHITE, 18, 226, 2);
			draw_custom_int(p.points % 60, 2, COLOR_WHITE, 22, 226, 2);

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
					 * it is relative to the player. 
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

			if (can_press && game_mode == ENHANCED) {
				if (kb_Data[1] & kb_Del) {
					draw_store(true, &p);
					can_press = false;
				}
				if (kb_Data[1] & kb_Mode) {
					draw_inventory(true, &p);
					can_press = false;
				}
				if (kb_Data[1] & kb_2nd && p.equipped_weapon != NULL) {
					/* Do an action based on what weapon is equipped. */
					if (p.equipped_weapon->type == TYPE_MELEE) {
						uint8_t swing_radius;

						if (p.equipped_weapon->id == ID_MACHETE)
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
							new_object(p.equipped_weapon->id);

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
							if (--p.equipped_weapon->quantity == 0) {
								/* If they've run out of the item that's in their hands,
								 * remove that item from the inventory. 
								 */

								/* Find the item in inventory and remove it from player inventory. */
								*p.equipped_weapon = newItem(TYPE_NONE, ID_NONE, "", "", 0, unknown);

								/* No weapon is equipped anymore. */
								p.equipped_weapon = NULL;
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
			if (game_mode == CLASSIC)
				p.points++;
			else if (game_mode == ENHANCED) {
				p.money+=hp.value;
				hp.value = rand() % 4;
			}
			hp.x = rand() % 310 + 2;
			hp.y = rand() % 230 + 2;
        }

		/* Health cannot exceed 200 */
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
                
                for (i = 0; i < 12; i++)
                    p.inv[i] = newItem(TYPE_NONE, ID_NONE, "", "", 0, unknown);

				p.equipped_weapon = NULL;
				p.equipped_armor = NULL;
				p.equipped_boots = NULL;

				/* Initialize health pack and health. */
				hp.x = rand() % 310 + 2;
				hp.y = rand() % 230 + 2;
				hp.value = rand() % 4;
				p.infected = false;
				can_press = false;
            }
        } else {
			/* Add points for how long the player has survived. */
			if (one_second && game_mode == ENHANCED)
				p.points++;
		}
        
		/* Decrement the zombie spawner every second. */
		if (one_second)
        	zombie_spawn_timer--;

        gfx_SwapDraw();

    } while (!(can_press && kb_Data[6] & kb_Clear));

	for (i = 0; i < obj_count; i++)
		free(objects[i]);

    for (i = 0; i < 12; i++)
	    p.inv[i] = newItem(TYPE_NONE, ID_NONE, "", "", 0, unknown);
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