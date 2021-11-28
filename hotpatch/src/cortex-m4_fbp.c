#ifdef SYS_CORTEX_M4
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "include/patch_point.h"
#include "include/utils.h"
#include "include/iotpatch.h"

#include "hotpatch/include/profiling.h"

#define IO volatile 

#define MAX_BP 6 // six inst breakpoint in cortex-m4
#define FPB_CB_REG 5

int hera_fix_func();

//static int bpkt_cb_idx;

#ifdef USE_KEIL
__asm void HERA_Dipatcher(void) 
{
	IMPORT hera_fix_func
	PUSH {r0, r1, r2, r3, r7, lr}
	LDR.w r3, =0x2000050   
	CMP lr, r3
	BEQ end
	LDR.w r3, =0x2000052   
	CMP lr, r3
	BEQ.N end
	LDR.w r3, =0x2000054 
	CMP lr, r3
	BEQ.N end
	LDR.w r3, =0x2000055  
	CMP lr, r3
	BEQ.N end
	LDR.w r3, =0x2000057  
	CMP lr, r3
	BEQ.N end
end
	BL hera_fix_func
	POP {r0, r1, r2, r3,  r7, pc}
}


// must put before usages
/*
TODO: 1. ��֤�Ƿ��������޸���r5���²��ȶ�
2. ��ȷ��ʱ
*/
__asm void FPB_Handler0(void)
{
	NOP
	NOP
	NOP
	NOP
	IMPORT leave_fpb_bpkt
//	B leave_fpb_bpkt
	PUSH {r0-r3, r12, lr}
	MRS r0, CONTROL
	TST r0, #2
	ITE EQ
	MRSEQ r0, MSP // r0 == 0
	MRSNE r0, PSP // r0 == 1
	MOVS r1, #0
	BL leave_fpb_bpkt
	// todo: return in other address
	MOV r5, r0
	POP {r0-r3, r12, lr}
//	MOV r0, r5
//	BX LR
	MOV PC, r5
}

__asm void FPB_Handler1(void)
{
	NOP
	NOP
	NOP
	NOP
	IMPORT leave_fpb_bpkt
//	B leave_fpb_bpkt
	PUSH {r0-r3, r12, lr}
	MRS r0, CONTROL
	TST r0, #2
	ITE EQ
	MRSEQ r0, MSP 
	MRSNE r0, PSP 
	MOVS r1, #1
	BL leave_fpb_bpkt
	// todo: return in other address
	MOV r5, r0
	POP {r0-r3, r12, lr}
	MOV r0, r5
//	BX LR
}

// r1 = 2
__asm void FPB_Handler2(void)
{
	NOP
	NOP
	NOP
	NOP
	IMPORT leave_fpb_bpkt
//	B leave_fpb_bpkt
	PUSH {r0-r3, r12, lr}
	TST lr, #4
	ITE EQ
	MRSEQ r0, MSP
	MRSNE r0, PSP
	MOVS r1, #2
	BL leave_fpb_bpkt
	// todo: return in other address
//	MOV r5, r0
	POP {r0-r3, r12, lr}
//	MOV r0, r5
	BX LR
}

// r1 = 3
__asm void FPB_Handler3(void)
{
	NOP
	NOP
	NOP
	NOP
	IMPORT leave_fpb_bpkt
//	B leave_fpb_bpkt
	PUSH {r0-r3, r12, lr}
	TST lr, #4
	ITE EQ
	MRSEQ r0, MSP
	MRSNE r0, PSP
	MOVS r1, #3
	BL leave_fpb_bpkt
	// todo: return in other address
//	MOV r5, r0
	POP {r0-r3, r12, lr}
//	MOV r0, r5
	BX LR
}

// r1 = 4
__asm void FPB_Handler4(void)
{
	NOP
	NOP
	NOP
	NOP
	IMPORT leave_fpb_bpkt
//	B leave_fpb_bpkt
	PUSH {r0-r3, r12, lr}
	TST lr, #4
	ITE EQ
	MRSEQ r0, MSP
	MRSNE r0, PSP
	MOVS r1, #4
	BL leave_fpb_bpkt
	// todo: return in other address
//	MOV r5, r0
	POP {r0-r3, r12, lr}
//	MOV r0, r5
	BX LR
}

// r1 = 5
/*
Use handler 5 to reset FPB
*/
__asm void FPB_Handler5(void)
{
	NOP
	NOP
	NOP
	NOP
	PUSH {r0-r3, r12, lr}
	IMPORT fpb_enable_callback
	BL fpb_enable_callback
	MOV r5, r0
//	MOV.W lr, r0
//	POP {r0, lr}
//	BX LR
	POP {r0-r3, r12, lr}
	MOV PC, r5
}

__asm void set_return(int ret_code, uint32_t addr);
#else
//		__asm__ __volatile__("nop"); \
//		__asm__ __volatile__("nop"); \
//		__asm__ __volatile__("nop"); \
//		__asm__ __volatile__("nop"); \

#define __NAKE __attribute__((naked))
#define DECLARE_FPB_HANDLE(bpkt_idx)  \
	void FPB_Handler##bpkt_idx(void) {\
		__asm__ __volatile__("PUSH {r0-r3, r12, lr}"); \
		__asm__ __volatile__("TST lr, #4"); \
		__asm__ __volatile__("ITE EQ"); \
		__asm__ __volatile__("MRSEQ r0, MSP"); \
		__asm__ __volatile__("MRSNE r0, PSP"); \
		__asm__ __volatile__("MOVS r1, #" #bpkt_idx ""); \
		__asm__ __volatile__("BL leave_fpb_bpkt"); \
		__asm__ __volatile__("POP {r0-r3, r12, lr}"); \
	} 
