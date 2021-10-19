#ifndef IOTPATCH_H_
#define IOTPATCH_H_
#include <stdbool.h>
#include <stdint.h>
// #include "patch_point.h"
#include "libebpf/include/hashmap.h"
// #include "fixed_patch_points.h"

#define MAX_DP_NUM 6 // maximum dynamic patch num

enum FilterResult {
	FILTER_PASS = 0,
	FILTER_DROP = 1,
	FILTER_REDIRECT = 2,
	FILTER_UNUSED, // do not set patch
};

enum PatchCASFlag {
	FLAG_PATCH_READ,
	FLAG_PATCH_READ_END,
	FLAG_PATCH_WRITE,
	FLAG_PATCH_WRITE_END,
};

typedef enum PatchType {
	FixedPatchPoint = 1,
	DynamicPatchPoint
} PatchType;

// struct FixedPatchPoints;

// read copy update
typedef struct fixed_patch {
	//darray *fixed_patches;
	arraymap *fiexed_patches; // <addr, patch>
} fixed_patch;


typedef struct dynamic_patch {
	struct ebpf_patch *patch;
	struct dynamic_patch *next; // *prev
	uint32_t inst_addr;
	uint32_t pc_addr;
} dynamic_patch;

typedef struct patch_context {
	int phid;
	uint32_t fbits_filter; // bitmap for active filter patches
	fixed_patch fpatch_list;
	uint32_t dbits_filter_bpkt; // bloom filter for dynamic patch's bpkt address
	uint32_t dbits_filter_pc; // bloom filter for dynamic patch's address
	dynamic_patch dpatch_list; //
	// darray *bpkt;
} patch_context;

extern patch_context pctx;

struct patch_desc;
typedef struct ebpf_patch {
	struct patch_desc *desc;
	struct ebpf_vm *vm;
	bool is_active;
} ebpf_patch;

void init_patch_sys(void);
void destory_patch_context(void);
void destory_ebpf_patch(ebpf_patch *patch);
void show_all_patches(void);

struct patch_desc;
// void active_local_patch(struct ebpf_patch *patch);
void notify_new_patch(struct patch_desc *desc);

ebpf_patch* get_fixed_patch_by_lr(uint32_t lr);
// patch inst_addr = bpkt_addr
ebpf_patch* get_dynamic_patch_by_bpkt(uint32_t bpkt);
// the next inst addr of the patch addr = pc_addr
ebpf_patch* get_dynamic_patch_by_pc(uint32_t inst_pc);
// inst_addr = fpb_addr (inst_addr | 0x1)
ebpf_patch* get_dynamic_patch_by_fpb(uint32_t fpb_addr);
uint64_t run_ebpf_filter(ebpf_patch *patch, void *args, int args_size);

/*
*/
void load_local_patch_to_ctx(ebpf_patch *patch);

/*
Patch data serialize
*/
void save_patch_list_to_flash();
void load_patch_list_from_flash();

#endif
