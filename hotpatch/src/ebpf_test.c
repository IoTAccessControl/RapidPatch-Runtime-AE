#include "include/ebpf_test.h"
#include "include/utils.h"
#include "include/iotpatch.h"
#include "include/patch_point.h"
#include "include/profiling.h"
#include "libebpf/include/hashmap.h"
#include "libebpf/include/ebpf.h"
#include "include/defs.h"

#ifdef EVA_TEST
/* 局部宏 EBPF_EVA
*/

static int binary_search(uint32_t *arr, int size, uint32_t val) {
	int start = 0;
	int end = size - 1;
	int mid = 0;
	while (start + 1 < end) {
		mid = (start + end) / 2;
		if (arr[mid] == val) {
			end = mid;
		} else if (arr[mid] > val) { // (start, mid]
			end = mid;
		} else { //
			start = mid;
		}
	}
	return end;
}

static void test_darray() {
	DEBUG_LOG("test_darray\n");
	darray *map =  darray_new(10);
	int val[20];
	int pos[20] = {0};
	for (int i = 0; i < 20; i++) {
		val[i] = i;
		pos[i] = darray_add(map, &val[i]);
	}
	DEBUG_LOG("map cur_idx: %d max_idx: %d\n", map->cur_size, map->max_size);
	for (int i = 0; i < 20; i++) {
		DEBUG_LOG("val[%d] = %d \n", i, *((int *)darray_get(map, pos[i])));
	}
	for (int i = 0; i < 20; i++) {
		if (i % 3 != 1) {
			darray_del(map, i);
		} else {
			darray_add(map, &val[i]);
		}
	}

	DEBUG_LOG("map cur_idx: %d max_idx: %d\n", map->cur_size, map->max_size);
	for (int i = 0; i < 20; i++) {
		int * ptr = (int *)darray_get(map, i);
		if (ptr == NULL) {
			DEBUG_LOG("val[%d] = %d \n", i, 0);
		} else {
			DEBUG_LOG("val[%d] = %d \n", i, *ptr);
		}
	}
	darray_destroy(map);
}

static void test_arraymap() {
	uint32_t arr[] = {2, 3, 4, 5, 5, 5, 6, 8, 8, 9, 9, 11, 12};
	int size = sizeof(arr) / sizeof(uint32_t);
	int val = 7;
	int idx = binary_search(arr, size, val);
	DEBUG_LOG("find val: %d idx: %d find: %d\n", val, idx, arr[idx]);

	arraymap *map = arraymap_new(8);
	for (uint32_t i = 0; i < size; i++) {
		arraymap_set(map, i, &arr[i]);
		if (i % 2 == 1) {
			arr[i]++;
			arraymap_set(map, i, &arr[i]);
		}
		void *val = arraymap_get(map, i);
		if (val != NULL) {
			DEBUG_LOG("key: %d val: %u sz: %u\n", i, *(uint32_t*) val, map->max_size);
		} else {
			DEBUG_LOG("not find key: %d\n", i);
		}
	}
	for (int i = 0; i < size; i++) {
		if (i % 2) {
			arraymap_del(map, i);
		}
	}
	for (int i = 0; i < map->cur_size; i++) {
		DEBUG_LOG("%d key: %u val: %u\n", i, map->keys[i], *((uint32_t *)map->vals[i]));
	}
}

struct __attribute__((aligned(2))) mydata1 {
	uint16_t pkt_len;
	uint32_t i32;
	uint8_t sign[16];
	uint16_t i16;
	uint8_t data[0];
};

struct mydata2 {
	uint8_t i1;
	uint32_t i4;
	uint16_t i2;
};

struct __attribute__((__packed__)) patch_descmy {
	uint16_t type;
	uint16_t code_len;
	union {
		uint32_t fixed_id;
		uint32_t inst_addr;
	};
	uint8_t code[0];
};

static void test_struct_serialize() {
	int t = sizeof(struct mydata1);
	int t2 = sizeof(struct mydata2);
	int t3 = sizeof(struct patch_descmy);
	DEBUG_LOG("struct sz: %d %d\n", t, t2);
	DEBUG_LOG("size of: %d\n", t3);
}

// Test EBPF
#ifdef EBPF_EVA
/*
https://github.com/zephyrproject-rtos/zephyr/issues/15605
https://community.arm.com/developer/ip-products/processors/f/cortex-m-forum/7179/hard-fault-in-cortex-m4

args_frame->r1

*/
typedef struct args_stack_frame {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12; // ip
	uint32_t lr;
	uint32_t pc; // return address
	uint32_t xpsr;
} __attribute__((__packed__)) args_stack_frame;