//		__asm__ __volatile__("BX LR"); \


//DECLARE_FPB_HANDLE(0);
void FPB_Handler0(void)
{
	__asm__("NOP");
	__asm__("NOP");
	__asm__("NOP");
	__asm__("NOP");
	__asm__("PUSH {r0-r3, r12, lr}");
	//DEBUG_LOG("FPB_Handler0\n");
//	fpb_trampoline_0();
//	IMPORT leave_fpb_bpkt
//	__asm__("PUSH {r3, lr}");
	__asm__("TST lr, #4");
	__asm__("ITE EQ");
	__asm__("MRSEQ r0, MSP");
	__asm__("MRSNE r0, PSP");
	__asm__("MOVS r1, #0");
	__asm__("BL leave_fpb_bpkt");
	// todo: return in other address
	__asm__("MOV r5, r0");
	__asm__("POP {r0-r3, r12, lr}");
//	DEBUG_LOG("exit FPB_Handler0\n");
	__asm__("MOV PC, r5");
//	__asm__("BX LR");
//	__asm__("pop	{r3, pc}");
//	return 0;
}

DECLARE_FPB_HANDLE(1);
DECLARE_FPB_HANDLE(2);
DECLARE_FPB_HANDLE(3);
DECLARE_FPB_HANDLE(4);
//DECLARE_FPB_HANDLE(5);

/*
Use handler 5 to reset FPB
*/
void FPB_Handler5(void)
{
//	__asm__("NOP");
//	__asm__("NOP");
//	__asm__("NOP");
//	__asm__("NOP");
	__asm__("PUSH {r0-r3, r12, lr}");
//	IMPORT fpb_enable_callback
	__asm__("BL fpb_enable_callback");
	__asm__("MOV r5, r0");
//	MOV.W lr, r0
//	POP {r0, lr}
//	BX LR
	__asm__("POP {r0-r3, r12, lr}");
	__asm__("MOV PC, r5");
}

#endif // end USE_KEIL

//struct stack_context;
//void leave_fpb_bpkt(struct stack_context *ctx, int bpkt_idx);
uint32_t calc_branch_instr(uint32_t instr_addr, uint32_t target_addr);

typedef struct {
	IO uint32_t CTRL;
	IO uint32_t REMAP;
	IO uint32_t COMP[MAX_BP];
} FPB_UNIT;

// address must be 
typedef struct {
	uint32_t inst[10];
	bool active;
} FPB_REMAP;

static FPB_UNIT *const FPB = (FPB_UNIT *) 0xE0002000;
static FPB_REMAP REMAP_ADDR;

struct breakpoints_info
{
	uint8_t cur_idx;
	uint8_t bpkt_num;
} bpkt_info;

static bool bpkt_is_init = false;

// enable hardware breakpoint
static void fpb_enable(void) {
	FPB->CTRL |= 0x3;
}

static void fpb_disable(void) {
	FPB->CTRL = (FPB->CTRL & ~0x3) | 0x2;
}

// address should be 32bit align
static inline uint32_t get_fpb_patch_remap() {
	return ((uint32_t)(&REMAP_ADDR) + 0x1f) & (~0x1f);
}

static void fpb_set_comp_reg(uint8_t idx, uint32_t comp_addr)
{
	FPB->COMP[idx] = (comp_addr | 0x01UL);
}

// covert the inst addr to valid pc address
uint32_t calc_bpkt_addr(uint32_t inst_addr) {
	uint32_t replace = (inst_addr & 0x2) == 0 ? 1 : 2;
	uint32_t fp_comp = (inst_addr & ~0x3) | 0x1 | (replace << 30);
	// const int enabled = fp_comp & 0x1;
	replace = fp_comp >> 30;

	uint32_t final_addr = fp_comp & 0x1FFFFFFC;
	if (replace == 0x2) {
		final_addr |= 0x2;
	}
	return final_addr;
}

int get_hw_bkpt_num() {
	return 6;
}

void show_hw_bkpt(void) {
	// uint32_t fp_ctrl = FPB->CTRL;
	// uint32_t num_code_comparators = (((fp_ctrl >> 12) & 0x7) << 4) | ((fp_ctrl >> 4) & 0xF);
	DEBUG_LOG("Dump current hardware breakpoints, MODE-%s:\n", REMAP_ADDR.active ? "FPB" : "debug monitor");
	for (int i = 0; i < MAX_BP; i++) {
		const uint32_t fp_comp = FPB->COMP[i];
		const int enabled = fp_comp & 0x1;
		const uint32_t replace = fp_comp >> 30;

		uint32_t inst_addr = fp_comp & 0x1FFFFFFC;
		if (replace == 0x2) {
		  inst_addr |= 0x2;
		}

		DEBUG_LOG("  FP_COMP[%d] Enabled %d, Replace: %d, Address 0x%x\n",(int)i, (int)enabled, (int)replace, inst_addr);
	}
}


///* Flash patch and breakpoint Mode
//*/
static inline void handle_fpb_remap(int idx) {
	TEST_LOG("handle_fpb_remap: %d\n", idx);
	// search patch
	uint32_t inst_addr = FPB->COMP[idx];
	ebpf_patch *patch = get_dynamic_patch_by_fpb(inst_addr);
	if (patch != NULL) {
		
	}
}

