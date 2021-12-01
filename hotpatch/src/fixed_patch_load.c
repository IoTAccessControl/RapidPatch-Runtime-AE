#include "include/patch_point.h"
#include "include/iotpatch.h"
#include "include/patch_service.h"
#include "include/utils.h"
#include "libebpf/include/ebpf.h"
#include "libebpf/include/ebpf_allocator.h"
#include "include/fixed_patch_points.h"

struct local_patch {
	const char *cve;
	uint32_t loc;
	const uint8_t *code;
	int code_len;
};

static patch_desc *_desc;
static ebpf_patch *_patch;
static bool is_init = false;

static void setup_cve_list();
#ifdef EBPF_EVA
static uint8_t dummy_bug_function[] = ""
"\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

static uint8_t zephyr_cve_10063[] = ""
"\x61\x11\x04\x00\x00\x00\x00\x00\x07\x01\x00\x00\x05\x00\x00\x00\x67\x01\x00\x00\x20\x00\x00\x00\x77"
"\x01\x00\x00\x20\x00\x00\x00\x69\x12\x00\x00\x00\x00\x00\x00\x18\x01\x00\x00\xea\xff\xff\xff\x00\x00"
"\x00\x00\x00\x00\x00\x00\x25\x02\x01\x00\xff\xef\x00\x00\xb7\x01\x00\x00\x00\x00\x00\x00\xb7\x00\x00"
"\x00\x01\x00\x00\x00\x25\x02\x01\x00\xff\xef\x00\x00\xb7\x00\x00\x00\x00\x00\x00\x00\x67\x00\x00\x00"
"\x20\x00\x00\x00\x4f\x10\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

static uint8_t zephyr_cve_10062[] = ""
"\xb7\x06\x00\x00\x00\x00\x00\x00\xb7\x04\x00\x00\x01\x00\x00\x00\x61\x11\x00\x00\x00\x00\x00\x00\x79"
"\x12\x00\x00\x00\x00\x00\x00\x71\x21\x00\x00\x00\x00\x00\x00\x67\x01\x00\x00\x38\x00\x00\x00\xc7\x01"
"\x00\x00\x38\x00\x00\x00\xb7\x03\x00\x00\x00\x00\x00\x00\x65\x01\x07\x00\xff\xff\xff\xff\xb7\x06\x00"
"\x00\x01\x00\x00\x00\x71\x25\x01\x00\x00\x00\x00\x00\x67\x05\x00\x00\x38\x00\x00\x00\xc7\x05\x00\x00"
"\x38\x00\x00\x00\xb7\x04\x00\x00\x00\x00\x00\x00\xb7\x03\x00\x00\x01\x00\x00\x00\x6d\x54\x33\x00\x00"
"\x00\x00\x00\x57\x01\x00\x00\x7f\x00\x00\x00\x55\x04\x29\x00\x00\x00\x00\x00\xb7\x04\x00\x00\x07\x00"
"\x00\x00\xb7\x05\x00\x00\x01\x00\x00\x00\xbf\x30\x00\x00\x00\x00\x00\x00\x57\x00\x00\x00\x01\x00\x00"
"\x00\x55\x06\x1b\x00\x00\x00\x00\x00\xb7\x05\x00\x00\x00\x00\x00\x00\xb7\x04\x00\x00\x07\x00\x00\x00"
"\x57\x03\x00\x00\x02\x00\x00\x00\xbf\x27\x00\x00\x00\x00\x00\x00\x0f\x57\x00\x00\x00\x00\x00\x00\x71"
"\x76\x02\x00\x00\x00\x00\x00\x57\x06\x00\x00\x7f\x00\x00\x00\xbf\x48\x00\x00\x00\x00\x00\x00\x07\x08"
"\x00\x00\x07\x00\x00\x00\x67\x08\x00\x00\x20\x00\x00\x00\x77\x08\x00\x00\x20\x00\x00\x00\x6f\x86\x00"
"\x00\x00\x00\x00\x00\x71\x77\x01\x00\x00\x00\x00\x00\x57\x07\x00\x00\x7f\x00\x00\x00\xbf\x48\x00\x00"
"\x00\x00\x00\x00\x67\x08\x00\x00\x20\x00\x00\x00\x77\x08\x00\x00\x20\x00\x00\x00\x6f\x87\x00\x00\x00"
"\x00\x00\x00\x0f\x17\x00\x00\x00\x00\x00\x00\x0f\x76\x00\x00\x00\x00\x00\x00\x07\x04\x00\x00\x0e\x00"
"\x00\x00\x07\x05\x00\x00\x02\x00\x00\x00\xbf\x61\x00\x00\x00\x00\x00\x00\x1d\x53\x01\x00\x00\x00\x00"
"\x00\x05\x00\xea\xff\x00\x00\x00\x00\x07\x05\x00\x00\x01\x00\x00\x00\xbf\x61\x00\x00\x00\x00\x00\x00"
"\x15\x00\x08\x00\x00\x00\x00\x00\x0f\x52\x00\x00\x00\x00\x00\x00\x71\x22\x00\x00\x00\x00\x00\x00\x57"
"\x02\x00\x00\x7f\x00\x00\x00\x67\x04\x00\x00\x20\x00\x00\x00\x77\x04\x00\x00\x20\x00\x00\x00\x6f\x42"
"\x00\x00\x00\x00\x00\x00\x0f\x12\x00\x00\x00\x00\x00\x00\xbf\x21\x00\x00\x00\x00\x00\x00\x67\x01\x00"
"\x00\x20\x00\x00\x00\x77\x01\x00\x00\x20\x00\x00\x00\x18\x00\x00\x00\xea\xff\xff\xff\x00\x00\x00\x00"
"\x01\x00\x00\x00\x25\x01\x02\x00\xff\xff\xff\x0f\x18\x00\x00\x00\x2b\x02\x00\x00\x00\x00\x00\x00\x02"
"\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\xb7\x03\x00\x00\x02\x00\x00\x00\xb7\x04\x00\x00\x00\x00"
"\x00\x00\x71\x25\x02\x00\x00\x00\x00\x00\x67\x05\x00\x00\x38\x00\x00\x00\xc7\x05\x00\x00\x38\x00\x00"
"\x00\xb7\x06\x00\x00\x00\x00\x00\x00\x65\x05\xc6\xff\xff\xff\xff\xff\xb7\x03\x00\x00\x03\x00\x00\x00"
"\x71\x25\x03\x00\x00\x00\x00\x00\x67\x05\x00\x00\x38\x00\x00\x00\xc7\x05\x00\x00\x38\x00\x00\x00\xb7"
"\x06\x00\x00\x00\x00\x00\x00\x65\x05\xc0\xff\xff\xff\xff\xff\xb7\x06\x00\x00\x00\x00\x00\x00\xb7\x04"
"\x00\x00\x01\x00\x00\x00\x18\x00\x00\x00\xea\xff\xff\xff\x00\x00\x00\x00\x01\x00\x00\x00\x71\x25\x04"
"\x00\x00\x00\x00\x00\x67\x05\x00\x00\x38\x00\x00\x00\xc7\x05\x00\x00\x38\x00\x00\x00\xb7\x03\x00\x00"
"\x00\x00\x00\x00\x65\x05\xe9\xff\xff\xff\xff\xff\x05\x00\xb6\xff\x00\x00\x00\x00"
"";

