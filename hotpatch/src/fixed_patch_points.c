#include "include/fixed_patch_points.h"
#include "libebpf/include/ebpf.h"
#include "hotpatch/include/iotpatch.h"
#include "hotpatch/include/utils.h"

#include "hotpatch/include/profiling.h"
#include "hotpatch/include/fixed_patch_point_def.h"

#define __NAKE __attribute__((naked))

// #ifndef __GNUC__
// #define asm __asm__
// #endif

#ifndef FIXED_OP_PASS
  #define FIXED_OP_PASS 0x00010000 // set a unusual value
#endif

void dispatch_fixed_patch_point(uint32_t sp);

// fixed_patch_point_hanlder implement
#ifdef USE_KEIL
// stmdb	sp!, {r4, r5, r6, r7, r8, r9, sl, lr}
__asm void save_context(void) {
//	IMPORT leave_fixed_patch_point
	PUSH {r0-r3}

//	TST lr, #4
//	ITE EQ
//	MRSEQ r0, MSP
//	MRSNE r0, PSP
//	BL leave_fixed_patch_point
}

// ldmia.w	sp!, {r4, lr}
__asm void restore_context(void) {
	POP {r0-r3}
}

/*
CONTROL[1] = 0 -> MSP
CONTROL[1] = 1 -> PSP
*/
__asm int fixed_patch_point_hanlder(void) {
	IMPORT dispatch_fixed_patch_point
	PUSH {r0, lr}
	MRS r0, CONTROL
	TST r0, #2
	ITE EQ
	MRSEQ r0, MSP // r0 == 0
	MRSNE r0, PSP // r0 == 1
	MRS r0, MSP
	BL dispatch_fixed_patch_point
	POP {r0, pc}
}

#elif defined(SYS_CORTEX_M4)
// TODO: inline the fixed_patch_point_handler
__NAKE int fixed_patch_point_hanlder(void) {
	// __asm("nop");
	__asm volatile("PUSH {r0, lr}");
	__asm volatile("MRS r0, CONTROL");
	__asm volatile("TST r0, #2");
	__asm volatile("ITE EQ");
	__asm volatile("MRSEQ r0, MSP");
	__asm volatile("MRSNE r0, PSP");
//	asm volatile("ADD r0, #8"); // set to origin sp (push {r0, lr})
	__asm volatile("BL dispatch_fixed_patch_point");
	__asm volatile("POP {r0, pc}");
//	asm volatile("ADDS r7, #16");
//	asm volatile("MOV  sp, r7");
//	asm volatile("pop {r7, pc}");
}

// __NAKE int fixed_patch_point_hanlder(void) {
// 	// __asm("nop");
// 	__asm volatile("PUSH {r0, lr}");
// 	__asm volatile("MRS r0, CONTROL");
// 	__asm volatile("TST r0, #2");
// 	__asm volatile("ITE EQ");
// 	__asm volatile("MRSEQ r0, MSP");
// 	__asm volatile("MRSNE r0, PSP");
// 	// __asm volatile("STR r0, PSP");
// //	asm volatile("ADD r0, #8"); // set to origin sp (push {r0, lr})
// 	// __asm volatile("BL dispatch_fixed_patch_point");
// 	__asm volatile("POP {r0, pc}");
// //	asm volatile("ADDS r7, #16");
// //	asm volatile("MOV  sp, r7");
// //	asm volatile("pop {r7, pc}");
// }

// __NAKE int fixed_patch_point_hanlder(void) {
// 	__asm__ volatile(
// 		"PUSH {r0, r1, lr}\n"
// 		"LDR  r0, [pc, #20]\n"
// 		"LDR  r0, [r0, #0]\n"
// 		"LDR  r1, [r7, #8]\n"
// 		"AND  r0, r1\n"
// 		"CMP  r0, r1\n"
// 		"POP {r0, r1, pc}\n"
// 	);
// }

// uint32_t filter;
// int fixed_patch_point_hanlder(void) {
// 	int sp = 0;
// 	uint32_t addr = &sp;
// 	if ((addr & filter) != addr) {
// 		return FIXED_OP_PASS;
// 	}
// 	return sp;
// }