#ifdef USE_KEIL


#else

#endif
void fpb_trampoline_0() {
	handle_fpb_remap(0);
}

//static void fpb_trampoline_1() {
//	handle_fpb_remap(1);
//}

//static void fpb_trampoline_2() {
//	handle_fpb_remap(2);
//}

//static void fpb_trampoline_3() {
//	handle_fpb_remap(3);
//}

//static void fpb_trampoline_4() {
//	handle_fpb_remap(4);
//}

//static void fpb_trampoline_5() {
//	handle_fpb_remap(5);
//}



uint32_t calc_branch_instr(uint32_t instr_addr, uint32_t target_addr) { // A7-205
	uint32_t offset = (target_addr & (~0x1)) - (instr_addr & (~0x1)) - 4;
	uint16_t offset_10_upper = (offset >> 12) & 0x03FF;
	uint16_t offset_11_lower = ((offset) >> 1)  & 0x07FF; // UNCERTAIN about this!
	
	uint8_t s_pos = 24;
	uint8_t s  = (offset >> s_pos) & 0x01u;
	uint8_t i1 = (offset >> (s_pos - 1)) & 0x01u;
	uint8_t i2 = (offset >> (s_pos - 2)) & 0x01u;
	
	uint8_t j1 = 0x01 & ((~i1) ^ s);
	uint8_t j2 = 0x01 & ((~i2) ^ s);
	
	uint16_t upper_bl_instr =  ((0x1E << 11) | (s << 10) | offset_10_upper);
	uint16_t lower_bl_instr =  ((0x02 << 14) | (j1 << 13) | (0x01 << 12) | (j2 << 11) | offset_11_lower);
	
	return ((upper_bl_instr << 16) | lower_bl_instr);
	//
	//uint16_t jp = 0xf7fd;
	//return (jp << 16) | (target_addr &(~0x1));
}

uint32_t my_gen_bw(uint32_t from_inst, uint32_t target_addr) {
	// https://stackoverflow.com/questions/33720488/how-to-generate-the-machine-code-of-thumb-instructions
	// a read returns the current instruction address plus 4 , cortex-m4 P-80
	int offset = (target_addr & (~0x1)) - (from_inst & (~0x1)) - 4;
	// A7.7.12 T4 
	uint32_t imm10 = (offset >> 12) & 0x03ff;
	uint32_t imm11 = (offset >> 1) & 0x07ff; // 11
	uint8_t s = offset < 0;
	uint8_t i1 = (offset >> 23) & 0x01; // i1 = ~(j1 ^ s)
	uint8_t i2 = (offset >> 22) & 0x01;
	uint8_t j1 = (~i1 ^ s) & 0x1;
	uint8_t j2 = (~i2 ^ s) & 0x1;
	uint32_t inst = (0xf0009000) | (s << 26) | (imm10 << 16) | (j1 << 13) | (j2 << 11) | (imm11);
	return little_endian_16_bit(inst);
}


uint32_t calc_branch_w_link_instr(uint32_t inst_addr, uint32_t target_addr) {
	uint32_t branch_instr = calc_branch_instr(inst_addr, target_addr);
	return (branch_instr | 0x00004000); // Set bit 14. This is the only difference between B and BL instructions.
}

static int add_fpb_bkpt(uint32_t inst_addr) {
	int idx = bpkt_info.cur_idx;
	uint32_t trampolines[] = { (uint32_t) FPB_Handler0, (uint32_t) FPB_Handler1, 
		(uint32_t) FPB_Handler2, (uint32_t) FPB_Handler3, (uint32_t) FPB_Handler4, (uint32_t) FPB_Handler5};
	// align jump address
	uint32_t trampoline_addr = ((trampolines[idx] & (~0x3)) + 3) & (~0x3);
	//uint32_t trampoline_addr = (trampolines[idx] & (~0x1));
	//uint32_t trampoline_addr = (((uint32_t) FPB_Handler0 & (~0x3)) + 3) & (~0x3);
	//trampoline_addr = (uint32_t) fpb_trampoline_0 & (~0x1);
	//trampoline_addr = (trampolines[0] & (0x1))
	uint32_t jump_inst = little_endian_16_bit(calc_branch_instr(inst_addr, trampoline_addr));
	//jump_inst = my_gen_bw(inst_addr, trampoline_addr);
	const uint32_t PATCH_TBL = get_fpb_patch_remap();
	
	//uint32_t my_jump = my_gen_bw(inst_addr, trampoline_addr);
	//jump_inst = my_jump;
	inst_addr = inst_addr & (~0x1);
	
	DEBUG_LOG("inst is half word: 0x%08x %d trampoline_addr: 0x%08x\n", inst_addr, inst_addr % 4, trampoline_addr);
	DEBUG_LOG("[0x%08x] fpb jump to: 0x%08x inst: 0x%08x jump_inst: 0x%08x\n", &(PATCH_TBL), trampoline_addr, inst_addr, jump_inst);
	if (inst_addr % 4 == 0) // Instruction is word aligned.
	{
		// use exist FP->COMP
		uint32_t fp_comp = inst_addr | 0x1;
		for (int i = 0; i < bpkt_info.bpkt_num; i++) {
			if (FPB->COMP[i] == fp_comp || FPB->COMP[i] == inst_addr) {
				FPB->COMP[i] = fp_comp;
				return i;
			}
		}
		
		// add new fpb remap
		//fpb_comparator_reg_config(reg_index, instr_addr);
		*((uint32_t *) (PATCH_TBL + (idx * 4))) = jump_inst;
		
		FPB->COMP[bpkt_info.cur_idx] = fp_comp;
		bpkt_info.cur_idx++;
		//set_fbp_jump();
	} 
	else // Instruction is half-word aligned.
	{
		// use exist FP->COMP
		uint32_t fp_comp1 = inst_addr & 0xFFFFFFFC;
		uint32_t fp_comp2 = (inst_addr & 0xFFFFFFFC) + 4;
		
		for (int i = 0; i < bpkt_info.bpkt_num - 1; i++) {
			if (FPB->COMP[i] == fp_comp1 && FPB->COMP[i + 1] == fp_comp2) {
				return i;
			}
		}
		
		// add new fpb remap
		uint32_t old_instr[2];
		old_instr[0] = *((uint32_t *)  (inst_addr & 0xFFFFFFFC));
		old_instr[1] = *((uint32_t *) ((inst_addr & 0xFFFFFFFC) + 4));
		
		*((uint32_t *) (PATCH_TBL + idx * 4))       = ((jump_inst & 0x0000FFFF) << 16) | (old_instr[0] & 0x0000FFFF);
		*((uint32_t *) (PATCH_TBL + (idx + 1) * 4)) = (old_instr[1] & 0xFFFF0000) | ((jump_inst & 0xFFFF0000) >> 16);
		
		FPB->COMP[bpkt_info.cur_idx] = fp_comp1;
		FPB->COMP[bpkt_info.cur_idx + 1] = fp_comp2;
		bpkt_info.cur_idx += 2;
	}
	
	
	// fpb bpkt
	
	return bpkt_info.cur_idx;
}

