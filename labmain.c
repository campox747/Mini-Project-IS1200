#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_X 320
#define SCREEN_Y 240
#define IMAGE_SIZE (SCREEN_X * SCREEN_Y)

extern void enable_interrupt();

enum State { IDLE, RUNNING, PAUSED };
enum State current_state = IDLE;

// global variables
volatile unsigned char* VGA = (volatile unsigned char*)0x08000000;
volatile int *VGA_CTRL = (volatile int*) 0x04000100; // pointer to VGA DMA
int timeout = 0;

/* if we need scroll */
// *(VGA_CTRL+1) = (unsigned int) (VGA+y_ofs*320); // pointer to back buffer
// *(VGA_CTRL+0) = 0; // write to perform the swap
// y_ofs= (y_ofs+ 1) % 240;

int timexreact[100]; // 100 element array to store all the times of the players


int times[10]; // PROBLEM -> EVERY PLAYER SHOULD HAVE THEIR OWN ARRAY OF TIMES
int i = 0; // counter -> SAME PROBLEM AS ABOVE WE NEED A COUNTER FOR EVERY PLAYER
/* show time function for the display time screen */
void show_time(int yourtime) // maybe showtime should take as inputs yourtime, username -> so that the function knows which players time list should be updated
{
  int bestime = times[0];
  int worstime = times[0];

  // compute bestime
  for (int j = 0; j < 10; j++)
  {
    if (bestime > times[j])
    {
      bestime = times[j];
    }

    if (i > 9 && bestime > yourtime)
    {
      bestime = yourtime;
    }
  }
  // we haven't yet updated the array of times with the time I just scored so...

  // compute worstime
  for (int w = 0; w < 10; w++)
  {
    int pos = 0; // position of the worst time 

    if (worstime < times[w])
    {
      worstime = times[w];
      pos = w; 
    }

    /* if we have more than 10 times we substitute the worstime with the second worst */
    if (i > 9 && worstime > yourtime) 
    {
      times[pos] = yourtime; // since the worstime variable is updated every time you enter the show_time function whenever 
                            // all the times a player scores more than 10 times we enter the new time just scored at the worstime position
    }
  }

  // if i < 10 it means there is still available space in the array hence I can store my time without the need of deliting the worst time from the array
  if (i < 10)
  {
    times[i] = yourtime;
  }
  i++; // update the counter

  draw_image(VGA, 9); // the image with just the blue screen behind

  int y_offset = 120;
  char yourtime[20];
  sprintf(yourtime, "%d", bestime);
  draw_text(VGA, 150, y_offset, yourtime); // PROBLEM -> HOW DO WE DO THIS draw_text FUNCTION?
}

/* leaderboard description */
typedef struct 
{
  char name[20];
  int best_time;
} Player;

Player leaderboard[6];  // store top 5 players + the players best time

void show_leaderboard() 
{
  draw_image(VGA, 12); // leaderboard

  for (int i = 0; i < 6; i++) 
  {
    int y_offset = 100 + i*20;  // vertical spacing

    // Draw player name
    draw_text(VGA, 150, y_offset, leaderboard[i].name);

    // Draw player score (converted to string)
    char score_text[20];
    sprintf(score_text, "%d", leaderboard[i].best_time);
    draw_text(VGA, 300, y_offset, score_text);
  }
}

/* random number function */
static unsigned int seed = 123456789;  // can initialize with anything

unsigned int rand(void) {
  seed = (1103515245 * seed + 12345) & 0x7fffffff; // random numbers to increase the level of randomness
  return seed%24000000;
}

void srand(unsigned int s) {
  seed = s;
}

/* set leds */
void set_leds(int led_mask) 
{
  volatile int *leds = (volatile int*)0x04000000;
  *leds = led_mask;
}

/* set 7-segment display */
void set_displays(int display_number, int value)
{
  volatile int *segment = (volatile int*)display_number;

  switch(value)
  {
    case 0: *segment = 0x40; break;
    case 1: *segment = 0x79; break;
    case 2: *segment = 0x24; break;
    case 3: *segment = 0x30; break;
    case 4: *segment = 0x19; break;
    case 5: *segment = 0x12; break;
    case 6: *segment = 0x02; break;
    case 7: *segment = 0x78; break;
    case 8: *segment = 0x00; break;
    case 9: *segment = 0x18; break;
  }
}

