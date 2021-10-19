#ifndef UTILS_H_
#define UTILS_H_
#include <stdint.h>
#include "hotpatch/include/defs.h"

void debug_log(const char *fmt, ...);
/*
Used for normal debug.
*/
//#define DEBUG
// defined in defs.h 
#ifdef DEBUG 
#define DEBUG_LOG(...)								\
do {												\
	debug_log(__VA_ARGS__);							\
} while (0)
#else
#define DEBUG_LOG(...) do {} while(0)
#endif // end DEBUG

/*
Used in the debug of evaluation functions. Defined in defs.h 
*/
#ifdef USE_TEST_LOG
#define TEST_LOG(...)								\
do {												\
	debug_log(__VA_ARGS__);							\
} while (0)
#else
#define TEST_LOG(...) do {} while(0)
#endif // end TEST_LOG


int str2int(char *str);
int byte2int(uint8_t *buf, int len); // little_edian

// calculate pc address based on inst type (arm32 or thumb16)
uint32_t calc_bpkt_pc(uint32_t inst_addr);

// Thumb32, use get previous inst addr
uint32_t clac_prev_inst_addr(uint32_t next_inst);

uint32_t little_endian_16_bit(uint32_t val);

#endif // end header