static int add_normal_bkpt(uint32_t inst_addr) {
	uint32_t replace = (inst_addr & 0x2) == 0 ? 1 : 2;
	uint32_t fp_comp = (inst_addr & ~0x3) | 0x1 | (replace << 30);
	
	// add hardware beakpoint
	for (int i = 0; i < bpkt_info.bpkt_num; i++) {
		if (FPB->COMP[i] == fp_comp) {
			return i;
		}
	}
	
	FPB->COMP[bpkt_info.cur_idx] = fp_comp;

	//fpb_enable();
	return bpkt_info.cur_idx++;
}

int add_hw_bkpt(uint32_t inst_addr) {
	if (!bpkt_is_init) {
		bpkt_info.cur_idx = 0;
		uint32_t fp_ctrl = FPB->CTRL;
		//uint32_t num_code_comparators = (((fp_ctrl >> 12) & 0x7) << 4) | ((fp_ctrl >> 4) & 0xF);
		//bpkt_info.bpkt_num = num_code_comparators;
		// there are six inst breakpoints in cortex-m4
		bpkt_info.bpkt_num = get_hw_bkpt_num();
		bpkt_is_init = true;
	}

	if (inst_addr >= 0x20000000) {
		DEBUG_LOG("Cannot add breakpoint in RAM.\n");
		return -1;
	}

	if (bpkt_info.cur_idx >= MAX_BP) {
		bpkt_info.cur_idx = 0;
		//DEBUG_LOG("Exceed the max breakpoints number! Reset current bpkt idx to 0!\n");
		// return false;
	}
	
	// set fpb remap
	if (REMAP_ADDR.active) {
		return add_fpb_bkpt(inst_addr);
	} else {
		return add_normal_bkpt(inst_addr);
	}
}

void clear_all_hw_bkpt() {
	uint32_t fp_ctrl = FPB->CTRL;
	uint32_t num_code_comparators = (((fp_ctrl >> 12) & 0x7) << 4) | ((fp_ctrl >> 4) & 0xF);
	for (int i = 0; i < num_code_comparators; i++) {
		FPB->COMP[i] = 0;
	}
	bpkt_info.cur_idx = 0;
	fpb_disable();
}


/* Debug Monitor Mode
*/

//
// Debug Halting Control and Status Register 
// DHCSR (volatile uint32_t *) (0xE000EDF0)
// Debug Core Register Selector Register
// DCRSR (volatile uint32_t *) (0xE000EDF4) 
// Debug Core Register Data Register 
// DCRDR (volatile uint32_t *) (0xE000EDF8)
// Debug Exception and Monitor Control Register
// DEMCR (volatile uint32_t *) (0xE000EDFC)
typedef struct {
	IO uint32_t DHCSR;
	IO uint32_t DCRSR;
	IO uint32_t DCRDR;
	IO uint32_t DEMCR;
} DEBUG_CONTROL_BLOCK;

// static DEBUG_CONTROL_BLOCK *const DEG_MON = (DEBUG_CONTROL_BLOCK *) 0xE000EDF0;

#define DCB_DEMCR 0xE000EDFC // page 464
#define DCB_DEMCR_MON_EN (uint32_t) (1 << 16)  
#define SCB_HFSR 0xE000E02C 
#define SCB_DFSR 0xE000ED30 // page 390
#define SCB_SHP8 0xE000ED20 // page 263, debug monitor priority


typedef struct __attribute__((packed)) stack_context {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12; // ip
	uint32_t lr;
	uint32_t pc; // return address
	uint32_t xpsr;
} stack_context;

typedef struct __attribute__((packed)) ebpf_args_frame {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
} ebpf_args_frame;


