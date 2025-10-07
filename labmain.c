// ===================== config =====================
#define SCREEN_X 320
#define SCREEN_Y 240
#define IMAGE_SIZE (SCREEN_Y * SCREEN_X)

extern void enable_interrupt();

// =================== VGA =======================
volatile unsigned char* VGA_FRONT = (volatile unsigned char*)0x08000000;
volatile unsigned char* VGA_BACK = (volatile unsigned char*)0x08000000 + 0x12c00;
volatile unsigned char* VGA = (volatile unsigned char*)0x08000000;
volatile unsigned char* VGA_CTRL = (volatile unsigned char*) 0x04000100; // pointer to VGA DMA

// ================== global variables ========================
long timeout = 0;

// --- memory game ---
int coins = 100;
int lives = 3;
int seq[100];
int level = 1;
int bestlevel = 1;
int counter = 0;

// --- react time ---
static int bestime = 50000000;

// ================== pseudo randomizer =======================
static unsigned int seed = 123456789; // base
unsigned int rand(void) {
  seed = (1103515245U * seed + 12345U) & 0x7fffffffU;
  return seed % 24000000U;
}
void srand(unsigned int s) { seed = s; }

// ================== board functions prototype ========================
void set_displays(int display_number, int value);
int  get_sw(void);
int  get_btn(void);
void draw_image(volatile unsigned char *buf, int n);

// ==================== handle interrupt & labinit ======================

void handle_interrupt(unsigned cause) { 
    if(cause == 16){ 
    volatile int* status = (volatile int*)0x04000020;
    *status = 0x0;
    timeout++;
    timeout %= 51000000;
  }
}

void labinit(void) {
  volatile int *periodl = (volatile int*)0x04000028;
  volatile int *periodh = (volatile int*)0x0400002c;
  *periodl = 29999 & 0x0ffff; //0x1c9c380
  *periodh = 29999 >> 16 & 0xffff;

  volatile int *control = (volatile int*)0x04000024;
  *control = 0x7;

  volatile int* status = (volatile int*)0x04000020;
  *status = 0x1;

  enable_interrupt();
}

// ==================== board functions definitions ========================
void set_displays(int display_number, int value) {
  volatile int *display_base = (volatile int *)0x04000050;
  volatile int *display = display_base + (display_number * 4);

  switch (value) {
    case 0: *display = 0xc0; break;
    case 1: *display = 0xf9; break;
    case 2: *display = 0xa4; break;
    case 3: *display = 0xb0; break;
    case 4: *display = 0x99; break;
    case 5: *display = 0x92; break;
    case 6: *display = 0x82; break;
    case 7: *display = 0xf8; break;
    case 8: *display = 0x80; break;
    case 9: *display = 0x98; break;
  }
}

int get_sw(void) {
  volatile int *location = (volatile int*)0x04000010;
  return *location & 0x000003ff;
}

int get_btn(void) {
  volatile int *sndbutton = (volatile int*)0x040000d0;
  return *sndbutton & 0x1;
}

// ==================== draw image ============================
void draw_image(volatile unsigned char *buf, int n){
  /*volatile unsigned char* VGA = (VGA_FRONT == (volatile unsigned char*)0x08000000)
                                ? (volatile unsigned char*)(0x08000000 + 0x12c00)
                                : (volatile unsigned char*)0x08000000;*/

  volatile unsigned char *starting_bg = (volatile unsigned char*) (0x02000000 + n * IMAGE_SIZE);

  for (int y = 0; y < SCREEN_Y; y++)
  {
    for (int x = 0; x < SCREEN_X; x++) 
    {
      buf[y * SCREEN_X + x] = starting_bg[(y) * 320 + x];
    }
  }
  //*(VGA_CTRL+1) = (unsigned int)VGA;
  //*(VGA_CTRL+0) = 0;

  //VGA_FRONT = VGA;
}

// ===================== delay function ==========================
static inline void wait(int cycles) {
  for (int i = 0; i < cycles; i++) asm volatile ("nop");
}

