#include "include/utils.h"
#include <stdarg.h>

#ifdef DEBUG

extern void log_print_porting(const char *fmt, va_list *args);

void debug_log(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_print_porting(fmt, &args);
	va_end(args);
}

#else
void debug_log(const char *fmt, ...) {}
#endif // end DEBUG

int str2int(char *str) {
	int val = 0;
	for (const char *c = str; *c != '\0'; c++) {
		val *= 10;
		val += *c - '0';
	}
	return val;
}

int byte2int(uint8_t *buf, int len) {
	int val = 0;
	for (int i = 0; i < len; i++) {
		val *= 256;
		val += (int) buf[i];
	}
	return val;
}


uint32_t calc_bpkt_pc(uint32_t inst_addr) {
#ifdef SYS_CORTEX_M4
	uint32_t inst_val = (uint32_t) (inst_addr & ~(0x1));
	uint16_t thumb_inst = *(uint16_t *) inst_val;
	// DEBUG_LOG("calc_bpkt_pc 0x%08x -> 0x%08x\n", inst_addr, thumb_inst);
	if ((uint32_t) thumb_inst >= 0xe800) {
		return inst_val + 4;
	} else {
		return inst_val + 2;
	}
#endif

	return inst_addr;
}

/*
The previous inst maybe thumb32 or thumb16
*/
uint32_t clac_prev_inst_addr(uint32_t next_inst) {
#ifdef SYS_CORTEX_M4
	uint32_t inst_val = (uint32_t) (next_inst & ~(0x1)) - 4;
	uint16_t thumb_inst = *(uint16_t *) inst_val;
	// is the previous ints thumb16 ?
	// [12:11] bit = 00
	// 0b11101 = 0xe800, ARM v7-M Architecture Reference Manual, Page A5-126 (top)
	// check if is thumb32
	if ((uint32_t) thumb_inst >= 0xe800) {
		return inst_val;
	} else { // prev inst is thumb16
		return inst_val + 2;
	}
#endif

	return next_inst;
}


uint32_t little_endian_16_bit(uint32_t val) {
	return ((val & 0xFFFF0000) >> 16) | ((val & 0x0000FFFF) << 16);
}
