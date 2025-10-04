#define SCREEN_X 320
#define SCREEN_Y 240
#define IMAGE_SIZE (SCREEN_X * SCREEN_Y)

extern void enable_interrupt();

// global variables
volatile unsigned char* VGA = (volatile unsigned char*)0x08000000;
volatile int *VGA_CTRL = (volatile int*) 0x04000100; // pointer to VGA DMA
long timeout = 0;
int coins = 100; // for memory game
int lives = 3; // for memory game

/* if we need scroll */
// *(VGA_CTRL+1) = (unsigned int) (VGA+y_ofs*320); // pointer to back buffer
// *(VGA_CTRL+0) = 0; // write to perform the swap
// y_ofs= (y_ofs+ 1) % 240;

/* random number function */
static unsigned int seed = 123456789;  // can initialize with anything

unsigned int rand(void) 
{
  seed = (1103515245 * seed + 12345) & 0x7fffffff; // random numbers to increase the level of randomness
  return seed%24000000;
}

void srand(unsigned int s) 
{
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

unsigned int y_ofs= 0;
/* Below is the function that will be called when an interrupt is triggered. */
void handle_interrupt(unsigned cause) 
{  
  volatile int* status = (volatile int*)0x04000020;
  *status = 0x0;
  timeout++;
  timeout %= 51000000;
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
    int randelay = 3000000 + rand(); // pseudo-random delay
    for (int i = 0; i < randelay; i++) asm volatile ("nop"); // wait
    
    timeout = 0;
    draw_image(VGA, 8); // reactgo
    while (timeout < 50000000)
    {
      asm volatile ("nop");
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

            // play again
            if (get_btn())
            {
              reactiongame();
            }

            if (get_sw() == 0x10)
            {
              return;
            }
          }
        }
      }
    }
    draw_image(VGA, 10); // toolate
    // play again
    while(1)
    {
      if (get_btn())
      {
        break;
      }

      if (get_sw() == 512)
      {
        return;
      }
    }
  }
}

void set_memory(int img_number)
{
  switch (img_number)
  {
    case 0: draw_image(VGA, 22); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 1: draw_image(VGA, 13); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 2: draw_image(VGA, 14); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 3: draw_image(VGA, 15); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 4: draw_image(VGA, 16); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 5: draw_image(VGA, 17); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 6: draw_image(VGA, 18); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 7: draw_image(VGA, 19); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 8: draw_image(VGA, 20); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
    case 9: draw_image(VGA, 21); for (int i = 0; i < 3000000; i++) asm volatile ("nop"); break;
  }
}

void display_money()
{
  int i = coins%10;
  int j = (coins/10)%10;
  int w = (coins/100)%10;
  int h = (coins/1000)%10;

  set_display(0, i);
  set_display(0, j);
  set_display(0, w);
  set_display(0, h);
}

void display_loss(int lives)
{
  switch (lives)
  {
    case 0: while(1) {draw_image(VGA, 23); if (get_btn()) {while(1) {if (coins > 100) {if (get_btn()) {coins -= 100; lives++; memory_game();}}}}}
    case 1: while(1) {draw_image(VGA, 25); if (get_btn()) {memory_game();}}
    case 2: while(1) {draw_image(VGA, 24); if (get_btn()) {memory_game();}}
  }
}

int seq[100];
int level = 1;
int counter = 0;
void memseq(int level)
{
  for (int i = 0; i < level; i++)
  {
    // creation of a random number
    srand(level); 
    int img = 1 + (rand()%9); 
    // display the sqr on screen
    set_memory(img);
    seq[i] = img; 
  }
}

void user_sequence() 
{
  // logic for the user to move withing the sqrs -> battaglia navale
  switch (get_sw())
  {
    case 9: while(get_sw() == 9){draw_image(VGA, 13); if (get_btn()) {if(seq[counter] == 1) {counter++; break;} lives--; display_loss(lives);}} 
    case 10: while(get_sw() == 10){draw_image(VGA, 14); if (get_btn()) {if(seq[counter] == 2) {counter++; break;} lives--; display_loss(lives);}}
    case 12: while(get_sw() == 12){draw_image(VGA, 15); if (get_btn()) {if(seq[counter] == 3) {counter++; break;} lives--; display_loss(lives);}}
    case 17: while(get_sw() == 17){draw_image(VGA, 16); if (get_btn()) {if(seq[counter] == 4) {counter++; break;} lives--; display_loss(lives);}}
    case 18: while(get_sw() == 18){draw_image(VGA, 17); if (get_btn()) {if(seq[counter] == 5) {counter++; break;} lives--; display_loss(lives);}}
    case 20: while(get_sw() == 20){draw_image(VGA, 18); if (get_btn()) {if(seq[counter] == 6) {counter++; break;} lives--; display_loss(lives);}}
    case 33: while(get_sw() == 33){draw_image(VGA, 19); if (get_btn()) {if(seq[counter] == 7) {counter++; break;} lives--; display_loss(lives);}}
    case 34: while(get_sw() == 34){draw_image(VGA, 20); if (get_btn()) {if(seq[counter] == 8) {counter++; break;} lives--; display_loss(lives);}}
    case 36: while(get_sw() == 36){draw_image(VGA, 13); if (get_btn()) {if(seq[counter] == 9) {counter++; break;} lives--; display_loss(lives);}}
  }
}

void memory_game()
{
  display_money(); // show money on display
  
}

int main ( void ) // delays to adjust because it is slow in switching images
{
  labinit();
  
  // global logic
  while(1)
  {
    // Game selection
    while (get_sw() == 0x0 || get_sw() == 512) 
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
        while(1)
        {
          draw_image(VGA, 6); // descreact
          for (int i = 0; i < 3000000; i++) asm volatile ("nop"); 

          if (get_btn())
          {
            reactiongame();
          }

          if (get_sw() == 512)
          {
            break;
          }
        }
      }
    }

    // Memorize game selection
    while (get_sw() == 0x1) // arrow right
    {
      draw_image(VGA, 4); // memorize1
      for (int i = 0; i < 3000000; i++) asm volatile ("nop");

      draw_image(VGA, 5); // memorize2
      for (int i = 0; i < 3000000; i++) asm volatile ("nop");

      if (get_btn())
      {
        while(1)
        {
          draw_image(VGA, 12); // descmem
          for (int i = 0; i < 3000000; i++) asm volatile ("nop");

          if (get_btn())
          {
            // tutorial of the game
            draw_image(VGA, 29); // memtutorial1
            for (int i = 0; i < 3000000; i++) asm volatile ("nop");
            if (get_btn())
            {
              draw_image(VGA, 30); // memtutorial2
              for (int i = 0; i < 3000000; i++) asm volatile ("nop");
              if (get_btn())
              {
                memory_game();
              }
            }
          }

          if (get_sw() == 512)
          {
            break;
          }
        }
      }
    }
  }
}
