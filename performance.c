//Measure CPU hardware counters and print to the terminal

#include "performance.h"

extern void printc();
extern void print();
extern void print_dec();

int mcycles, minstret, mhpmcounter3, mhpmcounter4, mhpmcounter5;
int mhpmcounter6, mhpmcounter7, mhpmcounter8, mhpmcounter9;

void analysis() {
  asm("csrr %0, mcycle" : "=r"(mcycles) );    // Read mcycles

  print("\nTime for reaction game() was: ");  //Print value
  print_dec(mcycles);
  print("\n");
  
  asm("csrr %0, minstret" : "=r"(minstret) );    // Read number of instructions retired

  print("\nMinstret: ");  //Print value
  print_dec(minstret);
  print("\n"); 

  asm("csrr %0, mhpmcounter3" : "=r"(mhpmcounter3) );    // Read number of memory instructions retired

  print("\nmhpmcounter3: ");  //Print value
  print_dec(mhpmcounter3);
  print("\n"); 

  asm("csrr %0, mhpmcounter4" : "=r"(mhpmcounter4) );    // Read number of times fetch resulted in I cache miss

  print("\nmhpmcounter4: ");  //Print value
  print_dec(mhpmcounter4);
  print("\n"); 

  asm("csrr %0, mhpmcounter5" : "=r"(mhpmcounter5) );    // Read number of times memory operation resulted in D cache miss

  print("\nmhpnmcounter5: ");  //Print value
  print_dec(mhpmcounter5);
  print("\n"); 

  asm("csrr %0, mhpmcounter6" : "=r"(mhpmcounter6) );    // Read number of CPU stalls due to I-cache miss

  print("\nCPU stalls due to I-cache misses: ");  //Print value
  print_dec(mhpmcounter6);
  print("\n"); 

  asm("csrr %0, mhpmcounter7" : "=r"(mhpmcounter7) );    // Read number of CPU stalls fue to D-cache miss

  print("\nCPU stalls due to D-cache misses:");  //Print value
  print_dec(mhpmcounter7);
  print("\n"); 

  asm("csrr %0, mhpmcounter8" : "=r"(mhpmcounter8) );    // Read number of CPU stalls due to data hazards

  print("\nCPU stalls due to data hazards:");  //Print value
  print_dec(mhpmcounter8);
  print("\n"); 

  asm("csrr %0, mhpmcounter9" : "=r"(mhpmcounter9) );    // Read number of CPU stalls fue to ALU

  print("\nCPU stalls due to ALU:");  //Print value
  print_dec(mhpmcounter9);
  print("\n");  
}