// =================== memory game functions =======================
void display_money(void) {
  int i = coins % 10;
  int j = (coins / 10)   % 10;
  int w = (coins / 100)  % 10;
  int h = (coins / 1000) % 10;

  set_displays(0, i);
  set_displays(1, j);
  set_displays(2, w);
  set_displays(3, h);
}

void show_time(int yourtime) {
  set_displays(0, yourtime % 10);
  set_displays(1, (yourtime / 10) % 10);
  set_displays(2, (yourtime / 100) % 10);
  set_displays(3, yourtime / 1000);
}

void display_level(void) {

  draw_image(VGA, 30); // yourlevel
  set_displays(0, level % 10);
  set_displays(1, (level / 10) % 10);
  set_displays(2, 0);
  set_displays(3, 0);
  wait(24000000);

  draw_image(VGA, 31); // bestlevel
  set_displays(0, bestlevel % 10);
  set_displays(1, (bestlevel / 10) % 10); 
  set_displays(2, 0);
  set_displays(3, 0);
  wait(24000000);
}

void set_memory(int img_number) {

  switch (img_number) {
    case 1: draw_image(VGA, 16); wait(3000000); break;
    case 2: draw_image(VGA, 17); wait(3000000); break;
    case 3: draw_image(VGA, 18); wait(3000000); break;
    case 4: draw_image(VGA, 19); wait(3000000); break;
    case 5: draw_image(VGA, 20); wait(3000000); break;
    case 6: draw_image(VGA, 21); wait(3000000); break;
    case 7: draw_image(VGA, 22); wait(3000000); break;
    case 8: draw_image(VGA, 23); wait(3000000); break;
    case 9: draw_image(VGA, 24); wait(3000000); break;
  }
}

void memseq(int cur_level) {
  for (int i = 0; i < cur_level; i++) 
  {
    int img = 1 + (rand() % 9);
    set_memory(img);     
    seq[i] = img;        
  }
}

int user_sequence_step(void); // user sequence prototype

// ==================== machine states ======================
typedef enum 
{
  // main menu
  STATE_MAIN_MENU,

  // game selection
  STATE_SELECT_REACT_DESC,    
  STATE_SELECT_MEM_DESC,       
  STATE_SELECT_MEM_TUTORIAL1,
  STATE_SELECT_MEM_TUTORIAL2,

  // reaction game
  STATE_REACT_READY_1,
  STATE_REACT_READY_2,
  STATE_REACT_WAIT_RANDOM,
  STATE_REACT_GO_RUNNING,
  STATE_REACT_SHOW_RESULT,
  STATE_REACT_BEST_AND_AGAIN,
  STATE_REACT_TOO_LATE,

  // memory game
  STATE_MEM_READY,
  STATE_MEM_SHOW_SEQUENCE,
  STATE_MEM_WAIT_INPUT,
  STATE_MEM_GOODJOB,
  STATE_MEM_LOSE_LIFE,
  STATE_MEM_GAME_OVER,
  STATE_MEM_SHOW_LEVELS,

} GameState;

static GameState g_state = STATE_MAIN_MENU;

// =================== display loss ======================
void display_loss(int cur_lives) 
{

  switch (cur_lives) 
  {
    case 0: 
    {
      if (coins >= 100)
      {
        draw_image(VGA, 26); // buylife
      }
      else 
      {
        draw_image(VGA, 27);
      }
      wait(30000000);
      for (int i = 0; i < 12000000; i++) 
      {
        asm volatile ("nop");
        if (coins >= 100) 
        {
          if (get_btn())
          {
            coins -= 100;
            lives = 1;     
            g_state = STATE_MEM_READY;
            return;
          }  
        }
        if (get_sw() == 512) { g_state = STATE_MAIN_MENU; return; }
      }

      draw_image(VGA, 27); // youlosteverything
      wait(24000000);

      if (bestlevel < level) bestlevel = level;
      lives = 3;

      g_state = STATE_MEM_SHOW_LEVELS;
      return;
    }

    case 1: {
      draw_image(VGA, 29); // lives1
      wait(18000000);
      
      g_state = STATE_MEM_READY;
      return;
    }

    case 2: 
    {
      draw_image(VGA, 28); // lives2
      wait(18000000);
      g_state = STATE_MEM_READY;
      return;
    }
  }
}

