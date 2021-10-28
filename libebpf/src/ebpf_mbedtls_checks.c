#include "include/ebpf_mbedtls_checks.h"
#include "ebpf_env.h"

const unsigned char code_cve_9989[] = ""
"\x61\x12\x00\x00\x00\x00\x00\x00\x61\x22\x00\x00\x00\x00\x00\x00\x71\x23\x00\x00\x00\x00\x00\x00\x67"
"\x03\x00\x00\x08\x00\x00\x00\x71\x24\x01\x00\x00\x00\x00\x00\x4f\x43\x00\x00\x00\x00\x00\x00\x0f\x32"
"\x00\x00\x00\x00\x00\x00\x61\x11\x04\x00\x00\x00\x00\x00\xb7\x00\x00\x00\x01\x00\x00\x00\x2d\x12\x01"
"\x00\x00\x00\x00\x00\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

struct mbedTLSInsertPoints {
	ebpf_prog *prog_9989; // CVE-2018-9989
} mbed_progs;

int setup_mbedtls_ebpf_progs() {
    //mbed_progs.prog_9989 = ebpf_add_prog(code_cve_9989, sizeof(code_cve_9989));
    return 0;
}

void remove_mbedtls_ebpf_progs() {

}

int run_prog_9989(prog_9989_args *args) {
    if (mbed_progs.prog_9989 != NULL) {
        // mbed_progs.prog_9989.
        //u64 ret = ebpf_vm_exec(mbed_progs.prog_9989->vm, args, sizeof(prog_9989_args));
        //printf("vm ret: %llu\n", ret);
        //return ret;
    }
    return -1;
}