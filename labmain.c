#define SCREEN_X 320
#define SCREEN_Y 240
#define IMAGE_SIZE (SCREEN_X * SCREEN_Y)

extern void enable_interrupt();

// global variables
volatile unsigned char* VGA = (volatile unsigned char*)0x08000000;
volatile int *VGA_CTRL = (volatile int*) 0x04000100; // pointer to VGA DMA
int timeout = 0;
int back = 0;

/* if we need scroll */
// *(VGA_CTRL+1) = (unsigned int) (VGA+y_ofs*320); // pointer to back buffer
// *(VGA_CTRL+0) = 0; // write to perform the swap
// y_ofs= (y_ofs+ 1) % 240;

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
void set_displays(int display_number, int value) {
  volatile int *display_base = (volatile int *)0x04000050;
  volatile int *display = display_base + (display_number * 4);

  switch (value) 
  {
    case 0: *display = 0x40; break;
    case 1: *display = 0x79; break;
    case 2: *display = 0x24; break;
    case 3: *display = 0x30; break;
    case 4: *display = 0x19; break;
    case 5: *display = 0x12; break;
    case 6: *display = 0x02; break;
    case 7: *display = 0x78; break;
    case 8: *display = 0x00; break;
    case 9: *display = 0x18; break;
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
  timeout %= 51000000;
}

/* show time function for the display time screen */
void show_time(int yourtime)
{
  set_displays(0, yourtime%10); // units of ms
  set_displays(1, (yourtime/10)%10); // tens of ms
  set_displays(2, (yourtime/100)%10); // hundreds of ms
  set_displays(3, yourtime/1000); // thousands of ms
}

static int bestime = 50000000;
void reactiongame() 
{
  while(1) // random delay -> show go screen -> from that screen you make start a timer -> perform subtraction between initial time and the moment you stopped the timer
  {
    draw_image(VGA, 7); // reactready
    srand(timeout);
    int randelay = 3000000 + rand(); // random delay
    for (int i = 0; i < randelay; i++) asm volatile ("nop"); // wait
    
    timeout = 0;
    draw_image(VGA, 8); // reactgo
    while (timeout < 50000000) // PROBLEM IT SEEMS THIS LOOP LASTS NOTHING ... WHY??
    {
      if (get_btn())
      {
        int yourtime = (timeout*40)/1000000;
        while(1)
        {
          // compute best time
          if (bestime > yourtime)
          {
            bestime = yourtime;
          }

          draw_image(VGA, 9); // lookboard for your time
          show_time(yourtime); // displaytime on the 7-segment display
          for (int i = 0; i < 50000000; i++) asm volatile ("nop");

          while(1)
          {
            draw_image(VGA, 11); // bestime & play again
            show_time(bestime);

            // back to home
            if (get_sw() == 0x16) 
            {
              back = 1;
              return;
            }

            // play again
            if (get_btn())
            {
              break;
            }
          }
        }
      }
      draw_image(VGA, 10); // toolate & play again
      // play again
      if (get_btn())
      {
        break;
      }
    }
  }
}

int main ( void ) // delays to adjust because it is slow in switching images
{
  labinit();
  
  // global logic
  while(1)
  {
    // Game selection
    while ((get_sw() == 0x0 || get_sw() == 0x16)) 
    {

      draw_image(VGA, 0); // gameselection1
      for (int i = 0; i < 3000000; i++) asm volatile ("nop");
      
      draw_image(VGA, 1); // gameselection2
      for (int i = 0; i < 3000000; i++) asm volatile ("nop");
    }

    // Reaction game selection
    while (get_sw() == 0x8) // arrow left
    {
      draw_image(VGA, 2); // reactime1
      for (int i = 0; i < 3000000; i++) asm volatile ("nop");

      draw_image(VGA, 3); // reactime2
      for (int i = 0; i < 3000000; i++) asm volatile ("nop"); 

      if (get_btn())
      {
        while(back == 0)
        {
          draw_image(VGA, 6); // descreact
          for (int i = 0; i < 3000000; i++) asm volatile ("nop"); 

          if (get_btn())
          {
            reactiongame();
          }
        }
        back = 0; 
      }
    }

    // Memorize game selection
    while (get_sw() == 0x1) // arrow right
    {
      draw_image(VGA, 4); // memorize1
      for (int i = 0; i < 3000000; i++) asm volatile ("nop");

      draw_image(VGA, 5); // memorize2
      for (int i = 0; i < 3000000; i++) asm volatile ("nop");
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