// AMNESIA33_cve_2020_17445
static char AMNESIA33_cve_2020_17445[] = ""
"\x61\x12\x00\x00\x00\x00\x00\x00\x61\x13\x08\x00\x00\x00\x00\x00\x07\x03\x00\x00\x02\x00\x00\x00\x71"
"\x21\x01\x00\x00\x00\x00\x00\x07\x02\x00\x00\x02\x00\x00\x00\x67\x01\x00\x00\x03\x00\x00\x00\x47\x01"
"\x00\x00\x06\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x18\x04\x00"
"\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x71\x25\x01\x00\x00\x00\x00\x00\x07\x05\x00\x00"
"\x02\x00\x00\x00\xbf\x57\x00\x00\x00\x00\x00\x00\x57\x07\x00\x00\xff\x00\x00\x00\x15\x07\x11\x00\x00"
"\x00\x00\x00\xbf\x36\x00\x00\x00\x00\x00\x00\x0f\x76\x00\x00\x00\x00\x00\x00\xbf\x67\x00\x00\x00\x00"
"\x00\x00\x67\x07\x00\x00\x20\x00\x00\x00\x77\x07\x00\x00\x20\x00\x00\x00\x67\x03\x00\x00\x20\x00\x00"
"\x00\x77\x03\x00\x00\x20\x00\x00\x00\x3d\x73\x09\x00\x00\x00\x00\x00\x1f\x51\x00\x00\x00\x00\x00\x00"
"\x57\x05\x00\x00\xff\x00\x00\x00\x0f\x52\x00\x00\x00\x00\x00\x00\xb7\x04\x00\x00\x00\x00\x00\x00\xbf"
"\x15\x00\x00\x00\x00\x00\x00\x57\x05\x00\x00\xff\x00\x00\x00\xbf\x63\x00\x00\x00\x00\x00\x00\xb7\x00"
"\x00\x00\x00\x00\x00\x00\x55\x05\xe6\xff\x00\x00\x00\x00\x4f\x40\x00\x00\x00\x00\x00\x00\x95\x00\x00"
"\x00\x00\x00\x00\x00"
"";