// __attribute__((used)) int fixed_patch_point_hanlder(void) {
// 	return FIXED_OP_PASS;
// }

#elif defined(ESP32S) // xtensa

__attribute__((used)) int fixed_patch_point_hanlder(void) {
	return FIXED_OP_PASS;
}

#elif defined(RISCV32) // riscv-32 

#else
int fixed_patch_point_hanlder(void) {
	return 0;
}
#endif


#ifdef EVA_TEST

static struct dummy_MQTT_buf_ctx {
	unsigned char *cur;
    unsigned char *end;
};
static void call_dummy_buggy_MQTT_function();
static int dummy_buggy_MQTT_packet_length_decode(struct dummy_MQTT_buf_ctx *buf, uint32_t *length);

// align 32
static struct func_args {
	int v1;
	char v2;
	int v3;
};

static int bug_function(int a, struct func_args *p1, struct func_args *p2, int b, int c, int d) {
	/* This is automatically added by the RapidPatch Tool.*/
	PATCH_FUNCTION_ERR_CODE;
	/*---End---*/

	int ERR = -1;
	DEBUG_LOG("enter bug_function\n");
	if (a + b + c + d == 10) {
		if (p1->v2 != p2->v2) {
			return ERR;
		}
		if (p1->v1 + p1->v3 == p2->v1 + p2->v3) {
			return 0;
		}
		return ERR;
	}
	return ERR;
}

static void bug2_function(int t) {
	PATCH_FUNCTION_VOID;
}

static void call_buggy_function() {
	// setup test arguments
	int a = 1, b = 2, c = 3, d = 4;
	DEBUG_LOG("addr ground-truth bug:0x%08x test:0x%08x ", bug_function, call_buggy_function);
	DEBUG_LOG("struct size: %d\n", sizeof(struct func_args));
	struct func_args p1 = {.v1 = 1, .v2 = 'a', .v3 = 3};
	//DEBUG_LOG("Test ADDR: d=0x%08x base=0x%08x v1=0x%08x v2=0x%08x v3=0x%08x\n", &d, &p1, &(p1.v1), &(p1.v2), &(p1.v3));
	struct func_args *p2 = ebpf_malloc(sizeof(struct func_args));
	DEBUG_LOG("p1:0x%08x p2:0x%08x\n", &p1, p2);

	p2->v1 = p2->v3 = 2;
	p2->v2 = 'b';

	profile_start(0);
	int ret = bug_function(a, &p1, p2, b, c, d);
	profile_end(0);

	DEBUG_LOG("Bug function return %d ", ret);
	if (ret == 0) {
		DEBUG_LOG("is fixed!\n");
	} else {
		DEBUG_LOG("is still vulnerable!\n");
	}
}

void test_fixed_patch_point() {
	// read_local_fixed_patch();
	profile_add_event("fixed patch");
	// call_buggy_function();
	call_dummy_buggy_MQTT_function();
	profile_dump(0);
}

#endif

typedef struct fixed_stack_frame {
	uint32_t r0_1;
	uint32_t lr; // patch index
	// uint32_t r3;
	// uint32_t r2;
	// uint32_t r1;
	// uint32_t r0;
	// uint32_t fp;
	// uint32_t return_address; // real lr
	// uint32_t args4;
	// uint32_t args5;
} fixed_stack_frame;

