#ifndef STORE_H_
#define STORE_H_

#include "types.h"
#include "gfx\gfx.h"


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

void draw_store(bool from_game, struct Player *p);

#endif