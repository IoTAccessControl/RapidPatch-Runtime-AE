#pragma once

//#include <stdint.h>
#include "ebpf_types.h"

/* eBPF definitions 
https://github.com/iovisor/bpf-docs/blob/master/eBPF.md
*/

typedef struct ebpf_inst {
	u8 opcode;
	u8 dst : 4;
	u8 src : 4;
	s16 offset;
	s32 imm;
} ebpf_inst;

// https://elixir.bootlin.com/linux/v4.4.110/source/kernel/bpf/core.c#L195

enum {
	BPF_REG_0 = 0,
	BPF_REG_1,
	BPF_REG_2,
	BPF_REG_3,
	BPF_REG_4,
	BPF_REG_5,
	BPF_REG_6,
	BPF_REG_7,
	BPF_REG_8,
	BPF_REG_9,
	BPF_REG_10,
	__MAX_BPF_REG,
};

/* BPF has 10 general purpose 64-bit registers and stack frame. */
#define MAX_BPF_REG	__MAX_BPF_REG

/* Kernel hidden auxiliary/helper register. */
#define BPF_REG_AX MAX_BPF_REG
#define MAX_BPF_EXT_REG		(MAX_BPF_REG + 1)
#define MAX_BPF_JIT_REG		MAX_BPF_EXT_REG
#define TMP_REG_1	(MAX_BPF_JIT_REG + 0)	/* TEMP Register 1 */
#define TMP_REG_2	(MAX_BPF_JIT_REG + 1)	/* TEMP Register 2 */
/* unused opcode to mark special call to bpf_tail_call() helper */
#define BPF_TAIL_CALL	0xf0
/* unused opcode to mark special load instruction. Same as BPF_ABS */
#define BPF_PROBE_MEM	0x20
/* unused opcode to mark call to interpreter with arguments */
#define BPF_CALL_ARGS	0xe0

#define BPF_REG_ARG1	BPF_REG_1
#define BPF_REG_ARG2	BPF_REG_2
#define BPF_REG_ARG3	BPF_REG_3
#define BPF_REG_ARG4	BPF_REG_4
#define BPF_REG_ARG5	BPF_REG_5
#define BPF_REG_CTX	BPF_REG_6
#define BPF_REG_FP	BPF_REG_10

/*
eBPF Args
*/

#define EBPF_CLS_MASK 0x07
#define EBPF_ALU_OP_MASK 0xf0

#define EBPF_CLS_LD 0x00
#define EBPF_CLS_LDX 0x01
#define EBPF_CLS_ST 0x02
#define EBPF_CLS_STX 0x03
#define EBPF_CLS_ALU 0x04
#define EBPF_CLS_JMP 0x05
#define EBPF_CLS_ALU64 0x07

#define EBPF_SRC_IMM 0x00
#define EBPF_SRC_REG 0x08

#define EBPF_SIZE_W 0x00
#define EBPF_SIZE_H 0x08
#define EBPF_SIZE_B 0x10
#define EBPF_SIZE_DW 0x18

/* Other memory modes are not yet supported */
#define EBPF_MODE_IMM 0x00
#define EBPF_MODE_MEM 0x60

#define EBPF_ALU_ADD 0x00
#define EBPF_ALU_SUB 0x10
#define EBPF_ALU_MUL 0x20
#define EBPF_ALU_DIV 0x30
#define EBPF_ALU_OR  0x40
#define EBPF_ALU_AND 0x50
#define EBPF_ALU_LSH 0x60
#define EBPF_ALU_RSH 0x70
#define EBPF_ALU_NEG 0x80
#define EBPF_ALU_MOD 0xc0
#define EBPF_ALU_XOR 0xa0
#define EBPF_ALU_ARSH 0xc0


