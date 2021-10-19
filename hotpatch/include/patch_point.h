#ifndef PATCH_POINT_H_
#define PATCH_POINT_H_
#include <stdint.h>
#include <stdbool.h>
#include "fixed_patch_points.h"

typedef enum patch_mode {
    CORTEX_FPB_PATCH = 0, // flash patch and breakpoint unit patch
    CORTEX_DEB_MON_PATCH, // debug monitor patch
} patch_mode_t;

// fixed patch point
// void add_fixed_patch_point();

// dynamic patch point
int get_hw_bkpt_num();
void set_patch_mode(patch_mode_t mode);
void show_hw_bkpt(void);
int add_hw_bkpt(uint32_t inst_addr);
uint32_t calc_bpkt_addr(uint32_t inst_addr);
void clear_all_hw_bkpt();

// test func
void test_debugmon_patch(void);
void trigger_debugmon(void);
int test_dynamic_bug(int val);

// for evaluation only
/* To make it easier to debug and evaluation in every devices
we load the patch from ram directly
*/
void show_local_patch_desc();
struct ebpf_patch* read_local_patch(int pid);

// load fixed patches
void show_fixed_patch_desc();
void load_local_fixed_patch(int pid);

#endif
