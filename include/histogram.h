#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define ADDR_PTR uint64_t
#define CYCLES uint64_t

//----------------------------------------------------------------
// PRIMITIVES FOR FLUSH+RELOAD CHANNEL
//----------------------------------------------------------------

/* Flush a cache block of address "addr" */
extern inline __attribute__((always_inline))
void clflush(ADDR_PTR addr)
{
  //TODO: Use clflush instruction.
  asm volatile ("clflush (%0)"
		: /*output*/ 
		: /*input*/ "r"(addr)
		: /*clobbers*/ "memory");
}


/* Load address "addr" */
void maccess(ADDR_PTR addr)
{

  //TODO: Use mov instruction.
  asm volatile("mov (%0), %%rax"
	       : /*output*/
	       : /*input*/ "r"(addr)
	       : /*clobbers*/ "rax", "memory" );
  
  return;
}


/* Loads addr and measure the access time */
CYCLES maccess_t(ADDR_PTR addr)
{
  CYCLES cycles;

  // TODO:
  // Use a mov instruction to load an address "addr",
  // which is sandwiched between two rdtscp instructions.
  // Calculate the latency using difference of the output of two rdtscps.
  CYCLES start, end;
 asm volatile(
    // Get start time
    "lfence\n"                "\n"
    "rdtscp\n"                "\n"
    "shl $32, %%rdx\n"        "\n"
    "or %%rdx, %%rax\n"       "\n"
    "mov %%rax, %0\n"         "\n"
    
    // Memory access to target address
    
    "mov (%2), %%rax\n"       "\n"
    "lfence\n"                "\n"
    
    // Get end time
    "rdtscp\n"                "\n"
    "shl $32, %%rdx\n"        "\n"
    "or %%rdx, %%rax\n"       "\n"
    "mov %%rax, %1\n"
    : "=r"(start), "=r"(end)           // Output operands: start and end times
    : "r"(addr)                       // Input operand: address to read from
    : "rax", "rcx", "rdx", "memory"    // Clobbered registers and memory
);

  cycles = end - start;
  return cycles;
}


/* Returns Time Stamp Counter (using rdtscp function)*/
extern inline __attribute__((always_inline))
uint64_t rdtscp(void) {
  uint64_t cycles;
  asm volatile ("rdtscp\n"
		"shl $32,%%rdx\n"
		"or %%rdx, %%rax\n"		      
		: /* outputs */ "=a" (cycles));
  return cycles;
}
