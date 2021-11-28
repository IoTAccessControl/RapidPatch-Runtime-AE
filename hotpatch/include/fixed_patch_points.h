#ifndef FIXED_PATCH_POINTS_H_
#define FIXED_PATCH_POINTS_H_
#include <stdint.h>
#include "libebpf/include/hashmap.h"
#include "defs.h"

// TODO: Remove this file, combile

// fixed patch point
// enum FixedPatchPointsDef {
// 	DUMMY_CVE1_PATCH_PT = 0,
// 	FREERTOS_IP_STACK_PATCH_PT, // 1

// };

// typedef struct FixedPatchPoints {
// 	//uintptr_t *dummy_cve_patch_point;
// 	arraymap patches;
// } FixedPatchPoints;

int fixed_patch_point_hanlder();


// int dummy_cv1_patch_point();
void test_fixed_patch_point();

void test_unbounded_loop();

#endif