void ebpf_eva(uint8_t *code, int code_len, void *args, int ags_len);

static uint8_t code0[] = ""
"\x61\x11\x00\x00\x00\x00\x00\x00\x07\x01\x00\x00\x2f\xf8\xff\xff\x67\x01\x00\x00\x20\x00\x00\x00\x77"
"\x01\x00\x00\x20\x00\x00\x00\xb7\x00\x00\x00\x01\x00\x00\x00\xb7\x02\x00\x00\xb8\x0b\x00\x00\x2d\x12"
"\x01\x00\x00\x00\x00\x00\xb7\x00\x00\x00\x00\x00\x00\x00\x67\x00\x00\x00\x20\x00\x00\x00\x95\x00\x00"
"\x00\x00\x00\x00\x00"
"";

// zephyr_cve_2020_10063
static uint8_t code1[] = ""
"\x61\x11\x04\x00\x00\x00\x00\x00\x07\x01\x00\x00\x05\x00\x00\x00\x67\x01\x00\x00\x20\x00\x00\x00\x77"
"\x01\x00\x00\x20\x00\x00\x00\x69\x11\x00\x00\x00\x00\x00\x00\x18\x00\x00\x00\xea\xff\xff\xff\x00\x00"
"\x00\x00\x01\x00\x00\x00\x25\x01\x01\x00\xff\xef\x00\x00\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00"
"\x00\x00\x00\x00\x00"
"";

// zephyr_cve_2020_10021
static uint8_t code2[] = ""
"\x61\x12\x00\x00\x00\x00\x00\x00\x61\x11\x04\x00\x00\x00\x00\x00\x61\x11\x00\x00\x00\x00\x00\x00\xb7"
"\x00\x00\x00\x00\x00\x00\x00\x2d\x21\x02\x00\x00\x00\x00\x00\x18\x00\x00\x00\x7c\x97\x00\x00\x00\x00"
"\x00\x00\x01\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

// zephyr_cve_2020_10023
static char code3[] = ""
"\x61\x11\x00\x00\x00\x00\x00\x00\xbf\x12\x00\x00\x00\x00\x00\x00\x07\x02\x00\x00\xff\xff\xff\xff\x71"
"\x23\x01\x00\x00\x00\x00\x00\x07\x02\x00\x00\x01\x00\x00\x00\x55\x03\xfd\xff\x00\x00\x00\x00\x18\x00"
"\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x15\x01\x13\x00\x00\x00\x00\x00\x1f\x12\x00"
"\x00\x00\x00\x00\x00\xbf\x20\x00\x00\x00\x00\x00\x00\x57\x00\x00\x00\xff\xff\x00\x00\xb7\x03\x00\x00"
"\x02\x00\x00\x00\x2d\x03\x0e\x00\x00\x00\x00\x00\xb7\x03\x00\x00\x00\x00\x00\x00\xb7\x04\x00\x00\xff"
"\xff\x00\x00\xb7\x05\x00\x00\x00\x00\x00\x00\x05\x00\x0b\x00\x00\x00\x00\x00\x1f\x02\x00\x00\x00\x00"
"\x00\x00\xb7\x05\x00\x00\x00\x00\x00\x00\x07\x04\x00\x00\xff\xff\xff\xff\xbf\x36\x00\x00\x00\x00\x00"
"\x00\x57\x06\x00\x00\xff\xff\x00\x00\xbf\x20\x00\x00\x00\x00\x00\x00\x57\x00\x00\x00\xff\xff\x00\x00"
"\xbf\x07\x00\x00\x00\x00\x00\x00\x07\x07\x00\x00\xff\xff\xff\xff\x6d\x67\x01\x00\x00\x00\x00\x00\x95"
"\x00\x00\x00\x00\x00\x00\x00\xbf\x50\x00\x00\x00\x00\x00\x00\xbf\x36\x00\x00\x00\x00\x00\x00\x57\x06"
"\x00\x00\xff\xff\x00\x00\xbf\x17\x00\x00\x00\x00\x00\x00\x0f\x67\x00\x00\x00\x00\x00\x00\x07\x03\x00"
"\x00\x01\x00\x00\x00\x71\x76\x00\x00\x00\x00\x00\x00\x55\x06\xef\xff\x20\x00\x00\x00\xbf\x26\x00\x00"
"\x00\x00\x00\x00\x57\x06\x00\x00\xff\xff\x00\x00\xbf\x37\x00\x00\x00\x00\x00\x00\x57\x07\x00\x00\xff"
"\xff\x00\x00\xbf\x05\x00\x00\x00\x00\x00\x00\x3d\x67\xe9\xff\x00\x00\x00\x00\xbf\x25\x00\x00\x00\x00"
"\x00\x00\x0f\x05\x00\x00\x00\x00\x00\x00\x0f\x45\x00\x00\x00\x00\x00\x00\xbf\x36\x00\x00\x00\x00\x00"
"\x00\xbf\x67\x00\x00\x00\x00\x00\x00\x57\x07\x00\x00\xff\xff\x00\x00\xbf\x18\x00\x00\x00\x00\x00\x00"
"\x0f\x78\x00\x00\x00\x00\x00\x00\x71\x87\x00\x00\x00\x00\x00\x00\x55\x07\xdd\xff\x20\x00\x00\x00\x07"
"\x00\x00\x00\x01\x00\x00\x00\xbf\x27\x00\x00\x00\x00\x00\x00\x57\x07\x00\x00\xff\xff\x00\x00\x07\x06"
"\x00\x00\x01\x00\x00\x00\xbf\x68\x00\x00\x00\x00\x00\x00\x57\x08\x00\x00\xff\xff\x00\x00\x1d\x78\xd8"
"\xff\x00\x00\x00\x00\x05\x00\xf2\xff\x00\x00\x00\x00"
"";