static bool halting_debug_enabled(void) {
	volatile uint32_t *dhcsr = (uint32_t *)0xE000EDF0;
	return (((*dhcsr) & 0x1) != 0);
}

// init fpb mode
/* In ARM cortex-m3/m4 
When both breakpoint (comparator register) and remap reg are set, it will enable the 
Flash patch and breakpoint unit and replace the instrument at the comp addr with the 
instrument at the remap addr.
*/
static void fpb_remap_init(uint32_t remap_addr)
{
	FPB->REMAP = remap_addr;
	REMAP_ADDR.active = true;
	fpb_enable();
}


// init arm debug monitor
static void debug_monitor_init() {
	volatile uint32_t *scb_hfsr = (uint32_t *) SCB_HFSR;
	volatile uint32_t *scb_dfsr = (uint32_t *) SCB_DFSR;
	*scb_hfsr = (1 << 31);
//	*SCB_DFSR = 1;
//	*SCB_DFSR = (1 << 1);
	*scb_dfsr = (1 << 2);
	
	if (halting_debug_enabled()) {
		DEBUG_LOG("Halting Debug Enabled - Can't Enable Monitor Mode Debug!");
		return;
	}
	
	volatile uint32_t *demcr = (uint32_t *) DCB_DEMCR;
	*demcr |= DCB_DEMCR_MON_EN; 
	//*demcr &= ~(1 << mon_en_bit); // DEG_MO = 0 && C_DEBUG = 1 -> HardFault
	fpb_enable();
	DEBUG_LOG("debug_monitor_init setup dhcsr: %u\n", *demcr);
	
	// Priority for DebugMonitor Exception is bits[7:0].
	// We will use the lowest priority so other ISRs can
	// fire while in the DebugMonitor Interrupt
	// Configurable priority values are in the range 0-255. This means that the Reset (), HardFault (-1), and NMI exceptions (-2), 
	// with fixed negative priority values, always have higher priority than any other exception.
	volatile uint32_t *shp8 = (uint32_t *)0xE000ED20;
	//DEBUG_LOG("SCB_SHP[8] priority: %d\n", (int) ((char)*shp8));
	*shp8 = 0xff;
	*shp8 = -1;
}

static inline void dump_context(stack_context *ctx) {
	TEST_LOG("Register Dump:\n");
	TEST_LOG(" r0  =0x%08x\n", ctx->r0);
	TEST_LOG(" r1  =0x%08x\n", ctx->r1);
	TEST_LOG(" r2  =0x%08x\n", ctx->r2);
	TEST_LOG(" r3  =0x%08x\n", ctx->r3);
	TEST_LOG(" r12 =0x%08x\n", ctx->r12);
	TEST_LOG(" lr  =0x%08x\n", ctx->lr);
	TEST_LOG(" pc  =0x%08x\n", ctx->pc);
	TEST_LOG(" xpsr=0x%08x\n", ctx->xpsr);
}


/*
Disable current BPKT to continue running.
Add bpkt to next inst and enable current BPKT.
*/
void fpb_disable_cur_inst(int bpkt_idx) {
	uint32_t cur_addr = FPB->COMP[bpkt_idx];
	uint32_t next_inst = calc_bpkt_pc(cur_addr);
	//next_inst = calc_bpkt_pc(next_inst);
	while ((next_inst & 0x3) != 0) {
		next_inst = calc_bpkt_pc(next_inst);
	}
	FPB->COMP[FPB_CB_REG] = next_inst | (0x1);
	FPB->COMP[bpkt_idx] &= ~(0x1);
	uint32_t trampoline_addr = (((uint32_t) FPB_Handler5 & (~0x1)) + 3) & (~0x3);
	uint32_t jump_inst = calc_branch_instr(next_inst, trampoline_addr);
	// add trampoline to BPKT#5
	const uint32_t PATCH_TBL = get_fpb_patch_remap();
	*((uint32_t *) (PATCH_TBL + (FPB_CB_REG * 4))) = little_endian_16_bit(jump_inst);
	TEST_LOG("fpb_disable_cur_inst: 0x%08x next: 0x%08x bkpt: 0x%08x fpb_cb: 0x%08x\n", cur_addr, next_inst, FPB->COMP[bpkt_idx], FPB->COMP[FPB_CB_REG]);
	//bpkt_cb_idx = bpkt_idx;
	//add_mulprf_step("fpb_disable_cur_inst");
}

// enable the FPB after the second inst is triggered
uint32_t fpb_enable_callback() {
	uint32_t cur_inst = FPB->COMP[FPB_CB_REG];
	//*(volatile uint32_t *) &(ctx->lr) = cur_inst;
	TEST_LOG("fpb_enable_callback: 0x%08x\n", cur_inst);
	//add_mulprf_step("fpb_enable_callback");
	// enable all bpkt
	for (int i = 0; i < FPB_CB_REG; i++) {
		if (FPB->COMP[i] != 0) {
			FPB->COMP[i] |= 1;
		}
	}
	//FPB->COMP[bpkt_cb_idx] |= 0x1;
	FPB->COMP[FPB_CB_REG] &= ~(0x1);
	return cur_inst;
}