#define EBPF_OP_ADD_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|EBPF_ALU_ADD) // BPF_K
#define EBPF_OP_ADD_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|EBPF_ALU_ADD) // BPF_X
#define EBPF_OP_SUB_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|EBPF_ALU_SUB)
#define EBPF_OP_SUB_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|EBPF_ALU_SUB)
#define EBPF_OP_MUL_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|0x20)
#define EBPF_OP_MUL_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|0x20)
#define EBPF_OP_DIV_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|0x30)
#define EBPF_OP_DIV_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|0x30)
#define EBPF_OP_OR_IMM   (EBPF_CLS_ALU|EBPF_SRC_IMM|0x40)
#define EBPF_OP_OR_REG   (EBPF_CLS_ALU|EBPF_SRC_REG|0x40)
#define EBPF_OP_AND_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|0x50)
#define EBPF_OP_AND_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|0x50)
#define EBPF_OP_LSH_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|0x60)
#define EBPF_OP_LSH_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|0x60)
#define EBPF_OP_RSH_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|0x70)
#define EBPF_OP_RSH_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|0x70)
#define EBPF_OP_NEG      (EBPF_CLS_ALU|0x80)
#define EBPF_OP_MOD_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|0x90)
#define EBPF_OP_MOD_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|0x90)
#define EBPF_OP_XOR_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|0xa0)
#define EBPF_OP_XOR_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|0xa0)
#define EBPF_OP_MOV_IMM  (EBPF_CLS_ALU|EBPF_SRC_IMM|0xb0)
#define EBPF_OP_MOV_REG  (EBPF_CLS_ALU|EBPF_SRC_REG|0xb0)
#define EBPF_OP_ARSH_IMM (EBPF_CLS_ALU|EBPF_SRC_IMM|0xc0)
#define EBPF_OP_ARSH_REG (EBPF_CLS_ALU|EBPF_SRC_REG|0xc0)
#define EBPF_OP_LE       (EBPF_CLS_ALU|EBPF_SRC_IMM|0xd0)
#define EBPF_OP_BE       (EBPF_CLS_ALU|EBPF_SRC_REG|0xd0)

#define EBPF_OP_ADD64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x00)
#define EBPF_OP_ADD64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0x00)
#define EBPF_OP_SUB64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x10)
#define EBPF_OP_SUB64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0x10)
#define EBPF_OP_MUL64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x20)
#define EBPF_OP_MUL64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0x20)
#define EBPF_OP_DIV64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x30)
#define EBPF_OP_DIV64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0x30)
#define EBPF_OP_OR64_IMM   (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x40)
#define EBPF_OP_OR64_REG   (EBPF_CLS_ALU64|EBPF_SRC_REG|0x40)
#define EBPF_OP_AND64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x50)
#define EBPF_OP_AND64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0x50)
#define EBPF_OP_LSH64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x60)
#define EBPF_OP_LSH64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0x60)
#define EBPF_OP_RSH64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x70)
#define EBPF_OP_RSH64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0x70)
#define EBPF_OP_NEG64      (EBPF_CLS_ALU64|0x80)
#define EBPF_OP_MOD64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0x90)
#define EBPF_OP_MOD64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0x90)
#define EBPF_OP_XOR64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0xa0)
#define EBPF_OP_XOR64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0xa0)
#define EBPF_OP_MOV64_IMM  (EBPF_CLS_ALU64|EBPF_SRC_IMM|0xb0)
#define EBPF_OP_MOV64_REG  (EBPF_CLS_ALU64|EBPF_SRC_REG|0xb0)
#define EBPF_OP_ARSH64_IMM (EBPF_CLS_ALU64|EBPF_SRC_IMM|0xc0)
#define EBPF_OP_ARSH64_REG (EBPF_CLS_ALU64|EBPF_SRC_REG|0xc0)

#define EBPF_OP_LDXW  (EBPF_CLS_LDX|EBPF_MODE_MEM|EBPF_SIZE_W)
#define EBPF_OP_LDXH  (EBPF_CLS_LDX|EBPF_MODE_MEM|EBPF_SIZE_H)
#define EBPF_OP_LDXB  (EBPF_CLS_LDX|EBPF_MODE_MEM|EBPF_SIZE_B)
#define EBPF_OP_LDXDW (EBPF_CLS_LDX|EBPF_MODE_MEM|EBPF_SIZE_DW)
#define EBPF_OP_STW   (EBPF_CLS_ST|EBPF_MODE_MEM|EBPF_SIZE_W)
#define EBPF_OP_STH   (EBPF_CLS_ST|EBPF_MODE_MEM|EBPF_SIZE_H)
#define EBPF_OP_STB   (EBPF_CLS_ST|EBPF_MODE_MEM|EBPF_SIZE_B)
#define EBPF_OP_STDW  (EBPF_CLS_ST|EBPF_MODE_MEM|EBPF_SIZE_DW)
#define EBPF_OP_STXW  (EBPF_CLS_STX|EBPF_MODE_MEM|EBPF_SIZE_W)
#define EBPF_OP_STXH  (EBPF_CLS_STX|EBPF_MODE_MEM|EBPF_SIZE_H)
#define EBPF_OP_STXB  (EBPF_CLS_STX|EBPF_MODE_MEM|EBPF_SIZE_B)
#define EBPF_OP_STXDW (EBPF_CLS_STX|EBPF_MODE_MEM|EBPF_SIZE_DW)
#define EBPF_OP_LDDW  (EBPF_CLS_LD|EBPF_MODE_IMM|EBPF_SIZE_DW)