// zephyr_cve_2020_10024
static char code4[] = ""
"\xb7\x02\x00\x00\x00\x00\x00\x00\x61\x13\x1c\x00\x00\x00\x00\x00\x57\x03\x00\x00\x00\x00\x00\x20\xb7"
"\x00\x00\x00\x00\x00\x00\x00\x15\x03\x06\x00\x00\x00\x00\x00\x18\x02\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x02\x00\x00\x00\x61\x10\x18\x00\x00\x00\x00\x00\x07\x00\x00\x00\x04\x00\x00\x00\x67\x00\x00"
"\x00\x20\x00\x00\x00\x77\x00\x00\x00\x20\x00\x00\x00\x4f\x20\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00"
"\x00\x00\x00\x00"
"";

// zephyr_cve_2020_10028
static char code5[] = ""
"\x61\x11\x04\x00\x00\x00\x00\x00\x61\x11\x04\x00\x00\x00\x00\x00\x61\x11\x00\x00\x00\x00\x00\x00\x18"
"\x00\x00\x00\xea\xff\xff\xff\x00\x00\x00\x00\x01\x00\x00\x00\x15\x01\x01\x00\x00\x00\x00\x00\xb7\x00"
"\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

// TODO
// zephyr_cve_2020_10062
static char code6[] = ""
"\xb7\x03\x00\x00\x01\x00\x00\x00\x61\x11\x00\x00\x00\x00\x00\x00\x79\x12\x00\x00\x00\x00\x00\x00\x71"
"\x21\x00\x00\x00\x00\x00\x00\x67\x01\x00\x00\x38\x00\x00\x00\xc7\x01\x00\x00\x38\x00\x00\x00\x65\x01"
"\x06\x00\xff\xff\xff\xff\xb7\x03\x00\x00\x02\x00\x00\x00\x71\x24\x01\x00\x00\x00\x00\x00\x67\x04\x00"
"\x00\x38\x00\x00\x00\xc7\x04\x00\x00\x38\x00\x00\x00\xb7\x05\x00\x00\x00\x00\x00\x00\x6d\x45\x17\x00"
"\x00\x00\x00\x00\x57\x01\x00\x00\x7f\x00\x00\x00\x55\x03\x04\x00\x01\x00\x00\x00\xb7\x00\x00\x00\xea"
"\xff\xff\xff\x25\x01\x01\x00\xff\xff\xff\x0f\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00"
"\x00\x00\x71\x24\x01\x00\x00\x00\x00\x00\x57\x04\x00\x00\x7f\x00\x00\x00\x67\x04\x00\x00\x07\x00\x00"
"\x00\x4f\x14\x00\x00\x00\x00\x00\x00\xbf\x41\x00\x00\x00\x00\x00\x00\x15\x03\xf6\xff\x02\x00\x00\x00"
"\x71\x21\x02\x00\x00\x00\x00\x00\x57\x01\x00\x00\x7f\x00\x00\x00\x67\x01\x00\x00\x0e\x00\x00\x00\x4f"
"\x41\x00\x00\x00\x00\x00\x00\x15\x03\xf1\xff\x03\x00\x00\x00\x71\x22\x03\x00\x00\x00\x00\x00\x57\x02"
"\x00\x00\x7f\x00\x00\x00\x67\x02\x00\x00\x15\x00\x00\x00\x4f\x12\x00\x00\x00\x00\x00\x00\xbf\x21\x00"
"\x00\x00\x00\x00\x00\x05\x00\xeb\xff\x00\x00\x00\x00\xb7\x03\x00\x00\x03\x00\x00\x00\x71\x24\x02\x00"
"\x00\x00\x00\x00\x67\x04\x00\x00\x38\x00\x00\x00\xc7\x04\x00\x00\x38\x00\x00\x00\x65\x04\xe4\xff\xff"
"\xff\xff\xff\xb7\x03\x00\x00\x04\x00\x00\x00\x71\x24\x03\x00\x00\x00\x00\x00\x67\x04\x00\x00\x38\x00"
"\x00\x00\xc7\x04\x00\x00\x38\x00\x00\x00\x65\x04\xdf\xff\xff\xff\xff\xff\x18\x00\x00\x00\xea\xff\xff"
"\xff\x00\x00\x00\x00\x00\x00\x00\x00\x71\x24\x04\x00\x00\x00\x00\x00\x67\x04\x00\x00\x38\x00\x00\x00"
"\xc7\x04\x00\x00\x38\x00\x00\x00\xb7\x03\x00\x00\x01\x00\x00\x00\x65\x04\xdd\xff\xff\xff\xff\xff\x05"
"\x00\xd7\xff\x00\x00\x00\x00"
"";

