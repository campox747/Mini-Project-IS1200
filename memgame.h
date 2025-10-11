#ifndef MEMGAME_H
#define MEMGAME_H

#include "gamestate.h"

// Externally defined globals
extern int coins;
extern int lives;
extern int seq[100];
extern int level;
extern int bestlevel;
extern int counter;

// Function prototypes
void display_money(void);
void show_time(int yourtime);
void display_level(void);
void set_memory(int img_number);
void memseq(int cur_level);
void display_loss(int cur_lives);
int  user_sequence_step(void);

// From main code
void draw_image(volatile unsigned char *buf, int n);
int get_sw(void);
int get_btn(void);
void set_displays(int display_number, int value);
void wait(int cycles);
unsigned int rand(void);

#endif // MEMGAME_H