// ===================== user sequence =====================
int user_sequence_step(void) 
{
  switch (get_sw()) 
  {
    case 0x9:
      while (get_sw() == 0x9) 
      {
        draw_image(VGA, 24);
        if (get_sw() == 521) return -1;
        if (get_btn()) {
          if (seq[counter] == 9) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    case 0xa:
      while (get_sw() == 0xa) {
        draw_image(VGA, 23);
        if (get_sw() == 522) return -1;
        if (get_btn()) {
          if (seq[counter] == 8) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    case 0xc:
      while (get_sw() == 0xc) {
        draw_image(VGA, 22);
        if (get_sw() == 524) return -1;
        if (get_btn()) {
          if (seq[counter] == 7) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    case 0x11:
      while (get_sw() == 0x11) 
      {
        draw_image(VGA, 21);
        if (get_sw() == 529) return -1;
        if (get_btn()) {
          if (seq[counter] == 6) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    case 18:
      while (get_sw() == 18) {
        draw_image(VGA, 20);
        if (get_sw() == 530) return -1;
        if (get_btn()) {
          if (seq[counter] == 5) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    case 20:
      while (get_sw() == 20) {
        draw_image(VGA, 19);
        if (get_sw() == 532) return -1;
        if (get_btn()) {
          if (seq[counter] == 4) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    case 33:
      while (get_sw() == 33) {
        draw_image(VGA, 18);
        if (get_sw() == 545) return -1;
        if (get_btn()) {
          if (seq[counter] == 3) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    case 34:
      while (get_sw() == 34) {
        draw_image(VGA, 17);
        if (get_sw() == 546) return -1;
        if (get_btn()) {
          if (seq[counter] == 2) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    case 36:
      while (get_sw() == 36) {
        draw_image(VGA, 16);
        if (get_sw() == 548) return -1;
        if (get_btn()) {
          if (seq[counter] == 1) { wait(5000000); counter++; return 1; }
          lives--;
          display_loss(lives);
          return 0;
        }
      }
      break;

    default:
      break;
  }
  return 2;
}

// ===================== main =====================
int main(void) 
{
  labinit();

 g_state = STATE_MAIN_MENU; // main menu

  while (1) 
  {

    if (get_sw() == 512 && g_state != STATE_MAIN_MENU) {
      g_state = STATE_MAIN_MENU;
    }

    switch (g_state) {

      case STATE_MAIN_MENU:
      {
        set_displays(0,0);
        set_displays(1,0);
        set_displays(2,0);
        set_displays(3,0);

        draw_image(VGA, 0); wait(5000000); // gameselection1
        draw_image(VGA, 1); wait(5000000); // gameselection2

        // select reach sx
        if (get_sw() == 0x8) {
          g_state = STATE_SELECT_REACT_DESC;
          break;
        }
        // select memory dx
        if (get_sw() == 0x1) {
          g_state = STATE_SELECT_MEM_DESC;
          break;
        }
      } break;

      // ======= reaction game selection =======
      case STATE_SELECT_REACT_DESC: {
        draw_image(VGA, 2); wait(1500000); // reactime1
        draw_image(VGA, 3); wait(1500000); // reactime2
        if (get_btn()) {
          g_state = STATE_REACT_READY_1;
        }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      // ------ game description ------
      case STATE_REACT_READY_1: {
        draw_image(VGA, 6); wait(3000000); // descreact
        if (get_btn()) { g_state = STATE_REACT_READY_2; }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      case STATE_REACT_READY_2: {
        draw_image(VGA, 7); // reactready
        
        srand(timeout); // random delay
        int randelay = 3000000 + (int)rand();
        wait(randelay);
        timeout = 0;
        g_state = STATE_REACT_GO_RUNNING;
      } break;

      case STATE_REACT_GO_RUNNING: {
        draw_image(VGA, 8); // reactgo
        
        while (timeout < 3000) { // generate random delay
          asm volatile ("nop");
          if (get_btn()) {
            int yourtime = (int)(timeout);
            if (bestime > yourtime) bestime = yourtime;
            
            draw_image(VGA, 9); // lookboard
            show_time(yourtime);
            wait(50000000);
            
            g_state = STATE_REACT_BEST_AND_AGAIN;
            break;
          }
          if (get_sw() == 512) { g_state = STATE_MAIN_MENU; break; }
        }
        
        if (g_state == STATE_REACT_GO_RUNNING) { // so that it goes in here only if the button was triggered and the state didn't change
          g_state = STATE_REACT_TOO_LATE;
        }
        break;
      }

      case STATE_REACT_BEST_AND_AGAIN: {
        draw_image(VGA, 10); // bestime & play again
        show_time(bestime);
        
        if (get_btn()) { // play again
          g_state = STATE_REACT_READY_2; // restart
        }
        if (get_sw() == 512) {
          g_state = STATE_MAIN_MENU;
        }
      } break;

      case STATE_REACT_TOO_LATE: {
        draw_image(VGA, 11); // toolate

        if (get_btn()) { // either restart or esc
          g_state = STATE_REACT_READY_2;
        } else if (get_sw() == 512) {
          g_state = STATE_MAIN_MENU;
        }
      } break;

      // ============== memory game selection =============
      case STATE_SELECT_MEM_DESC: 
      {
        draw_image(VGA, 4); wait(1500000);
        draw_image(VGA, 5); wait(1500000);
        if (get_btn()) {
          g_state = STATE_SELECT_MEM_TUTORIAL1;
        }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      // --------- game description ----------
      case STATE_SELECT_MEM_TUTORIAL1: {
        draw_image(VGA, 12); wait(1500000); // descmem
        if (get_btn()) {
          g_state = STATE_SELECT_MEM_TUTORIAL2;
        }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      case STATE_SELECT_MEM_TUTORIAL2: {
        draw_image(VGA, 13); wait(1500000); // memtutorial1
        if (get_btn()) 
        {
            while(1)
            {
                draw_image(VGA, 14); wait(1500000); // memtutorial2
                if (get_btn()) 
                { // start game
                    lives = 3;
                    level = 1;
                    counter = 0;
                
                    srand((unsigned int)(timeout ^ 0xA5A5A5A5)); // initial pseudo-randomic seed
                    g_state = STATE_MEM_READY;
                    break;
                }
            }
        }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      // ======= set up game =======
      case STATE_MEM_READY: {
        display_money();
        counter = 0;
        draw_image(VGA, 15); // sqr0
        wait(9000000);
        // before generating a new sequence, changes the seed for a greater level of randomness
        srand((unsigned int)(seed ^ level ^ timeout));
        g_state = STATE_MEM_SHOW_SEQUENCE;
      } break;

      case STATE_MEM_SHOW_SEQUENCE: {
        memseq(level);      // display & save seq
        draw_image(VGA, 15); // sqr0
        wait(9000000);
        counter = 0;
        g_state = STATE_MEM_WAIT_INPUT;
      } break;

      case STATE_MEM_WAIT_INPUT: {
        // user input
        int r = user_sequence_step();
        if (r == -1) { g_state = STATE_MAIN_MENU; break; } // esc
        if (r == 0) { /* wrong */} 
        if (r == 1) { 
          // create image "correct!"
          draw_image(VGA, 33); // correct selection
          wait(10000000);
          // correct if the whole sequence has been completed
          if (counter == level) {
            g_state = STATE_MEM_GOODJOB;
          }
        }
      } break;

      case STATE_MEM_GOODJOB: {
        counter = 0;
        draw_image(VGA, 25); // goodjob
        coins += (2*level);
        if (level % 5 == 0) coins *= 2;
        wait(18000000);
        level++;
        g_state = STATE_MEM_READY; // next level
      } break;

      case STATE_MEM_SHOW_LEVELS: 
      {
        display_level();
        // after having shown the level restart a new game
        while(1)
        {
          if (get_btn())
          {
            lives = 3;
            level = 1;
            counter = 0;
            g_state = STATE_MEM_READY;
            break;
          }
          if (get_sw() == 512)
          {
            g_state = STATE_MAIN_MENU;
            break;
          }
        }
      } break;

      default:
        g_state = STATE_MAIN_MENU;
        break;
    }
  } 
  return 0;
}
