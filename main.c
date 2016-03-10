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
#include <graphc.h>
#include <keypadc.h>

#include "zombie.h"

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
    gc_InitGraph();
    gc_SetPalette(ZombiePalette, sizeof(ZombiePalette));
    gc_DrawBuffer();
    
    BIG = rand()%150+200;
    hx = rand()%316;
    hy = rand()%236;
    zx[0] = rand()%316;
    zy[0] = rand()%236;
    zs[0] = 2;
    healthcolor = 0x01;
    
    gc_SetTransparentColor(0x01);
    while( kb_ScanGroup(kb_group_6) != kb_Clear ) {
        gc_FillScrn(0x03);
        
        /* Draw the health */
        gc_SetColorIndex(healthcolor);
        if(health > 1) {
            gc_NoClipRectangle((320-health)/2, 3, health, 3);
        }
        
        gc_SetColorIndex(0x02);
        gc_NoClipRectangle(px, py, 4, 4);
        gc_SetColorIndex(healthcolor);
        gc_NoClipRectangle(hx, hy, 4, 4);
        gc_SetTextColor(0x0102);
        gc_SetTextXY(0, 232);
        gc_PrintInt(points, 3);
        gc_SetColorIndex(0x00);
        for(i = 0; i < numzomb; i++) {
            gc_NoClipRectangle(zx[i], zy[i], 4, 4);
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
        if((key & kb_Left) && (px > 0)) {
            px-=2;
        }
        if((key & kb_Right) && (px < 315)) {
            px+=2;
        }
        if((key & kb_Up) && (py > 0)) {
            py-=2;
        }
        if((key & kb_Down) && (py < 235)) {
            py+=2;
        }
        if((py < 235) && (py > 0) && (px < 315) && (px > 0)) {
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
            points = j = 0;
            numzomb = 1;
            BIG = (rand()%150)+200;
            px = 158;
            py = 236;
            hx=rand()%316;
            hy=rand()%236;
            health = 200;
        }
        
        j++;
        gc_SwapDraw();
    }
    
    gc_CloseGraph();
    pgrm_CleanUp();
}

void drawFail(void) {
    gc_NoClipDrawTransparentSprite(ZombieFail, 67, 64, 194, 72);
    gc_SetTextColor(0x0100);
    gc_PrintStringXY(fail_string, 74, 154);
    gc_SetTextColor(0x0102);
    gc_PrintStringXY(fail_string, 75, 155);
    gc_SwapDraw();
    zx[0] = rand()%316;
    zy[0] = rand()%236;
    zs[0] = 2;
    while(kb_ScanGroup(kb_group_1) != kb_Mode);
}
