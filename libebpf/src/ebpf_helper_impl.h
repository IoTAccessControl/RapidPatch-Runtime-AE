#ifndef EBPF_HELPER_IMPL_H_
#define EBPF_HELPER_IMPL_H_
#include "hotpatch/include/utils.h"
#include "include/hashmap.h"

struct ebpf_vm;

typedef uint32_t (*c_func)(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3);
extern int ebpf_register(struct ebpf_vm *vm, unsigned int idx, const char *name, void *fn);

void set_default_helpers(struct ebpf_vm *vm);

#ifdef SYS_CORTEX_M4
static void iot_print_log(char *str) {
	DEBUG_LOG(str);
}

//
// push {arg4, arg5, arg6}
static void iot_call_C_func_noret(uint64_t func_addr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	// typedef 
	c_func func = (c_func) (u32) func_addr;
	func(arg0, arg1, arg2, arg3);
	// fp 
	// DEBUG_LOG("%d %d %d %d\n", (u32) arg0, (u32) arg1, (u32) arg2, (u32) arg3);
	DEBUG_LOG("want to call c: func:0x%08x arg0:%d arg1:%d arg2:%d arg3:%d\n",  (u32) func_addr, (u32) arg0, (u32) arg1, (u32) arg2, (u32) arg3);
}

static void iot_call_C_func2_noret(void *arg1, void *arg2) {

}

// static 
static arraymap amap;
uint64_t dval;
static void tmp_map_save_val(void *mp, uint64_t key, uint64_t val) {
	if (mp == NULL) {

	}
	dval = val;
}

static uint64_t tmp_map_get_val(void *mp, uint64_t key) {
	return dval;
}

void set_default_helpers(struct ebpf_vm *vm) {
	ebpf_register(vm, 1, "print_log", iot_print_log);
	ebpf_register(vm, 2, "call_c_func", iot_call_C_func_noret);
	// DEBUG_LOG("func 2: 0x%08x\n", iot_call_C_func_noret);
	ebpf_register(vm, 3, "map_set", tmp_map_save_val);
	ebpf_register(vm, 4, "map_get", tmp_map_get_val);
}
#else
void set_default_helpers(struct ebpf_vm *vm) {

}
#endif

#endif // EBPF_HELPERS_H_