void dispatch_fixed_patch_point(uint32_t sp) {
#if 1
	uint32_t lr = *(uint32_t *) (sp + 4);
	uint32_t addr = (lr - 4) & (~0x3);
	ebpf_patch *patch = get_fixed_patch_by_lr(addr);
	// DEBUG_LOG("Patch instruction num %d\n", patch->vm->num_insts);
	uint64_t ret = 0;
	DEBUG_LOG("try to get patch at: 0x%08x\n", addr);
	fixed_stack_frame *args = (fixed_stack_frame *) sp;
	if (patch == NULL) {
		*(volatile uint32_t *) &(args->r0_1) = FIXED_OP_PASS;
		DEBUG_LOG("Do not find Patch here\n");
		return;
	} 
	ret = run_ebpf_filter(patch, args, sizeof(fixed_stack_frame));
	// DEBUG_LOG("ret:0x%08x\n", (uint32_t)ret);
	uint32_t op = ret >> 32;
	// DEBUG_LOG("op code:0x%08x \n", op);
	uint32_t ret_code = ret & 0x00000000ffffffff;
	// DEBUG_LOG("ret code:0x%08x \n", ret_code);
	//op = FILTER_DROP;
	if (op == FILTER_DROP) {
		*(volatile uint32_t *) &(args->r0_1) = 0; 
		return;
	} else if (op == FILTER_REDIRECT) {
		*(volatile uint32_t *) (args->lr) = ret_code;
		TEST_LOG("FILTER_REDIRECT\n");
		return;
	} else { // FILTER_PASS
		*(volatile uint32_t *) &(args->r0_1) = FIXED_OP_PASS;
		TEST_LOG("FILTER_PASS\n");
		return;
	}

#else // test patch stack
	//fixed_stack_frame *frame = (fixed_stack_frame*) r0;
	uint32_t lr = *(uint32_t *) (sp + 4);
	int r0 = *(int *)(sp);
	// r0, lr, 
	int addr = sp + 8;
	uint32_t patch_idx = lr - 4;
	uint32_t return_addr = *(uint32_t*)(addr + 16);
	DEBUG_LOG("sp:0x%08x lr:0x%08x return_addr:0x%08x\n", sp, patch_idx, return_addr);
	DEBUG_LOG("val: %d %c\n", r0, 'a');
	int v1 = *(int*)(addr + 12);
	struct func_args *p1, *p2;
	p1 = (struct func_args *) (*(uint32_t *) (addr + 8));
	p2 = (struct func_args *) (*(uint32_t *) (addr + 4));
	int v4 = *(int*)(addr);
	int v5 = *(int*)(addr + 24);
	int v6 = *(int*)(addr + 28);
	DEBUG_LOG("0x%08x a=%d p1=0x%08x p2=0x%08x b=%d c=%d d=%d\n", addr, v1, p1, p2, v4, v5, v6);
	DEBUG_LOG("p1=%d p1-c=%c %d p2=%d p2-c=%c %d\n", p1->v1, p1->v2, p1->v3, p2->v1, p2->v2, p2->v3);
#endif
}

static int dummy_buggy_MQTT_packet_length_decode(struct dummy_MQTT_buf_ctx *buf, uint32_t *length)
{
	PATCH_FUNCTION_ERR_CODE;

	uint8_t shift = 0U;
	uint8_t bytes = 0U;

	int ERR = -1;

	*length = 0U;
	do {
		if (bytes > 4) {
			return ERR;
		}

		if (buf->cur >= buf->end) {
			return ERR;
		}

		*length += ((uint32_t)*(buf->cur) & 0x7f)
								<< shift;
		shift += 7;
		bytes++;
	} while ((*(buf->cur++) & 0x80) != 0U);

	DEBUG_LOG("dummy MQTT packet length:0x%08x \n", *length);

	return 0;
}

static void call_dummy_buggy_MQTT_function() {
	// setup test arguments
	DEBUG_LOG("addr ground-truth bug:0x%08x test:0x%08x \n", dummy_buggy_MQTT_packet_length_decode, call_dummy_buggy_MQTT_function);
	
	uint8_t packet_buf[10];
	
	for (int i=0; i<4; ++i) packet_buf[i] = 0xff;
	packet_buf[4] = 0x7f;
	for (int i=5; i<10; ++i) packet_buf[i] = 0;

	struct dummy_MQTT_buf_ctx dbc;
	dbc.cur = &packet_buf[0];
	dbc.end = &packet_buf[9];	

	uint32_t pkt_length = 0;
	
	profile_start(0);
	int ret = dummy_buggy_MQTT_packet_length_decode(&dbc, &pkt_length);
	profile_end(0);

	DEBUG_LOG("Decoded MQTT packet length is %d\n", pkt_length);

	DEBUG_LOG("Bug function return %d ", ret);
	if (ret != 0) {
		DEBUG_LOG("is still vulnerable!\n\n");
	} else {
		DEBUG_LOG("is fixed!\n");
	}
}
