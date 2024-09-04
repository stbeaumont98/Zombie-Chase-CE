#ifndef MENU_H_
#define MENU_H_

static char menu_options[3][15] = {
    "CLASSIC",
    "ENHANCED",
    "ABOUT"
};
static char quit_string[] = "PRESS [CLEAR] TO QUIT";

void draw_menu();

#endif