/*
https://www.keil.com/support/man/docs/armclang_intro/armclang_intro_ddx1471430827125.htm
*/
// uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3
//extern void BusFault_Handler(void);
uint32_t leave_fpb_bpkt(stack_context *ctx, int bpkt_idx) {
	//DEBUG_LOG("argmeunts: r0=0x%08x r1=0x%08x r2=0x%08x r3=0x%08x\n", r0, r1, r2, r3);
	//dump_context(ctx);
	TEST_LOG("enter leave_fpb_bpkt bpkt_idx: %d\n", bpkt_idx);
	//bpkt_idx = 0;
	uint32_t inst_addr = FPB->COMP[bpkt_idx] & (~0x1);
	ebpf_patch *patch = get_dynamic_patch_by_bpkt(inst_addr);
	if (patch != NULL) {
		ebpf_args_frame *args = (ebpf_args_frame *) ctx;
		uint64_t ret = 0;
		profile_start(EV1);
		ret = run_ebpf_filter(patch, args, sizeof(ebpf_args_frame));
		profile_end(EV1);
		//profile_dump(EV1);
		uint32_t op = ret >> 32;
		uint32_t ret_code = ret & 0x00000000ffffffff;
		TEST_LOG("run_ebpf_filtern res: %u %u\n", op, ret_code);
		//op = FILTER_DROP;
		if (op == FILTER_PASS) {
			TEST_LOG("FILTER PASS addr:0x%08x inst:0x%08x lr:0x%08x\n", FPB->COMP[bpkt_idx], inst_addr, ctx->lr);
			//*(volatile uint32_t *) &(ctx->r0) = 0;
			fpb_disable_cur_inst(bpkt_idx);
			TEST_LOG("set_return: 0x%08x\n", ctx->lr);
			return FPB->COMP[bpkt_idx];
		} else if (op == FILTER_DROP) {
			TEST_LOG("filter and return: %u 0x%08x 0x%08x\n", ret_code, ctx->lr, ctx->pc);
			*(volatile uint32_t *) &(ctx->r0) = ret_code;
			TEST_LOG("set_return: 0x%08x\n", ctx->lr);
		} else if (op == FILTER_REDIRECT) { // redirect lr to other address
			//*(volatile uint32_t *) &(ctx->lr) = ret_code;
			return ret_code;
		} else {
			TEST_LOG("patch do not intercept the origin function: op_code: %d pc: 0x%08x \n", op, ctx->pc);
		}
	}
	TEST_LOG("leave_fpb_bpkt: 0x%08x\n", inst_addr);
	//return 0;
	return ctx->lr;
}


static inline bool run_patch_here(stack_context *ctx) {
	ebpf_patch *patch = get_dynamic_patch_by_bpkt(ctx->pc);
	
	if (patch != NULL) {
		ebpf_args_frame *args = (ebpf_args_frame *) ctx;
		uint64_t ret;
		profile_start(EV1);
		ret = run_ebpf_filter(patch, args, sizeof(ebpf_args_frame));
		profile_end(EV1);
		uint32_t op = ret >> 32;
		uint32_t ret_code = ret & 0x00000000ffffffff;
		TEST_LOG("run_ebpf_filter res: %u %u\n", op, ret_code);
		//op = FILTER_DROP;
		if (op == FILTER_DROP) {
			TEST_LOG("filter and return: %u 0x%08x 0x%08x\n", ret_code, ctx->lr, ctx->pc);
			*(volatile uint32_t *) &(ctx->r0) = ret_code;
			*(volatile uint32_t *) &(ctx->pc) = ctx->lr; // calc_bpkt_addr(ctx->lr); //
			TEST_LOG("set_return: 0x%08x\n", ctx->pc);
			//set_return(ret_code, ctx->lr);
			//__asm ("BX lr");
			return true;
		} else if (op == FILTER_REDIRECT) { // redirect pc to other address
			//*(volatile uint32_t *) &(ctx->lr) = ret_code;
			*(volatile uint32_t *) &(ctx->pc) = ret_code; 
			TEST_LOG("set_return: 0x%08x\n", ctx->pc);
			return true;
		} else { // FILTER_PASS
			TEST_LOG("patch do not intercept the origin function: op_code: %d pc: 0x%08x \n", op, ctx->pc);
		}
		return false;
	} else {
		TEST_LOG("do not find patch for pc addr: 0x%08x \n", ctx->pc);
	}
	return false;
}

void leave_debug_monitor(stack_context *ctx) {
	TEST_LOG("debug_monitor_handler_c lr:0x%08x pc:0x%08x\n", ctx->lr, ctx->pc);
	volatile uint32_t *demcr = (uint32_t *) DCB_DEMCR;
	volatile uint32_t *dfsr  = (uint32_t *) SCB_DFSR;
	const uint32_t demcr_single_step_mask = (1 << 18);
	const uint32_t dfsr_dwt_evt_bitmask = (1 << 2);
	const uint32_t dfsr_bkpt_evt_bitmask = (1 << 1);
	const uint32_t dfsr_halt_evt_bitmask = (1 << 0);
	const bool is_dwt_dbg_evt = (*dfsr & dfsr_dwt_evt_bitmask);
	const bool is_bkpt_dbg_evt = (*dfsr & dfsr_bkpt_evt_bitmask);
	const bool is_halt_dbg_evt = (*dfsr & dfsr_halt_evt_bitmask);

	
	if (is_bkpt_dbg_evt) {
		// only enter bkpt once

		bool skip = run_patch_here(ctx);
		//bool skip = false;
		/*
			when the bug is triggered and return ealier with ebpf return code
			do not need to enter debug monitor again
		*/
		if (skip) { 
			//__asm__ __volatile__("cpsie i");
			return;
		}

		// enter halt mode to enable breakpoints
		*demcr |= demcr_single_step_mask;
		//*demcr &= ~(demcr_single_step_mask);
		*dfsr = dfsr_bkpt_evt_bitmask;
		// TODO: disable/enable single breakpoint
		fpb_disable(); // will exit
		//TEST_LOG("is_bkpt_dbg_evt\n");
	} else if (is_halt_dbg_evt) {
		*demcr &= ~(demcr_single_step_mask);
		*dfsr = dfsr_halt_evt_bitmask;
		fpb_enable();
		//DEBUG_LOG("bpkt inst addr: 0x%08x\n", ctx->pc - 4);
		// enter twice debug monitor
		//TEST_LOG("is_halt_dbg_evt\n");
		//__asm__ __volatile__("cpsie i");
	} 
}

