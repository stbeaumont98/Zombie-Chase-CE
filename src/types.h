#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <graphx.h>

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

struct Node {
	struct Item *data;
	struct Node *next;
};

struct LinkedList {
	struct Node *head;
	struct Node *tail;
};

struct Item *newItem(uint8_t type, uint8_t id, char name[], char desc[], uint8_t quantity, gfx_sprite_t *icon);
int8_t getNodeIndex(struct LinkedList *list, struct Node *item);
int8_t getItemIndex(struct LinkedList *list, uint8_t id);
void removeItem(struct LinkedList *list, struct Node *item);
void addItem(struct LinkedList *list, struct Item *item);
void removeAllItems(struct LinkedList *list);
void incItemQuantity(struct LinkedList *list, uint8_t index, uint8_t quantity);
void decItemQuantity(struct LinkedList *list, uint8_t id);

#endif