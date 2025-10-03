#define SCREEN_X 320
#define SCREEN_Y 240
#define IMAGE_SIZE (SCREEN_X * SCREEN_Y)
// all the static images of our arcade

extern void enable_interrupt();

enum State { IDLE, RUNNING, PAUSED };
enum State current_state = IDLE;

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

  //volatile char *VGA = (volatile char*) 0x08000000; // pointer to VGA pixel buffer
  //volatile int *VGA_CTRL = (volatile int*) 0x04000100; // pointer to VGA DMA
  // *(VGA_CTRL+1) = (unsigned int) (VGA+y_ofs*320); // pointer to back buffer
  // *(VGA_CTRL+0) = 0; // write to perform the swap
  // y_ofs= (y_ofs+ 1) % 240;
}

int main ( void ) 
{
  labinit();
  volatile unsigned char* VGA = (volatile unsigned char*)0x08000000;
  
  while(1)
  {
    // Game selection
    while (get_sw() == 0x0 && current_state == IDLE) {

      for (int i = 0; i < 6000000; i++) asm volatile ("nop");
      draw_image(VGA, 0);

      draw_image(VGA, 1);
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");
    }

    // Reaction game
    while (get_sw() == 0x1 && current_state == IDLE)
    {
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");

      // Arrow right
      draw_image(VGA, 2);
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");
      
      draw_image(VGA, 3);
    }

    // Memorize game
    while (get_sw() == 0x8 && current_state == IDLE)
    {
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");

      // Arrow left
      draw_image(VGA, 4);
      for (int i = 0; i < 6000000; i++) asm volatile ("nop");
      
      draw_image(VGA, 5);
    }
  }


  
  /* general usage of switches and buttons */
  /*
  

  if (get_sw() == 0x4)
  {
    // vai in alto
  }

  if (get_sw() == 0x8)
  {
    // vai in basso
  }

  if (get_btn() == 0x1)
  {
    // clicca
  }
  */

  /* 
  The 1st game consists of a reaction game.
  The player should click as soon as the screen turns green, and the board records the time it took the player to do so.
  After having stored the result of the player (by keeping only the highest score), while showing the leaderboard it
  asks the player if they want to play again. 
  */
  
  

  /* 
  The 2nd game consistst of a memory game. 
  The board displays an increasing combination of boxes. The player should then click the right combination to pass the 'level'.
  If the player gets it right then they proceed onto the next level, otherwise they lose a life and stay at the same level. 
  Each player has three lives, when they reach 0 lives the game is over. The player has a chance for 10 seconds to purchase 
  other lives with the money they win from passing each level. 
  */
}
