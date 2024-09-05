#ifndef MENU_H_
#define MENU_H_

#define CLASSIC 0
#define ENHANCED 1
#define SCORES 2

static char menu_options[3][15] = {
    "CLASSIC",
    "ENHANCED",
    "SCORES"
};
static char quit_string[] = "PRESS [CLEAR] TO QUIT";

int8_t main_menu(int8_t selected_item);

#endif