// freertos_cve_2018_16524
static char code7[] = ""
"\xb7\x00\x00\x00\x00\x00\x00\x00\x61\x11\x04\x00\x00\x00\x00\x00\x71\x11\x18\x00\x00\x00\x00\x00\x71"
"\x12\x2e\x00\x00\x00\x00\x00\xb7\x03\x00\x00\x60\x00\x00\x00\x2d\x23\x24\x00\x00\x00\x00\x00\x77\x02"
"\x00\x00\x02\x00\x00\x00\x57\x02\x00\x00\x3c\x00\x00\x00\x0f\x12\x00\x00\x00\x00\x00\x00\x07\x02\x00"
"\x00\x22\x00\x00\x00\x07\x01\x00\x00\x36\x00\x00\x00\x71\x13\x00\x00\x00\x00\x00\x00\x65\x03\x03\x00"
"\x01\x00\x00\x00\x15\x03\x09\x00\x01\x00\x00\x00\x15\x03\x1b\x00\x00\x00\x00\x00\x05\x00\x02\x00\x00"
"\x00\x00\x00\x15\x03\x0e\x00\x02\x00\x00\x00\x15\x03\x08\x00\x03\x00\x00\x00\x71\x13\x01\x00\x00\x00"
"\x00\x00\x0f\x31\x00\x00\x00\x00\x00\x00\x15\x03\x15\x00\x00\x00\x00\x00\x2d\x12\xf5\xff\x00\x00\x00"
"\x00\x05\x00\x13\x00\x00\x00\x00\x00\x07\x01\x00\x00\x01\x00\x00\x00\x2d\x12\xf2\xff\x00\x00\x00\x00"
"\x05\x00\x10\x00\x00\x00\x00\x00\x71\x13\x01\x00\x00\x00\x00\x00\x55\x03\xf7\xff\x03\x00\x00\x00\x07"
"\x01\x00\x00\x03\x00\x00\x00\x2d\x12\xed\xff\x00\x00\x00\x00\x05\x00\x0b\x00\x00\x00\x00\x00\x71\x13"
"\x01\x00\x00\x00\x00\x00\x55\x03\xf2\xff\x04\x00\x00\x00\x71\x12\x03\x00\x00\x00\x00\x00\x71\x11\x02"
"\x00\x00\x00\x00\x00\x67\x01\x00\x00\x08\x00\x00\x00\x4f\x21\x00\x00\x00\x00\x00\x00\x57\x01\x00\x00"
"\xff\xff\x00\x00\xb7\x00\x00\x00\x01\x00\x00\x00\x15\x01\x01\x00\x00\x00\x00\x00\xb7\x00\x00\x00\x00"
"\x00\x00\x00\x67\x00\x00\x00\x20\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";