static char AMNESIA33_cve_2020_17445_SFI[] = ""
"\xb7\x08\x00\x00\x00\x00\x00\x00\x61\x12\x00\x00\x00\x00\x00\x00\x61\x13\x08\x00"
"\x00\x00\x00\x00\x07\x03\x00\x00\x02\x00\x00\x00\x71\x21\x01\x00\x00\x00\x00\x00"
"\x07\x02\x00\x00\x02\x00\x00\x00\x67\x01\x00\x00\x03\x00\x00\x00\x47\x01\x00\x00"
"\x06\x00\x00\x00\x18\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00"
"\x18\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x71\x25\x01\x00"
"\x00\x00\x00\x00\x07\x05\x00\x00\x02\x00\x00\x00\xbf\x57\x00\x00\x00\x00\x00\x00"
"\x57\x07\x00\x00\xff\x00\x00\x00\x15\x07\x13\x00\x00\x00\x00\x00\xbf\x36\x00\x00"
"\x00\x00\x00\x00\x0f\x76\x00\x00\x00\x00\x00\x00\xbf\x67\x00\x00\x00\x00\x00\x00"
"\x67\x07\x00\x00\x20\x00\x00\x00\x77\x07\x00\x00\x20\x00\x00\x00\x67\x03\x00\x00"
"\x20\x00\x00\x00\x77\x03\x00\x00\x20\x00\x00\x00\x3d\x73\x0b\x00\x00\x00\x00\x00"
"\x1f\x51\x00\x00\x00\x00\x00\x00\x57\x05\x00\x00\xff\x00\x00\x00\x0f\x52\x00\x00"
"\x00\x00\x00\x00\xb7\x04\x00\x00\x00\x00\x00\x00\xbf\x15\x00\x00\x00\x00\x00\x00"
"\x57\x05\x00\x00\xff\x00\x00\x00\xbf\x63\x00\x00\x00\x00\x00\x00\xb7\x00\x00\x00"
"\x00\x00\x00\x00\x07\x08\x00\x00\x01\x00\x00\x00\x65\x08\x03\x00\x00\x08\x00\x00"
"\x55\x05\xe4\xff\x00\x00\x00\x00\x4f\x40\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00"
"\x00\x00\x00\x00\xb7\x00\x00\x00\x01\x00\x00\x00\x67\x00\x00\x00\x20\x00\x00\x00"
"\x95\x00\x00\x00\x00\x00\x00\x00"
"";

static struct local_patch patch_list[] = {
	{"dummy_bug_function", 0x08002950, dummy_bug_function, sizeof(dummy_bug_function)},
	{"zephyr_cve_2020_10063 attack_coap", 0x0000d0be, zephyr_cve_10063, sizeof(zephyr_cve_10063)},
	{"zephyr_cve_2020_10062 dummy_MQTT_packet_length_decode_patch", 0x08002c8e, zephyr_cve_10062, sizeof(zephyr_cve_10062)},
	{"AMNESIA33_cve_2020_17445 dummy_pico_ipv6_process_destopt_patch", 0x08002dec, AMNESIA33_cve_2020_17445_SFI, sizeof(AMNESIA33_cve_2020_17445_SFI)},
};

void setup_cve_list() {
	// patch_list[0].loc = (uint32_t) (bug_f)
}
#else
static struct local_patch patch_list[] = {};
void setup_cve_list() {}
#endif

void show_fixed_patch_desc() {
	int n = sizeof(patch_list) / sizeof(struct local_patch);
	for (int i = 0; i < n; i++) {
		DEBUG_LOG("%d -> %s\n", i, patch_list[i].cve);
	}
	setup_cve_list();
}

void load_local_fixed_patch(int pid) {
	int n = sizeof(patch_list) / sizeof(struct local_patch);
	if (pid > n) {
		show_fixed_patch_desc();
		DEBUG_LOG("patch id should less than %d\n", n);
		return;
	}

	if (!is_init) {
		_desc = ebpf_calloc(1, sizeof(patch_desc));
		_patch = ebpf_calloc(1, sizeof(ebpf_patch));
		is_init = true;
	}

	init_patch_sys();

	// DEBUG_LOG("start to load patch: %d\n", pid);
	struct local_patch *pt = &patch_list[pid];
	_desc->type = FixedPatchPoint;
	_desc->code_len = 0; // code do not save in desc now
	// inst addr should be divided by 4
	_desc->fixed_id = pt->loc;
	_patch->desc = _desc;
	ebpf_vm *vm = _patch->vm;
	if (vm == NULL) {
		vm = init_ebpf_vm(pt->code, pt->code_len);
	}
	ebpf_vm_set_inst(vm, pt->code, pt->code_len);
	vm->use_jit = false;
	if (vm->use_jit) {
		gen_jit_code(vm);
	}
	_patch->vm = vm;
	load_local_patch_to_ctx(_patch);
	DEBUG_LOG("load fixed patch %s success!\n", pt->cve);
}