#ifdef USE_KEIL
// put in the first line 
//__asm void FPB_Handler(void) {
//	IMPORT leave_fpb_bpkt
////	B leave_fpb_bpkt
//	PUSH {r0-r4, lr}
//	TST lr, #4
//	ITE EQ
//	MRSEQ r0, MSP
//	MRSNE r0, PSP
//	BL leave_fpb_bpkt
//	MOV r5, r0
//	POP {r0-r4, lr}
//	MOV r0, r5
//	BX LR
//}

//void function_gap() {
//	DEBUG_LOG("This is a function_gap\n");
//}

__asm void DebugMon_Handler(void) {
	IMPORT leave_debug_monitor
	TST lr, #4
	ITE EQ
	MRSEQ r0, MSP
	MRSNE r0, PSP
	B leave_debug_monitor
}
// #elif defined(ZEPHYR_OS)
#else
__NAKE void __debug_monitor(void) {
	/* select interru ed stack */
	//__asm__ __volatile__("cpsid i");
	__asm__ __volatile__("and r0, lr, #4");
	__asm__ __volatile__("cmp r0, #0");
	__asm__ __volatile__("ite eq");
	__asm__ __volatile__("mrseq r0, msp");
	__asm__ __volatile__("mrsne r0, psp");
	__asm__ __volatile__("b leave_debug_monitor");
	//__asm__ __volatile__("cpsie i");
}

//void __debug_monitor(void) {
//	DEBUG_LOG("__debug_monitor\n");
//}
#endif // end USE_KEIL

void set_patch_mode(patch_mode_t mode) {
	// reset the fpb bpkt
	clear_all_hw_bkpt();
	memset(&REMAP_ADDR, 0, sizeof(REMAP_ADDR));
	REMAP_ADDR.active = false;
	FPB->REMAP = 0;
	if (mode == CORTEX_FPB_PATCH) { // use fpb
//		FPB->REMAP = (uint32_t *) &REMAP;
		//const uint32_t PATCH_TBL = (uint32_t) (&REMAP_ADDR) & (~0x1F);
		uint32_t addr = get_fpb_patch_remap();
		fpb_remap_init(addr);
		//DEBUG_LOG("Remap addr: 0x%08x 0x%08x 0x%08x\n", addr, &REMAP_ADDR, &(REMAP_ADDR.active));
	} else if (mode == CORTEX_DEB_MON_PATCH) {
		debug_monitor_init();
	}
}

/*
FPB test functions
*/

static int buggy_func(int v) {
	if (v > 2000) { // fixed: v > 5000 return -1
		return -1;
	}
	return 0;
}

static void test_func() {
	DEBUG_LOG("run test_func: 0x%08x\n", (uint32_t) buggy_func);
	int v = buggy_func(4500);
	DEBUG_LOG("is bug fixed? %s\n", v == 0 ? "yes": "no");
}

static void add_breakpoint(uint32_t inst) {
	add_hw_bkpt(inst);
	DEBUG_LOG("add dynamic point to: 0x%08x\n", inst);
	show_hw_bkpt();
}

static void RawBuggyFunc() {
	DEBUG_LOG("enter RawBuggyFunc\n");
	DEBUG_LOG("exit RawBuggyFunc\n");
}

static void FixedBuggyFunc() {
	DEBUG_LOG("run FixedBuggyFunc\n");
}



#define ipMAC_ADDRESS_LENGTH_BYTES ( 6 )
#define ipIP_ADDRESS_LENGTH_BYTES ( 4 )
#define ipSIZE_OF_ETH_HEADER			14u
#define ipSIZE_OF_IPv4_HEADER			20u

typedef struct NetworkBufferDescriptor {
	int xDataLength;
} NetworkBufferDescriptor_t;


typedef struct xMAC_ADDRESS
{
	uint8_t ucBytes[ ipMAC_ADDRESS_LENGTH_BYTES ];
} MACAddress_t;

typedef struct xETH_HEADER
{
	MACAddress_t xDestinationAddress; /*  0 + 6 = 6  */
	MACAddress_t xSourceAddress;      /*  6 + 6 = 12 */
	uint16_t usFrameType;              /* 12 + 2 = 14 */
} EthernetHeader_t;
 
typedef struct xIP_HEADER
{
	uint8_t ucVersionHeaderLength;        /*  0 + 1 =  1 */
	uint8_t ucDifferentiatedServicesCode; /*  1 + 1 =  2 */
	uint16_t usLength;                    /*  2 + 2 =  4 */
	uint16_t usIdentification;            /*  4 + 2 =  6 */
	uint16_t usFragmentOffset;            /*  6 + 2 =  8 */
	uint8_t ucTimeToLive;                 /*  8 + 1 =  9 */
	uint8_t ucProtocol;                   /*  9 + 1 = 10 */
	uint16_t usHeaderChecksum;            /* 10 + 2 = 12 */
	uint32_t ulSourceIPAddress;           /* 12 + 4 = 16 */
	uint32_t ulDestinationIPAddress;      /* 16 + 4 = 20 */
} IPHeader_t;