// freertos_cve_2018_16528
static char code8_1[] = ""
"\x61\x13\x04\x00\x00\x00\x00\x00\x61\x12\x00\x00\x00\x00\x00\x00\x67\x02\x00\x00\x20\x00\x00\x00\xc7"
"\x02\x00\x00\x20\x00\x00\x00\x67\x03\x00\x00\x20\x00\x00\x00\xc7\x03\x00\x00\x20\x00\x00\x00\xb7\x01"
"\x00\x00\x00\x00\x00\x00\x85\x00\x00\x00\x03\x00\x00\x00\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00"
"\x00\x00\x00\x00\x00"
"";
static char code8_2[] = ""
"\x61\x12\x00\x00\x00\x00\x00\x00\x67\x02\x00\x00\x20\x00\x00\x00\xc7\x02\x00\x00\x20\x00\x00\x00\xb7"
"\x01\x00\x00\x00\x00\x00\x00\x85\x00\x00\x00\x04\x00\x00\x00\xbf\x01\x00\x00\x00\x00\x00\x00\x67\x01"
"\x00\x00\x20\x00\x00\x00\x77\x01\x00\x00\x20\x00\x00\x00\x18\x00\x00\x00\xd0\x07\x00\x00\x00\x00\x00"
"\x00\x02\x00\x00\x00\x15\x01\x01\x00\x00\x00\x00\x00\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00"
"\x00\x00\x00\x00"
"";

// freertos_cve_2018_16603
static char code9[] = ""
"\x61\x11\x00\x00\x00\x00\x00\x00\x79\x11\x1c\x00\x00\x00\x00\x00\x07\x01\x00\x00\xde\xff\xff\xff\xb7"
"\x00\x00\x00\x01\x00\x00\x00\xb7\x02\x00\x00\x14\x00\x00\x00\x2d\x12\x01\x00\x00\x00\x00\x00\xb7\x00"
"\x00\x00\x00\x00\x00\x00\x67\x00\x00\x00\x20\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

// mbedTLS_2017_2784
static char code10[] = ""
"\x61\x10\x18\x00\x00\x00\x00\x00\x67\x00\x00\x00\x20\x00\x00\x00\x18\x01\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x02\x00\x00\x00\x0f\x10\x00\x00\x00\x00\x00\x00\xc7\x00\x00\x00\x20\x00\x00\x00\x0f\x10"
"\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

// AMNESIA33_cve_2020_17443
static char code11[] = ""
"\x61\x11\x00\x00\x00\x00\x00\x00\x69\x11\x26\x00\x00\x00\x00\x00\x18\x00\x00\x00\xff\xff\xff\xff\x00"
"\x00\x00\x00\x01\x00\x00\x00\xb7\x02\x00\x00\x08\x00\x00\x00\x2d\x12\x01\x00\x00\x00\x00\x00\xb7\x00"
"\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

// AMNESIA33_cve_2020_17445
static char code12[] = ""
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


struct ebpf_code_eva {
	char *code;
	int len;
	void (*setup_args)(void *args);
};

int orig_c0(int v) {
	if (v > 2000) { // fixed: v > 5000 return -1
		//DEBUG_LOG("test_dynamic_bug-112: %d\n", v);
		return -1;
	}
	//DEBUG_LOG("test_dynamic_bug-: %d\n", v);
	return 0;
}

void test_ebpf_c0() {
	/* ebpf: 4511 
	jit: 111 
	origin: 34 
	*/
	args_stack_frame args_frame;
	args_frame.r0 = 2500;
	ebpf_eva(code0, sizeof(code0), &args_frame, sizeof(args_frame));

	profile_start(EV0);
	orig_c0(2500);
	profile_end(EV0);
	profile_dump(EV0);
}

