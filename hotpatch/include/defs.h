#ifndef DEFS_H_
#define DEFS_H_

// #define USE_FIXED_PATCH_INSTRUMENT 1

// --------------------------------------------------------------
// 0 false, do not use dynamic patch point, use fixed patch point
#define USE_DYNAMIC_PATCH 1
#define USE_FIX_PATCH 0

// --------------------------------------------------------------
// use debug log, can use everywhere
#define DEBUG
#define USE_TEST_LOG
// used in the evaluated functions, thus we can disable these log during evaluation
// #define USE_TEST_LOG

// --------------------------------------------------------------
#define EVA_TEST

#ifdef EVA_TEST
	#define EBPF_EVA
#endif

#endif // end DEFS_H_