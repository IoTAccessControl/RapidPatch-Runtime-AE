#include "include/patch_point.h"
#include "include/utils.h"

#ifdef SYS_CORTEX_M4
// #include "cortex-m4_fbp.c"
#endif

#ifdef LINUX_TEST

void show_hw_bkpt(void) {
	DEBUG_LOG("Linux donot support dynamic patch point.\n");
}

void set_patch_mode(patch_mode_t mode) {
	
}

int add_hw_bkpt(uint32_t inst_addr) {
	return -1;
}

void clear_all_hw_bkpt() {
	
}

uint32_t get_hw_bkpt_addr(int idx) {
	return 0;
}

uint32_t calc_bpkt_addr(uint32_t inst_addr) {
	return 0;
}
#endif 