void test_ebpf_c1() {
	/*
	zephyr_cve_2020_10063

	*/
	unsigned char testcase[] = {
		0, 0, 0, 0,
		0x0E, /* delta=0, length=14 */ // COAP_OPTION_EXT_14 = 14
		0xFE, 0xF0, /* First option */
		0x00 /* More data following the option to skip the "if (r == 0) {" case */
	};
	// uint8_t mem[48];
	args_stack_frame args_frame;
	//args_frame.r0 = (uint32_t) testcase;
	args_frame.r1 = (uint32_t) testcase;
	args_frame.r2 = 500;
	// memcpy(mem, &args_frame, sizeof(args_frame));
	uint32_t addr = args_frame.r1;
	// DEBUG_LOG("addr: 0x%08x 0x%08x val: %d\n", addr, testcase + 5, *(uint16_t *)(addr + 5));
	// DEBUG_LOG("test_ebpf_c1: 0x%08x 0x%08x 0x%08x %d\n", &args_frame, args_frame.r1, testcase, sizeof(args_frame));
	// DEBUG_LOG("struct size: %p %p %p\n", &args_frame, &(args_frame.r1), &(args_frame.r2));
	ebpf_eva(code1, sizeof(code1), &args_frame, sizeof(args_frame));
}

// problem
void test_ebpf_c2() {
	/*
	zephyr_cve_2020_10021
	*/
	int mem_size = 233;
	args_stack_frame args_frame ={
		.r0 = 20,
		.r1 = ((uint32_t)(&mem_size)),
		.r2 = 0,
		.r3 = 0,
		.r12 = 0,
		.lr = 0,
		.pc = 0,
		.xpsr = 0,
	};
	ebpf_eva(code2, sizeof(code2), &args_frame, sizeof(args_frame));
}

static void ebpf_call_c(int a1, int a2, char *a3, short a4) {
	DEBUG_LOG("ebpf_call_c: %d %d %d %d", a1, a2, a3, a4);
}

static void my_memmove(uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	char *dst = (char *) arg1;
	char *src = (char *) arg2;
	int len = arg3;
	// for (int i = 0; i < len; i++) {
	// 	dst[i] = src[i];
	// }
	DEBUG_LOG("my_memmove: %d %d %d\n", dst, src, len);
}

void test_ebpf_c3() { 
	// zephyr_cve_2020_10023
	char *test_str = "test\n";
	args_stack_frame args_frame = {0};
	args_frame.r0 = test_str;
	// args_frame.r1 = (uint32_t) ebpf_call_c;
	args_frame.r1 = (uint32_t) my_memmove;
	//args_frame.r2 = 2;
	ebpf_eva(code3, sizeof(code3), &args_frame, sizeof(args_frame));
}

void test_ebpf_c4() {
	// zephyr_cve_2020_10024
	args_stack_frame args_frame;
	args_frame.pc = 5;
	args_frame.xpsr = 7;
	ebpf_eva(code4, sizeof(code4), &args_frame, sizeof(args_frame));
}

void test_ebpf_c5() {
	// zephyr_cve_2020_10028
	static struct gpio_struct2 {
		int a;
		int b;
		int c;
		int d;
		int *e;
	};

	static struct gpio_struct1 {
		int a;
		struct gpio_struct2* b;
	};
	struct gpio_struct2 g2 = {
		.a = 0,
		.b = 0,
		.c = 0,
		.d = 0,
		.e = 0,
	};

	struct gpio_struct1 g1 = {
		.a = 0,
		.b = &g2,
	};

	args_stack_frame args_frame = {
		.r0 = 0,
		.r1 = &g1,
		.r2 = 0,
		.r3 = 0,
		.r12 = 0,
		.lr = 0,
		.pc = 0,
		.xpsr = 0,
	};
	ebpf_eva(code5, sizeof(code5), &args_frame, sizeof(args_frame));
}

void test_ebpf_c6() {
	// zephyr_cve_2020_10062
	struct mqtt_buf_ctx {
		unsigned char *cur;
		unsigned char *end;
	};
	unsigned char mqtt_buf[6] = {0x81, 0x21, 0x12, 0, 0, 0};
	struct mqtt_buf_ctx mbc = {
		.cur = mqtt_buf,
		.end = mqtt_buf + 10,
	};

	args_stack_frame args_frame = {
		.r0 = &mbc,
	};
	ebpf_eva(code6, sizeof(code6), &args_frame, sizeof(args_frame));
}

void test_ebpf_c7() {
	// freertos_cve_2018_16524
	static struct IPPacket {
		unsigned char padd[24];
		unsigned char *tcpptr;
	};

	static struct TCPPacket {
		unsigned char data[60];
	};

	struct TCPPacket tcppkt = {
		.data = {0},
	};

	struct IPPacket ippkt = {
		.padd = {0},
		.tcpptr = &tcppkt,
	};

	args_stack_frame args_frame = {
		.r0 = 0,
		.r1 = &ippkt,
	};
	ebpf_eva(code7, sizeof(code7), &args_frame, sizeof(args_frame));
}