/* switches status */
int get_sw(void)
{
  volatile int *location = (volatile int*)0x04000010;
  return *location & 0x000002ff;
}

/* second push button status */
int get_btn(void)
{
  volatile int *sndbutton = (volatile int*)0x040000d0;
  return *sndbutton & 0x1;
}

/* function to load buffer */
void draw_image(volatile unsigned char *buf, int n){
  const unsigned char *starting_bg = (const unsigned char*) (0x02000000 + n * IMAGE_SIZE);

  for (int y = 0; y < SCREEN_Y; y++)
  {
    for (int x = 0; x < SCREEN_X; x++) 
    {
      buf[y * SCREEN_X + x] = starting_bg[(y) * 320 + x];
    }
  }
}

/* Add your code here for initializing interrupts. */
void labinit(void)
{
  volatile int *periodl = (volatile int*)0x04000028;
  volatile int *periodh = (volatile int*)0x0400002c;
  *periodl = 0x28; 
  *periodh = 0x0; 

  // set control
  volatile int *control = (volatile int*)0x04000024;
  *control = 0x7;

  // in case there is some residual in the flag
  volatile int* status = (volatile int*)0x04000020;
  *status = 0x1;

  // enable interrupts from (g)
  enable_interrupt();
}

unsigned int y_ofs= 0;
/* Below is the function that will be called when an interrupt is triggered. */
void handle_interrupt(unsigned cause) 
{  
  volatile int* status = (volatile int*)0x04000020;
  *status = 0x0;
  timeout++;
  timeout %= 510000000;
}

void reactiongame() 
{
  while(1) // random delay -> show go screen -> from that screen you make start a timer -> perform subtraction between initial time and the moment you stopped the timer
  {
    draw_image(VGA, 7); // reactready
    srand(timeout);
    int randelay = 6000000 + rand(); // random delay
    for (int i = 0; i < randelay; i++) asm volatile ("nop"); // wait
    
    timeout = 0;
    draw_image(VGA, 8); // reactgo
    while (timeout < 500000000)
    {
      if (get_btn())
      {
        int yourtime = (timeout*40)/10^6;
        while(1)
        {
          show_time(yourtime); // displaytime
          for (int i = 0; i < 24000000; i++) asm volatile ("nop"); // display ingame leaderboard & play again

          draw_image(VGA, 11); // leaderboard with user name + their best time + overall players
        }
      }
    }
    draw_image(VGA, 10); // tooslow & play again
  }
}

int main ( void ) // delays to adjust because it is slow in switching images
{
  labinit();
  
  // global logic
  while(1)
  {
    // Game selection
    while (get_sw() == 0x0 && current_state == IDLE) 
    {

      draw_image(VGA, 0); // gameselection1
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");
      
      draw_image(VGA, 1); // gameselection2
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");
    }

    // Reaction game selection
    while (get_sw() == 0x1 && current_state == IDLE)
    {
      draw_image(VGA, 2); // reactime1
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");

      draw_image(VGA, 3); // reactime2
      for (int i = 0; i < 6000000; i++) asm volatile ("nop"); 

      if (get_btn())
      {
        current_state = RUNNING;
        while(1)
        {
          draw_image(VGA, 6); // descreact

          if (get_btn())
          {
            reactiongame();
          }
        }
      }
    }

    // Memorize game selection
    while (get_sw() == 0x8 && current_state == IDLE)
    {
      draw_image(VGA, 4); // memorize1
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");

      draw_image(VGA, 5); // memorize2
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");
    }
  }

  /* 
  The 2nd game consistst of a memory game. 
  The board displays an increasing combination of boxes. The player should then click the right combination to pass the 'level'.
  If the player gets it right then they proceed onto the next level, otherwise they lose a life and stay at the same level. 
  Each player has three lives, when they reach 0 lives the game is over. The player has a chance for 10 seconds to purchase 
  other lives with the money they win from passing each level. 
  */
}
