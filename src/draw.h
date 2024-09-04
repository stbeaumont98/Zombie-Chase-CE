#ifndef DRAW_H_
#define DRAW_H_

#include <stdint.h>

static char fail_string[] = "PRESS [MODE] TO PLAY AGAIN";

void draw_player(uint16_t x, uint8_t y);
void draw_health_pack(uint16_t x, uint8_t y);
void draw_custom_text(char* text, uint8_t color, uint16_t x, uint8_t y, int scale);
void draw_custom_int(int i, uint8_t length, uint8_t color, uint16_t x, uint8_t y, int scale);
void draw_fail(void);

#endif