void test_ebpf_c8() {
	// freertos_cve_2018_16603
	args_stack_frame args_frame;
	args_frame.r0 = 12;
	args_frame.r1 = 0; // 1 success, 0 failed
	// ebpf_eva(code8, sizeof(code8), &args_frame, sizeof(args_frame));
	ebpf_eva(code8_1, sizeof(code8_1), &args_frame, sizeof(args_frame));
	ebpf_eva(code8_2, sizeof(code8_2), &args_frame, sizeof(args_frame));
	// DEBUG_LOG("NOT IMPL\n");
}

void test_ebpf_c9() {
	// freertos_cve_2018_16603
	static struct ETHPacket {
		unsigned char padd[28];
		uint64_t xDataLength;
	};
	struct ETHPacket ethpkt = {
		.padd = {0},
		.xDataLength = 100,
	};
	args_stack_frame args_frame = {
		.r0 = &ethpkt
	};
	ebpf_eva(code9, sizeof(code9), &args_frame, sizeof(args_frame));
}

void test_ebpf_c10() {
	// DEBUG_LOG("NOT IMPL\n");
	args_stack_frame args_frame;
	args_frame.pc = 2020;
	ebpf_eva(code10, sizeof(code10), &args_frame, sizeof(args_frame));
}

void test_ebpf_c11() {
	// AMNESIA33_cve_2020_17443
	static struct PicoFrame {
		unsigned char padd[38];
		uint16_t transport_len;
	};
	struct PicoFrame pf = {
		.padd = {0},
		.transport_len = 7,
	};
	args_stack_frame args_frame = {
		.r0 = &pf,
	};
	ebpf_eva(code11, sizeof(code11), &args_frame, sizeof(args_frame));
}

void test_ebpf_c12() {
	// AMNESIA33_cve_2020_17445
	uint32_t opt_ptr = 0;
	uint8_t destopt[50] = {1, -2, -2, -2, -2, -2};
	args_stack_frame args_frame = {
		.r0 = destopt,
		.r1 = 0,
		.r2 = opt_ptr,
	};
	ebpf_eva(code12, sizeof(code12), &args_frame, sizeof(args_frame));
}

void ebpf_eva(uint8_t *code, int code_len, void *args, int ags_len) {
	bool test_jit = true;
	// test_jit = false;
	ebpf_vm vm;
	ebpf_vm_set_inst(&vm, code, code_len);
	//profile_exit();
	profile_add_event("ebpf");
	uint64_t ret1 = 0, ret2 = 0;
	profile_start(EV0);
	ret1 = ebpf_vm_exec(&vm, args, ags_len);
	profile_end(EV0);
	// jit_compile
	if (test_jit) {
		gen_jit_code(&vm);
		profile_start(EV1);
		ret2 = vm.jit_func(args, ags_len);
		profile_end(EV1);
		DEBUG_LOG("jit_func address: 0x%08x\n", (uint32_t) vm.jit_func);
		jit_mem_free(vm.jmem);
	}
	
	DEBUG_LOG("Interupter: Op=%d Ret=%d Jit: Op=%d Ret=%d\n", (int) (ret1 >> 32), (int) (ret1 & 0xffffffff), 
		(int) (ret2 >> 32), (int) (ret2 & 0xffffffff));
	profile_dump(EV0);
	profile_dump(EV1);

	int total_mem = jit_mem_statistic(&vm) + get_ebpf_alloc_size();
	DEBUG_LOG("mem size: %d bytes\n", total_mem);
	DEBUG_LOG("finish----------------------------------------\n");
}

typedef void (*ebpf_test_func)();

ebpf_test_func test_funcs[] = {
	test_ebpf_c0, test_ebpf_c1, test_ebpf_c2, test_ebpf_c3, test_ebpf_c4, test_ebpf_c5, test_ebpf_c6,
	test_ebpf_c7, test_ebpf_c8, test_ebpf_c9, test_ebpf_c10, test_ebpf_c11, test_ebpf_c12
};

#endif // EBPF_EVA


