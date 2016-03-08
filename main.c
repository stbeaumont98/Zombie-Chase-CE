#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <Zombie.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphc.h>
#include <keypadc.h>

int i, k, l;
int zx[1000];
int zy[1000];
int zs[1000];
int px = 158;
int py = 236;
int hx;
int hy;
unsigned char healthcolor;
float health = 200;
int numzomb = 0;
float j = 0;
int BIG;
int fail = 0;
int points = 0;
uint8_t key;

int main( void ) {
	srand((unsigned) rtc_GetSeconds());
	gc_InitGraph();
	gc_SetPalette(ZombiePalette, sizeof(ZombiePalette));
	gc_DrawBuffer();
	BIG = rand()%150+200;
    hx = rand()%317;
    hy = rand()%237;
    zx[0] = rand()%317;
    zy[0] = rand()%237;
    zs[0] = 2;
	healthcolor = 0x01;
	gc_SetTransparentColor(0x01);
	while( kb_ScanGroup(kb_group_6) != kb_Clear ) {
		key = kb_ScanGroup(kb_group_7);
		gc_FillScrn(0x03);
		if(health > 0){
			gc_SetColorIndex(healthcolor);
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
        for(i = 0; i < numzomb; i++){
            gc_NoClipRectangle(zx[i], zy[i], 4, 4);
            if(zx[i] < px && rand()%2 == 0){
                zx[i]+=zs[i];
            }
            if(zx[i] > px && rand()%2 == 0){
                zx[i]-=zs[i];
            }
            if(zy[i] < py && rand()%2 == 0){
                zy[i]+=zs[i];
            }
            if(zy[i] > py && rand()%2 == 0){
                zy[i]-=zs[i];
            }
            //ZOMBIE COLLISIONS
            if((px < zx[i] + 4) && (px + 4 > zx[i]) && (py < zy[i] + 4) && (4 + py > zy[i])){
				health-=2;
			}
        }
        if((int)j == BIG){
            numzomb++;
            zx[numzomb] = rand()%317;
            zy[numzomb] = rand()%237;
            zs[numzomb] = 2;
            j = 0;
            BIG = rand()%150+200;
        }
        if(!fail){
            if((key & kb_Left) && (px > 0))
                px-=2;
            if((key & kb_Right) && (px < 315))
                px+=2;
            if((key & kb_Up) && (py > 0))
                py-=2;
            if((key & kb_Down) && (py < 235))
				py+=2;
			if((key & kb_Up & kb_Left) && (py < 235) && (py > 0) && (px < 315) && (px > 0)){
                py+=1;
				px-=1;
			}
            if((key & kb_Up & kb_Right) && (py < 235) && (py > 0) && (px < 315) && (px > 0)){
                py+=1;
				px+=1;
			}
            if((key & kb_Down & kb_Left) && (py < 235) && (py > 0) && (px < 315) && (px > 0)){
                py-=1;
				px-=1;
			}
            if((key & kb_Down & kb_Right) && (py < 235) && (py > 0) && (px < 315) && (px > 0)){
                py-=1;
				px+=1;
			}
        }
        //HEALTH BONUS COLLISIONS
		if((px < hx + 4) && (px + 4 > hx) && (py < hy + 4) && (4 + py > hy)){
			health+=5;
			hx=rand()%317;
			hy=rand()%237;
			points++;
		}
		if(health <= 0){
			health = 0; 
            fail = 1;
        }
        if(fail){
			gc_NoClipDrawTransparentSprite(ZombieFail, 67, 64, 194, 72);
			gc_SetTextColor(0x0100);
			gc_PrintStringXY("Press [mode] to Play Again", 74, 154);
			gc_SetTextColor(0x0102);
            gc_PrintStringXY("Press [mode] to Play Again", 75, 155);
            if(kb_ScanGroup(kb_group_1) == kb_Mode){
                for(i = 0; i < numzomb; i++){
                    zx[i] = 0;
                    zy[i] = 0;
                    zs[i] = 0;
                }
                px = 158;
                py = 236;
                hx=rand()%317;
                hy=rand()%237;
                health = 200;
                numzomb = 0;
                j = 0;
                BIG = (rand()%150)+200;
                fail = 0;
                zx[0] = rand()%317;
                zy[0] = rand()%237;
                zs[0] = 1;
				points = 0;
            }
        }
		j+=1;
		gc_SwapDraw();
	}
	gc_CloseGraph();
	pgrm_CleanUp();
	return 0;
}
