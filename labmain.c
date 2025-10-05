// ===================== CONFIG E HW =====================
#define SCREEN_X 320
#define SCREEN_Y 240
#define IMAGE_SIZE (SCREEN_X * SCREEN_Y)

extern void enable_interrupt();

// ===================== GLOBALI HW =====================
volatile unsigned char* VGA = (volatile unsigned char*)0x08000000;
volatile int *VGA_CTRL = (volatile int*) 0x04000100; // pointer to VGA DMA

// ===================== VARIABILI GLOBALI =====================
long timeout = 0;

// --- Memory game ---
int coins = 100;
int lives = 3;
int seq[100];
int level = 1;
int bestlevel = 1;
int counter = 0;

// --- Reaction game ---
static int bestime = 50000000;

// --- RNG semplice ---
static unsigned int seed = 123456789; // base
unsigned int rand(void) {
  seed = (1103515245U * seed + 12345U) & 0x7fffffffU;
  return seed % 24000000U;
}
void srand(unsigned int s) { seed = s; }

// ===================== PROTOTIPI FUNZIONI HW =====================
void set_leds(int led_mask);
void set_displays(int display_number, int value);
int  get_sw(void);
int  get_btn(void);
void draw_image(volatile unsigned char *buf, int n);

// ===================== INTERRUZIONI / TIMER =====================
unsigned int y_ofs= 0;
void handle_interrupt(unsigned cause) {  
  volatile int* status = (volatile int*)0x04000020;
  *status = 0x0;
  timeout++;
  timeout %= 51000000;
}

void labinit(void) {
  volatile int *periodl = (volatile int*)0x04000028;
  volatile int *periodh = (volatile int*)0x0400002c;
  *periodl = 0x28;
  *periodh = 0x0;

  volatile int *control = (volatile int*)0x04000024;
  *control = 0x7;

  volatile int* status = (volatile int*)0x04000020;
  *status = 0x1;

  enable_interrupt();
}

// ===================== FUNZIONI HW =====================
void set_leds(int led_mask) {
  volatile int *leds = (volatile int*)0x04000000;
  *leds = led_mask;
}