#define EBPF_JA         0x00
#define EBPF_JEQ        0x10
#define EBPF_JGT        0x20
#define EBPF_JGE        0x30
#define EBPF_JSET       0x40
#define EBPF_JNE        0x50
#define EBPF_JSGT       0x60
#define EBPF_JSGE       0x70
#define EBPF_JLT        0xa0
#define EBPF_JLE        0xb0
#define EBPF_JSLT       0xc0
#define EBPF_JSLE       0xd0

#define EBPF_OP_JA       (EBPF_CLS_JMP|0x00)
#define EBPF_OP_JEQ_IMM  (EBPF_CLS_JMP|EBPF_SRC_IMM|0x10)
#define EBPF_OP_JEQ_REG  (EBPF_CLS_JMP|EBPF_SRC_REG|0x10)
#define EBPF_OP_JGT_IMM  (EBPF_CLS_JMP|EBPF_SRC_IMM|0x20)
#define EBPF_OP_JGT_REG  (EBPF_CLS_JMP|EBPF_SRC_REG|0x20)
#define EBPF_OP_JGE_IMM  (EBPF_CLS_JMP|EBPF_SRC_IMM|0x30)
#define EBPF_OP_JGE_REG  (EBPF_CLS_JMP|EBPF_SRC_REG|0x30)
#define EBPF_OP_JSET_REG (EBPF_CLS_JMP|EBPF_SRC_REG|0x40)
#define EBPF_OP_JSET_IMM (EBPF_CLS_JMP|EBPF_SRC_IMM|0x40)
#define EBPF_OP_JNE_IMM  (EBPF_CLS_JMP|EBPF_SRC_IMM|0x50)
#define EBPF_OP_JNE_REG  (EBPF_CLS_JMP|EBPF_SRC_REG|0x50)
#define EBPF_OP_JSGT_IMM (EBPF_CLS_JMP|EBPF_SRC_IMM|0x60)
#define EBPF_OP_JSGT_REG (EBPF_CLS_JMP|EBPF_SRC_REG|0x60)
#define EBPF_OP_JSGE_IMM (EBPF_CLS_JMP|EBPF_SRC_IMM|0x70)
#define EBPF_OP_JSGE_REG (EBPF_CLS_JMP|EBPF_SRC_REG|0x70)
#define EBPF_OP_CALL     (EBPF_CLS_JMP|0x80)
#define EBPF_OP_EXIT     (EBPF_CLS_JMP|0x90)
#define EBPF_OP_JLT_IMM  (EBPF_CLS_JMP|EBPF_SRC_IMM|0xa0)
#define EBPF_OP_JLT_REG  (EBPF_CLS_JMP|EBPF_SRC_REG|0xa0)
#define EBPF_OP_JLE_IMM  (EBPF_CLS_JMP|EBPF_SRC_IMM|0xb0)
#define EBPF_OP_JLE_REG  (EBPF_CLS_JMP|EBPF_SRC_REG|0xb0)
#define EBPF_OP_JSLT_IMM (EBPF_CLS_JMP|EBPF_SRC_IMM|0xc0)
#define EBPF_OP_JSLT_REG (EBPF_CLS_JMP|EBPF_SRC_REG|0xc0)
#define EBPF_OP_JSLE_IMM (EBPF_CLS_JMP|EBPF_SRC_IMM|0xd0)
#define EBPF_OP_JSLE_REG (EBPF_CLS_JMP|EBPF_SRC_REG|0xd0)

#define BPF_OP(code)    ((code) & 0xf0)
#define BPF_CLASS(code) ((code) & 0x07)
#define BPF_SIZE(code)  ((code) & 0x18)
#define BPF_SRC(code)  ((code) & 0x8)

