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

#include "gfx\gfx_group_1.h"

/* Function prototypes */
void drawFail(void);

struct Zombie {
	uint24_t x;
	uint8_t y, t; //x position, y position, target
};

/* Global things */


//uint16_t zx[1000];
//uint8_t zy[1000];
//uint8_t zs[1000];
//uint8_t zt[1000];
char fail_string[] = "Press [mode] to Play Again";

void main( void ) {
	
	struct Zombie z[250];
	
    uint8_t key;
    uint8_t healthcolor;
    unsigned BIG;
    uint16_t hx;
    uint8_t hy;
    unsigned numzomb = 1;
    unsigned points = 0;
    unsigned j = 0;
    uint16_t px = 158; //player position x
    uint8_t py = 236; //player position y
	
	uint16_t bombx = 0;
	uint8_t bomby = 0;
	int bombIsPlaced = 0;
	int bombBlinker = 0x00;
	int bombTimer = 5;
	
	uint16_t shrapnelX[8];
	uint8_t shrapnelY[8];
	int shrapnelXVel[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
	int shrapnelYVel[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
	
    int i,k,l;
	int infected = 0;
    int health = 200;

	int zbombs = 0;
	uint8_t zrand = 0;
	
	int oldTime, time, oneSecond, xplTime;

    srand(rtc_Time());
    gfx_Begin(gfx_8bpp);
	gfx_SetPalette(gfx_group_1_pal, sizeof(gfx_group_1_pal), 0);
	gfx_SetDrawBuffer();
    
    BIG = rand()%150+200;
    hx = rand()%316;
    hy = rand()%236;
    z[0].x = rand()%316;
    z[0].y = rand()%236;
	z[0].t = 0; //zombie target
    healthcolor = 0x00;
    
	time = rtc_Time();
    while( kb_ScanGroup(kb_group_6) != kb_Clear ) {
		
		oldTime = time;
		time = rtc_Time();
		oneSecond = time - oldTime;
		
        gfx_FillScreen(0x03);
        
        /* Draw the health */
        gfx_SetColor(healthcolor);
        if(health >= 1) {
			if(infected && oneSecond){
				health-=4;
				gfx_SetColor(0x02);
			}
            gfx_FillRectangle_NoClip((320-health)/2, 3, health, 3);
        }
        
        gfx_SetColor(0x01);
        gfx_FillRectangle_NoClip(px, py, 4, 4);
        gfx_SetColor(healthcolor);
        gfx_FillRectangle_NoClip(hx, hy, 4, 4);
        gfx_SetTextFGColor(0x02);
        gfx_SetTextBGColor(0x00);
        gfx_SetTextTransparentColor(0x00);
        gfx_SetTextXY(0, 232);
        gfx_SetTextFGColor(0x01);
        gfx_PrintInt(points, 3);
        gfx_SetColor(0x02);
        for(i = 0; i < numzomb; i++) {
			gfx_FillRectangle_NoClip(z[i].x, z[i].y, 4, 4);
			if(z[i].t == 0){
				if(z[i].x < px && rand()&1)
					z[i].x+=2;
				if(z[i].x > px && rand()&1)
					z[i].x-=2;
				if(z[i].y < py && rand()&1)
					z[i].y+=2;
				if(z[i].y > py && rand()&1)
					z[i].y-=2;
			} else {
				if(z[i].x < bombx && rand()&1)
					z[i].x+=2;
				if(z[i].x > bombx && rand()&1)
					z[i].x-=2;
				if(z[i].y < bomby && rand()&1)
					z[i].y+=2;
				if(z[i].y > bomby && rand()&1)
					z[i].y-=2;
			}
			/* Zombie collisions */
			if((px < z[i].x + 4) && (px + 4 > z[i].x) && (py < z[i].y + 4) && (4 + py > z[i].y)) {
				health-=2;
				if(!infected)
					infected = 1;
			}
        }
		
		if(bombIsPlaced && bombTimer > 0){
			gfx_SetColor(0x01);
			gfx_FillCircle(bombx, bomby, 2);
			if(oneSecond == 1){
				bombBlinker = (bombBlinker + 1) % 2;
				bombTimer--;
			}
			gfx_SetColor(bombBlinker);
			gfx_FillRectangle_NoClip(bombx - 1, bomby - 1, 3, 2);
			
			xplTime = time;
		}
		
		if(bombTimer == 0){
			gfx_SetColor(0x01);
			for(i = 0; i < 16; i++){
				gfx_FillCircle(bombx, bomby, i);
			}
			numzomb = numzomb - zbombs;
			bombIsPlaced = 0;
			bombTimer = 5;
		}
		
		
		
        if(j == BIG) {
			z[numzomb].x = rand()%316;
			z[numzomb].y = rand()%236;
			z[numzomb].t = 0; //initial target is the game player
			numzomb++;
			
            BIG = (rand()%150)+200;
            j = 0;
        }
        
        /* Process key input */
        key = kb_ScanGroup(kb_group_7);
		
        if((key & kb_Left) && (px > 0) && (health > 0)) {
            px-=2;
        }
        if((key & kb_Right) && (px < 315) && (health > 0)) {
            px+=2;
        }
        if((key & kb_Up) && (py > 0) && (health > 0)) {
            py-=2;
        }
        if((key & kb_Down) && (py < 235) && (health > 0)) {
            py+=2;
        }
        if((py < 235) && (py > 0) && (px < 315) && (px > 0) && (health > 0)) {
            if(key & kb_Up & kb_Left) {
                py+=1;
                px-=1;
            }
            if(key & kb_Up & kb_Right) {
                py+=1;
                px+=1;
            }
            if(key & kb_Down & kb_Left) {
                py-=1;
                px-=1;
            }
            if(key & kb_Down & kb_Right) {
                py-=1;
                px+=1;
            }
        }
		
        key = kb_ScanGroup(kb_group_1);
		if((key & kb_2nd) && (health > 0) && (bombIsPlaced == 0)){
			
			
			
			zrand = rand()%numzomb;
			zbombs = numzomb - zrand;
			
			for(i = zbombs; i < numzomb; i++){
				z[i].t = 1; //new target is bomb
			}
			
			bombIsPlaced = 1;
			bombx = px;
			bomby = py;
		}
		
        
        /* Health bonus collisions */
        if((px < hx + 4) && (px + 4 > hx) && (py < hy + 4) && (4 + py > hy)) {
			if(infected)
				health+=10;
			else
				health+=5;
				hx=rand()%316;
				hy=rand()%236;
				points++;
        }
        
        /* Have we died? */
        if(health <= 0) {
            drawFail();
			if(kb_ScanGroup(kb_group_1) == kb_Mode){
                for(i = 0; i < numzomb; i++){
                    z[i].x = 0;
                    z[i].y = 0;
                }
				z[0].x = rand()%316;
				z[0].y = rand()%236;
                points = j = 0;
				numzomb = 1;
				BIG = (rand()%150)+200;
				px = 158;
				py = 236;
				hx=rand()%316;
				hy=rand()%236;
				health = 200;
				infected = 0;
            }
        }
        
        j++;
        gfx_SwapDraw();
    }
    
    gfx_End();
    pgrm_CleanUp();
}

void drawFail(void) {
    gfx_ScaledTransparentSprite_NoClip(ZombieFail, 78, 64, 4, 4);
    gfx_SetTextFGColor(0x02);
    gfx_PrintStringXY(fail_string, 74, 154);
    gfx_SetTextFGColor(0x01);
    gfx_PrintStringXY(fail_string, 75, 155);
}