void set_displays(int display_number, int value) {
  volatile int *display_base = (volatile int *)0x04000050;
  volatile int *display = display_base + (display_number * 4);

  switch (value) {
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

int get_sw(void) {
  volatile int *location = (volatile int*)0x04000010;
  return *location & 0x000002ff;
}

int get_btn(void) {
  volatile int *sndbutton = (volatile int*)0x040000d0;
  return *sndbutton & 0x1;
}

void draw_image(volatile unsigned char *buf, int n) {
  const unsigned char *starting_bg = (const unsigned char*) (0x02000000 + n * IMAGE_SIZE);
  for (int y = 0; y < SCREEN_Y; y++) {
    for (int x = 0; x < SCREEN_X; x++) {
      buf[y * SCREEN_X + x] = starting_bg[(y) * 320 + x];
    }
  }
}

// ===================== HELPERS GENERICI =====================
static inline void busy_wait(int cycles) {
  for (int i = 0; i < cycles; i++) asm volatile ("nop");
}

// ===================== DISPLAY UTILI =====================
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
  draw_image(VGA, 26); // yourlevel
  set_displays(0, level % 10);
  set_displays(1, (level / 10) % 10);
  busy_wait(24000000);

  draw_image(VGA, 27); // currentbest
  set_displays(0, bestlevel % 10);
  set_displays(1, (bestlevel / 10) % 10); // <-- fix
  busy_wait(24000000);
}

// ===================== MEMORY GAME: SEQUENZA =====================
void set_memory(int img_number) {
  switch (img_number) {
    case 0: draw_image(VGA, 22); busy_wait(3000000); break;
    case 1: draw_image(VGA, 13); busy_wait(3000000); break;
    case 2: draw_image(VGA, 14); busy_wait(3000000); break;
    case 3: draw_image(VGA, 15); busy_wait(3000000); break;
    case 4: draw_image(VGA, 16); busy_wait(3000000); break;
    case 5: draw_image(VGA, 17); busy_wait(3000000); break;
    case 6: draw_image(VGA, 18); busy_wait(3000000); break;
    case 7: draw_image(VGA, 19); busy_wait(3000000); break;
    case 8: draw_image(VGA, 20); busy_wait(3000000); break;
    case 9: draw_image(VGA, 21); busy_wait(3000000); break;
  }
}

// genera e mostra la sequenza del livello corrente (seed impostato a parte)
void memseq(int cur_level) {
  for (int i = 0; i < cur_level; i++) {
    int img = 1 + (rand() % 9);
    set_memory(img);     // mostra immagine + delay
    seq[i] = img;        // salva in sequenza
  }
}

// Una singola “mossa” dell’utente: blocca finché l’utente non rilascia/decide come nel tuo codice originale.
// Ritorna 1 se ha premuto il riquadro corretto (counter++ avviene qui), 0 se ha sbagliato (e gestisce vite/stato),
// -1 se è stato richiesto il ritorno al menu con SW == 512.
int user_sequence_step(void);

// ===================== STATE MACHINE =====================
typedef enum {
  // Menu principale
  STATE_MAIN_MENU,

  // Scelta giochi
  STATE_SELECT_REACT_DESC,     // schermo descrittivo reaction
  STATE_SELECT_MEM_DESC,       // schermo descrittivo memory
  STATE_SELECT_MEM_TUTORIAL1,
  STATE_SELECT_MEM_TUTORIAL2,

  // Reaction game
  STATE_REACT_READY_1,
  STATE_REACT_READY_2,
  STATE_REACT_WAIT_RANDOM,
  STATE_REACT_GO_RUNNING,
  STATE_REACT_SHOW_RESULT,
  STATE_REACT_BEST_AND_AGAIN,
  STATE_REACT_TOO_LATE,

  // Memory game
  STATE_MEM_READY,
  STATE_MEM_SHOW_SEQUENCE,
  STATE_MEM_WAIT_INPUT,
  STATE_MEM_GOODJOB,
  STATE_MEM_LOSE_LIFE,
  STATE_MEM_GAME_OVER,
  STATE_MEM_SHOW_LEVELS,

} GameState;

static GameState g_state = STATE_MAIN_MENU;

// --------- Gestione perdita vite/partita (NON ricorsiva) ---------
void display_loss_and_set_state(int cur_lives) {
  // Nota: questa funzione NON richiama memory_game(); imposta solo il nuovo stato
  // e fa i disegni con i delay necessari, poi ritorna al loop principale che prosegue.
  switch (cur_lives) {
    case 0: {
      // Schermata: "spendi 100?" (img 23) – disegna una sola volta e aspetta
      draw_image(VGA, 23);
      // Finestra in cui l'utente può premere per spendere 100 e continuare
      for (int i = 0; i < 12000000; i++) {
        asm volatile ("nop");
        if (get_btn() && coins >= 100) {
          coins -= 100;
          lives = 1;     // recupera una vita e riparte
          g_state = STATE_MEM_READY;
          return;
        }
        // Esc con SW==512: ritorno al menu
        if (get_sw() == 512) { g_state = STATE_MAIN_MENU; return; }
      }

      // Non ha “comprato” la continuazione: ha perso tutto
      draw_image(VGA, 31); // youlosteverything
      busy_wait(24000000);

      if (bestlevel < level) bestlevel = level;
      lives = 3;
      level = 1;

      // Mostra i livelli (come da codice originale)
      g_state = STATE_MEM_SHOW_LEVELS;
      return;
    }

    case 1: {
      draw_image(VGA, 25); // 1 vita rimasta
      busy_wait(18000000);
      // Riparte lo stesso livello mostrandone di nuovo la sequenza
      g_state = STATE_MEM_READY;
      return;
    }

    case 2: {
      draw_image(VGA, 24); // 2 vite rimaste
      busy_wait(18000000);
      // Riparte lo stesso livello mostrandone di nuovo la sequenza
      g_state = STATE_MEM_READY;
      return;
    }
  }
}

// ===================== USER SEQUENCE (passo singolo) =====================
int user_sequence_step(void) {
  // Stessa logica dei tuoi case con while(get_sw()==X) che attendono il click.
  // Alla fine ritorna per lasciare avanzare la state machine.
  switch (get_sw()) {
    case 9:
      while (get_sw() == 9) {
        draw_image(VGA, 13);
        if (get_btn()) {
          if (seq[counter] == 1) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    case 10:
      while (get_sw() == 10) {
        draw_image(VGA, 14);
        if (get_btn()) {
          if (seq[counter] == 2) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    case 12:
      while (get_sw() == 12) {
        draw_image(VGA, 15);
        if (get_btn()) {
          if (seq[counter] == 3) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    case 17:
      while (get_sw() == 17) {
        draw_image(VGA, 16);
        if (get_btn()) {
          if (seq[counter] == 4) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    case 18:
      while (get_sw() == 18) {
        draw_image(VGA, 17);
        if (get_btn()) {
          if (seq[counter] == 5) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    case 20:
      while (get_sw() == 20) {
        draw_image(VGA, 18);
        if (get_btn()) {
          if (seq[counter] == 6) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    case 33:
      while (get_sw() == 33) {
        draw_image(VGA, 19);
        if (get_btn()) {
          if (seq[counter] == 7) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    case 34:
      while (get_sw() == 34) {
        draw_image(VGA, 20);
        if (get_btn()) {
          if (seq[counter] == 8) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    case 36:
      while (get_sw() == 36) {
        draw_image(VGA, 21);
        if (get_btn()) {
          if (seq[counter] == 9) { counter++; return 1; }
          lives--;
          display_loss_and_set_state(lives);
          if (get_sw() == 512) return -1;
          return 0;
        }
      }
      break;

    default:
      break;
  }
  return 1; // nessun input significativo: continua
}

// ===================== MAIN =====================
int main(void) {
  labinit();

  // Stato iniziale = menu principale
  g_state = STATE_MAIN_MENU;

  while (1) {

    // Aggiorna sempre (o spesso) i 7-segment con i soldi quando siamo nel memory o nei menu
    // (se i tuoi display sono latchati, non serve; se sono multiplexati, questo aiuta a tenerli vivi)
    display_money();

    // ESC globale dai giochi verso menu
    if (get_sw() == 512 && g_state != STATE_MAIN_MENU) {
      g_state = STATE_MAIN_MENU;
    }

    switch (g_state) {

      // ======= MENU PRINCIPALE: animazione e scelte =======
      case STATE_MAIN_MENU: {
        // Animazione alternata
        draw_image(VGA, 0); busy_wait(3000000);
        draw_image(VGA, 1); busy_wait(3000000);

        // Selezione Reaction a sx (0x8)
        if (get_sw() == 0x8) {
          g_state = STATE_SELECT_REACT_DESC;
          break;
        }
        // Selezione Memory a dx (0x1)
        if (get_sw() == 0x1) {
          g_state = STATE_SELECT_MEM_DESC;
          break;
        }
      } break;

      // ======= REACTION: schermata descrizione =======
      case STATE_SELECT_REACT_DESC: {
        draw_image(VGA, 2); busy_wait(3000000);
        draw_image(VGA, 3); busy_wait(3000000);
        if (get_btn()) {
          g_state = STATE_REACT_READY_1;
        }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      // ======= REACTION: loop ready (come nel codice originale) =======
      case STATE_REACT_READY_1: {
        draw_image(VGA, 6); busy_wait(3000000); // descrizione
        if (get_btn()) { g_state = STATE_REACT_READY_2; }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      case STATE_REACT_READY_2: {
        draw_image(VGA, 7); // reactready
        // prepara delay casuale
        srand(timeout);
        int randelay = 3000000 + (int)rand();
        busy_wait(randelay);
        timeout = 0;
        g_state = STATE_REACT_GO_RUNNING;
      } break;

      case STATE_REACT_GO_RUNNING: {
        draw_image(VGA, 8); // reactgo
        // timer via timeout (interruzione)
        while (timeout < 50000000) {
          asm volatile ("nop");
          if (get_btn()) {
            int yourtime = (int)((timeout * 40) / 1000000);
            if (bestime > yourtime) bestime = yourtime;
            // Mostra risultato e best, poi chiedi replay
            draw_image(VGA, 9);  // lookboard per il tuo tempo
            show_time(yourtime);
            busy_wait(50000000);
            g_state = STATE_REACT_BEST_AND_AGAIN;
            break;
          }
          if (get_sw() == 512) { g_state = STATE_MAIN_MENU; break; }
        }
        if (timeout >= 50000000) {
          g_state = STATE_REACT_TOO_LATE;
        }
      } break;

      case STATE_REACT_BEST_AND_AGAIN: {
        draw_image(VGA, 11); // bestime & play again
        show_time(bestime);
        // play again
        if (get_btn()) {
          g_state = STATE_REACT_READY_2; // riparte
        }
        if (get_sw() == 0x10) {
          g_state = STATE_MAIN_MENU;
        }
      } break;

      case STATE_REACT_TOO_LATE: {
        draw_image(VGA, 10); // toolate
        // Attesa di click o esc
        if (get_btn()) {
          g_state = STATE_REACT_READY_2;
        } else if (get_sw() == 512) {
          g_state = STATE_MAIN_MENU;
        }
      } break;

      // ======= MEMORY: schermata descrizione e tutorial =======
      case STATE_SELECT_MEM_DESC: {
        draw_image(VGA, 4); busy_wait(3000000);
        draw_image(VGA, 5); busy_wait(3000000);
        if (get_btn()) {
          g_state = STATE_SELECT_MEM_TUTORIAL1;
        }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      case STATE_SELECT_MEM_TUTORIAL1: {
        draw_image(VGA, 12); busy_wait(3000000);
        if (get_btn()) {
          g_state = STATE_SELECT_MEM_TUTORIAL2;
        }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      case STATE_SELECT_MEM_TUTORIAL2: {
        draw_image(VGA, 29); busy_wait(3000000);
        if (get_btn()) {
          draw_image(VGA, 30); busy_wait(3000000);
          if (get_btn()) {
            // RESET di partita memory
            lives = 3;
            level = 1;
            counter = 0;
            // seed iniziale dalla variabile di sistema timeout
            srand((unsigned int)(timeout ^ 0xA5A5A5A5));
            g_state = STATE_MEM_READY;
          }
        }
        if (get_sw() == 512) g_state = STATE_MAIN_MENU;
      } break;

      // ======= MEMORY: pronto a mostrare la sequenza =======
      case STATE_MEM_READY: {
        display_money();
        counter = 0;
        draw_image(VGA, 22); // schermo "center/ready"
        busy_wait(9000000);
        // prima di generare la sequenza, “ritocca” il seed per il nuovo livello
        srand((unsigned int)(seed ^ level ^ timeout));
        g_state = STATE_MEM_SHOW_SEQUENCE;
      } break;

      case STATE_MEM_SHOW_SEQUENCE: {
        memseq(level);                // mostra e salva seq
        draw_image(VGA, 22);
        busy_wait(9000000);
        counter = 0;
        g_state = STATE_MEM_WAIT_INPUT;
      } break;

      case STATE_MEM_WAIT_INPUT: {
        // passo di input utente
        int r = user_sequence_step();
        if (r == -1) { g_state = STATE_MAIN_MENU; break; } // ESC
        if (r == 0) {
          // ha sbagliato: display_loss_and_set_state ha già impostato g_state
          // Se dopo la perdita vite si è passati a READY o MENU, lasciamo che lo switch prosegua
        } else {
          // corretto → se ha completato tutta la sequenza
          if (counter == level) {
            g_state = STATE_MEM_GOODJOB;
          }
        }
      } break;

      case STATE_MEM_GOODJOB: {
        counter = 0;
        draw_image(VGA, 28); // goodjob
        busy_wait(18000000);
        level++;
        coins += (2*level);
        if (level % 5 == 0) coins *= 2;
        g_state = STATE_MEM_READY; // prossimo livello
      } break;

      case STATE_MEM_SHOW_LEVELS: {
        display_level();
        // dopo aver mostrato livelli, ricomincia nuova partita memory
        lives = 3;
        level = 1;
        counter = 0;
        g_state = STATE_MEM_READY;
      } break;

      default:
        g_state = STATE_MAIN_MENU;
        break;
    } // switch(g_state)
  } // while(1)
  // non si arriva qui
  return 0;
}
