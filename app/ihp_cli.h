#ifndef CLI_H_
#define CLI_H_
#include "ihp_porting.h"

typedef enum patch_tests {
	TEST_LOG = 0,
	TEST_FPB,
	TEST_CLEAR,
	TEST_EBPF,
	TEST_SVR,
	TEST_FIXED_PATCH_POINT,
} run_test_t;

// triggered by shell commands
void run_shell_cli(void);

// triggered by button
void run_test_by_id(run_test_t cid);


#endif
