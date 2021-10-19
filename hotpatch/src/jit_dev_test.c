#include "include/ebpf_test.h"
#include "include/utils.h"

#define __NAKE __attribute__((naked))

static char jit_buffer[2048];

typedef void (*f_void)(void);
typedef uint64_t (*f_ebpf)(void*);

int jit_int_sum(uint16_t* instrs, int n) {
	// 0 1 0 0 0 1 1 0
	int idx = 0;
	if (n == 0) {
		// A7-291
		// MOV   Rd  Value
		// 00100 000 00000000
		// 2   0     0   0
		instrs[idx++] = 0x2000;
	} else {
		for (unsigned k = 1; k != n; ++k) {
			// 0 0 0 1 1 0 0 Rm Rn Rd
			// Rd := Rn + Rm
			// Add     Rm  Rn  Rd
			// 0001100 000 xxx 000
			// 1   8    0    0
			instrs[idx++] = (0x1800 | (k << 3));
		}
	}
	instrs[idx++] = (0x4770); // Return

	__asm("DSB");
	__asm("ISB");
	//barrior();
	typedef int (*f_4_int_to_int)(int, int, int, int);
	f_4_int_to_int fp = (f_4_int_to_int)((uint32_t) instrs | 0x1);
	return fp(1, 2, 3, 4);
}

uint64_t jit_dummy_cve() {
	uint16_t *instptr = (uint16_t *) ((uint32_t) jit_buffer & (~0x3));
	DEBUG_LOG("jit_dummy_cve func addr: 0x%08x\n", instptr);
	int ret = jit_int_sum(instptr, 4);
	DEBUG_LOG("jit_int_sum: %d\n", ret);
}

int add_mum(int a, int b, int c, int d) {
    int sum = 0;
    sum += a;
    sum += b;
    sum += c;
    sum += d;
    return sum;
}

// ARM®v7-M Architecture Reference Manual
void first_jit_func() {
    uint16_t* instrs = (uint16_t *) ((uint32_t) jit_buffer & (~0x3));
    // my_printf("Run jit test: %p -> %p\n", jit_buffer, instrs);
    __asm__("DSB");
    __asm__("ISB");
    /*
    Page:
    bx: 0 1 0 0 0 1 1 1 0, 47
    lr: r14, 0 1110 000, 70
    */
    // instrs[0] = 
    instrs[0] = 0x4770; // bx lr
    f_void fp = (f_void)((unsigned)instrs | 0x1);
    fp();
    add_mum(7, 2, 3, 4);
    // my_printf("jit int sum: %d\n", jit_int_sum(instrs, 4));

    int src = 1;
    int dst;   

//    asm ("mov r1, %0\n\t"
//        "add r1, %0"
//        : "=r" (dst) 
//        : "r" (src));

    //my_printf("%d\n", dst);
}

static uint16_t my_jit_func2(void *ctx) {
    uint16_t* instrs = (uint16_t *) ((uint32_t) jit_buffer & (~0x3));
    instrs[0] = 0x4600 | (0 << 3) | (1); // bx lr
    instrs[1] = 0x4770; // bx lr
    __asm("DSB");
    __asm("ISB");
    f_ebpf fp = (f_ebpf)((unsigned)instrs | 0x1);
    uint8_t * d = (uint8_t *) ((uint32_t) jit_buffer & (~0x3));
    DEBUG_LOG("my_jit_func2 start\n");
    for (int i = 0; i < 4; i++) {
        DEBUG_LOG("0x%x ", (uint8_t) d[i]);
    }
    DEBUG_LOG("\nmy_jit_func2 end\n");
    return fp(ctx);
}