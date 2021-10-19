#ifndef EBPF_TEST_H_
#define EBPF_TEST_H_
#include <stdint.h>

void run_ebpf_test();
void run_ebpf_eva(int eid);
uint64_t jit_dummy_cve();

#endif