/*
Load prog to VM directly
*/
void run_ebpf_eva(int eid) {
#ifdef EBPF_EVA
	int n = sizeof(test_funcs) / sizeof(ebpf_test_func);
	if (eid < 0 || eid >= n) {
		DEBUG_LOG("Invalide idx: %d (should be 0~%d)\n", eid, n);
		return;
	}
	ebpf_test_func func = test_funcs[eid];
	func();
#endif // EBPF_EVA
}

/* -----------------------------------------------------------------
Other Evaluation:
1. 
*/
void measure_print() {
	profile_add_event("measure_print");
	profile_start(EV0);
	profile_end(EV0);
	profile_dump(EV0);
	profile_start(EV1);
	DEBUG_LOG("measure_print: %d\n", 123);
	profile_end(EV1);
	profile_dump(EV1);
}

// 
struct patch_list {
	arraymap *patches;
};

void patch_num_eva(struct patch_list *plist, int n, int times) {
	if (plist->patches == NULL) {
		plist->patches = arraymap_new(72);
	}
	for (int i = 0; i < n; i++) {
		arraymap_set(plist->patches, i, 1);
	}
	int start = get_cur_tick();
	for (int t = 0; t < times; t++) {
		for (int i = 0; i < n; i++) {
			int val = arraymap_get(plist->patches, i);
			(void) val;
		}
	}
	int cycles = get_cur_tick() - start;
	DEBUG_LOG("num: %d ti: %d\n", n, (int) (cycles2us(cycles / times / n) * 100));
}

/*
Event 0 -> cycle: 108 time(us): 1.687500 / 1
Event 0 -> cycle: 427 time(us): 6.671875 / 4
Event 0 -> cycle: 955 time(us): 14.921875 / 8
Event 0 -> cycle: 2121 time(us): 33.140625 / 16
Event 0 -> cycle: 5743 time(us): 89.734375 / 32
num: 1 ti: 1.500000
num: 4 ti: 1.531250
num: 8 ti: 3.671875
num: 16 ti: 2.859375
num: 32 ti: 2.578125
*/
void test_patch_dispatcher() {
	// setup patch list
	profile_add_event("EV0");
	profile_start(EV0);
	profile_end(EV0);
	profile_dump(EV0);
	struct patch_list plist;
	plist.patches = NULL;
	int TI = 100;
	for (int i = 1; i < 65; i += 4) {
		patch_num_eva(&plist, i, TI);
	}
	patch_num_eva(&plist, 64, TI);
	arraymap_destroy(plist.patches);
}

#ifdef DEV_STM32L475
#include "include/flash_api.h"

void flash_rw_speed() {
	test_flash_write_speed();
}
#else 
void flash_rw_speed() {
	DEBUG_LOG("Please run on DEV_STM32L475.\n");
}
#endif

/*
mov.w	r3, #1 // r3 = 1
add	r1, sp, #4
ldrexb	r2, [r1] // t = r2 (1) set
strexb	r0, r3, [r1] // r0 = 1 failed, r0 = 0 update memory, if t = r3
cmp	r0, #0
bne.n	d94e <spinlock+0x8>
dmb	ish // wait for store inst finish
__atomic_clear(&t, __ATOMIC_RELEASE);
dmb	ish
movs	r3, #0
strb.w	r3, [sp, #4]

lock: 0 -> 1，内存被更新
unlock: v -> 0
*/
#include "libebpf/include/ebpf_lock.h"
void spinlock() {
	int t;
	//__atomic_test_and_set(&t, __ATOMIC_ACQUIRE);
	//__atomic_clear(&t, __ATOMIC_RELEASE);
}

void run_ebpf_test() {
	DEBUG_LOG("run_ebpf_test >>>>>> \n");
	// measure_print();
	test_patch_dispatcher();
	// flash_rw_speed();
	// spinlock();

#ifndef LINUX_TEST
	// jit_dummy_cve();
#endif
	//test_darray();
	//test_arraymap();
	//test_struct_serialize();
	// int eid = profile_add_event("printf");
	// profile_start(eid);
	// DEBUG_LOG("this is a single printf:%d\n", eid);
	// profile_end(eid);
	// profile_dump(eid);
	// profile_exit();
}
#else
void run_ebpf_test() {
	DEBUG_LOG("run_ebpf_test Not Define EVA_TEST\n");
}

void run_ebpf_eva(int eid) {
	DEBUG_LOG("run_ebpf_test Not Define EVA_TEST\n");
}

uint64_t jit_dummy_cve() {
	DEBUG_LOG("run_ebpf_test Not Define EVA_TEST\n");
}
#endif 