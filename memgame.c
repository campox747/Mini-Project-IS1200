#include "memgame.h"
#include "gamestate.h"

//=====================VGA=======================
extern volatile unsigned char* VGA;

//=====================Memory game functions=======================

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