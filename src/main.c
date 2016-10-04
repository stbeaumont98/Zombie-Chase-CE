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

/* Global things */
uint16_t zx[1000];
uint8_t zy[1000];
uint8_t zs[1000];
char fail_string[] = "Press [mode] to Play Again";

void main( void ) {
    uint8_t key;
    uint8_t healthcolor;
    unsigned BIG;
    uint16_t hx;
    uint8_t hy;
    unsigned numzomb = 1;
    unsigned points = 0;
    unsigned j = 0;
    uint16_t px = 158;
    uint8_t py = 236;
    int i,k,l;
    int health = 200;

    srand(rtc_Time());
    gfx_Begin(gfx_8bpp);
	gfx_SetPalette(gfx_group_1_pal, sizeof(gfx_group_1_pal), 0);
	gfx_SetDrawBuffer();
    
    BIG = rand()%150+200;
    hx = rand()%316;
    hy = rand()%236;
    zx[0] = rand()%316;
    zy[0] = rand()%236;
    zs[0] = 2;
    healthcolor = 0x00;
    
    while( kb_ScanGroup(kb_group_6) != kb_Clear ) {
        gfx_FillScreen(0x03);
        
        /* Draw the health */
        gfx_SetColor(healthcolor);
        if(health > 1) {
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
            gfx_FillRectangle_NoClip(zx[i], zy[i], 4, 4);
            if(zx[i] < px && rand()&1) {
                zx[i]+=zs[i];
            }
            if(zx[i] > px && rand()&1) {
                zx[i]-=zs[i];
            }
            if(zy[i] < py && rand()&1) {
                zy[i]+=zs[i];
            }
            if(zy[i] > py && rand()&1) {
                zy[i]-=zs[i];
            }
            /* Zombie collisions */
            if((px < zx[i] + 4) && (px + 4 > zx[i]) && (py < zy[i] + 4) && (4 + py > zy[i])) {
                health-=2;
            }
        }
        
        if(j == BIG) {
            zx[numzomb] = rand()%316;
            zy[numzomb] = rand()%236;
            zs[numzomb] = 2;
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
        
        /* Health bonus collisions */
        if((px < hx + 4) && (px + 4 > hx) && (py < hy + 4) && (4 + py > hy)) {
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
                    zx[i] = 0;
                    zy[i] = 0;
                    zs[i] = 0;
                }
				zx[0] = rand()%316;
				zy[0] = rand()%236;
				zs[0] = 2;
                points = j = 0;
				numzomb = 1;
				BIG = (rand()%150)+200;
				px = 158;
				py = 236;
				hx=rand()%316;
				hy=rand()%236;
				health = 200;
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
