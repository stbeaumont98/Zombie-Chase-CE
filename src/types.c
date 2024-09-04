#include <stdint.h>
#include <string.h>

#include <debug.h>

#include "inventory.h"

#include "types.h"

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
	while (temp != item && temp != NULL && index < list->size) {
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
	while (temp->data->id != id && temp != NULL && index < list->size) {
		temp = temp->next;
		index++;
	}
	if (temp != NULL && index < list->size)
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
	for (index = 0; index < list->size; index++) {
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