typedef struct xIP_PACKET {
	EthernetHeader_t xEthernetHeader;
	IPHeader_t xIPHeader;
} IPPacket_t;
static int freertos_cve_func(const IPPacket_t * const pxIPPacket, NetworkBufferDescriptor_t * const pxNetworkBuffer);

// compare with HERA
static void test_freertos_cve() {
	IPPacket_t pkt = {
		.xIPHeader = {
			.ucVersionHeaderLength = 50,
		}
	};
	NetworkBufferDescriptor_t data = {
		.xDataLength = 0,
	};
	profile_add_event("HERA Compare");
	
	profile_start(0);
	int t = freertos_cve_func(&pkt, &data);
	profile_end(0);
	
	profile_dump(0);
	
	profile_start(1);
	
	profile_end(1);
	hera_fix_func(&pkt, &data);
	profile_dump(1);
	DEBUG_LOG("bug func: %d\n", t);
}


int freertos_cve_func(const IPPacket_t * const pxIPPacket, NetworkBufferDescriptor_t * const pxNetworkBuffer) {
	
	const IPHeader_t * pxIPHeader = &( pxIPPacket->xIPHeader );
	uint32_t uxHeaderLength = ( uint32_t ) ( ( pxIPHeader->ucVersionHeaderLength & 0x0Fu ) << 2 );
	uint8_t ucProtocol;
	
	/* fixs: */
//	if (uxHeaderLength > pxNetworkBuffer->xDataLength - ipSIZE_OF_ETH_HEADER ||
//		uxHeaderLength < ipSIZE_OF_IPv4_HEADER)
//	{
//		return -1;
//	}

	// do some thing
	
	return 0;
}

int hera_fix_func(const IPPacket_t * const pxIPPacket, NetworkBufferDescriptor_t * const pxNetworkBuffer) {
	//DEBUG_LOG("hera_fix_func\n");
	const IPHeader_t * pxIPHeader = &( pxIPPacket->xIPHeader );
	uint32_t uxHeaderLength = ( uint32_t ) ( ( pxIPHeader->ucVersionHeaderLength & 0x0Fu ) << 2 );
	uint8_t ucProtocol;
	
	/* fixs: */
	if (uxHeaderLength > pxNetworkBuffer->xDataLength - ipSIZE_OF_ETH_HEADER ||
		uxHeaderLength < ipSIZE_OF_IPv4_HEADER)
	{
		return -1;
	}
	return 0;
}

static void fpb_setup_test() {
	uint32_t buggy_addr = (uint32_t) RawBuggyFunc & 0xFFFFFFFE;
	uint32_t fixed_addr = (uint32_t) FixedBuggyFunc & 0xFFFFFFFE;
	
	// fixed_addr = (uint32_t) FPB_Handler0 & 0xFFFFFFFE;
	/* HERA approach */
	//buggy_addr = (uint32_t) freertos_cve_func & 0xFFFFFFFE;
	//fixed_addr = (uint32_t) HERA_Dipatcher & 0xFFFFFFFE;

	
	// set buggy breakpoint
	int fidx = 1;
	FPB->COMP[fidx] = buggy_addr | 0x1;
	// set remap inst, bl xxx, jump to fixed addr 
	uint32_t jmp_inst = calc_branch_instr(buggy_addr, fixed_addr);
	const uint32_t PATCH_TBL = (uint32_t) (&REMAP_ADDR) & (~0x1F);
	//const uint32_t PATCH_TBL = 0x20010000;
	*((volatile uint32_t *) (PATCH_TBL + fidx * 4)) = little_endian_16_bit(jmp_inst);
	DEBUG_LOG("buggy_addr: 0x%08x fixed_addr: 0x%08x\n", buggy_addr, fixed_addr);
	DEBUG_LOG("ADDR: 0x%08x excepted value: 0x%08x\n", (&REMAP_ADDR), little_endian_16_bit(jmp_inst));
	DEBUG_LOG("remap: 0x%08x inst addr: 0x%08x\n", PATCH_TBL, *((volatile uint32_t *) (PATCH_TBL + fidx * 4)));
	// enable flash patch break point
	FPB->REMAP = PATCH_TBL;
	FPB->CTRL = 3;
}

void test_debugmon_patch() {
	DEBUG_LOG("Before add Kprobe Patch\n");
	set_patch_mode(CORTEX_DEB_MON_PATCH);
	
	test_func();
	DEBUG_LOG("Set Kprobe Patch for buggy_func\n");
	add_breakpoint((uint32_t) buggy_func);
	// do not find patch for pc addr: 0x080084e6
	// you should setup patch first
	test_func();
}

void test_fpb_flash_patch(void) {
	DEBUG_LOG("Before add FPB Flash Patch.\n");
	set_patch_mode(CORTEX_FPB_PATCH);

	RawBuggyFunc();
	DEBUG_LOG("Setup FPB Flash Patch.\n");
	fpb_setup_test();
	DEBUG_LOG("After FPB Flash Patch.\n");
	RawBuggyFunc();
}
#endif

