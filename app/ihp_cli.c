#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "ihp_cli.h"
#include "hotpatch/include/iotpatch.h"
#include "dummy_cve.h"
#include "hotpatch/include/patch_service.h"
#include "hotpatch/include/utils.h"
#include "hotpatch/include/ebpf_test.h"
#include "hotpatch/include/patch_point.h"
#include "hotpatch/include/fixed_patch_points.h"
#include "rt_app.h"
#include "hotpatch/include/defs.h"

// https://github.com/memfault/interrupt/blob/master/example/firmware-shell/complex/shell/src/shell.c
// getchar from uart or linux-stdio
extern char shell_get_char();
extern void shell_put_char(char c);
static void cli_print_help();
static void handle_trigger_func(int tid);
static void show_patch_list();
static void handle_patch_func(int pid);
static void handle_eBPF_vm_run_patch(int vid);

#define SHELL_BUFFER_SIZE 128
#define SHELL_PROMPT "$ "
static bool exit_shell;
static struct shell_context {
	uint32_t rx_pos;
	char rx_buffer[SHELL_BUFFER_SIZE];
} shell_cli;

static void shell_echo(char c) {
	if (c == '\n') {
		shell_put_char('\r');
		shell_put_char('\n');
	} else {
		shell_put_char(c);
	}
}

static void shell_echo_str(char *str) {
	for (const char *c = str; *c != '\0'; c++) {
		shell_echo(*c);
	}
}

static void shell_printf(char *fmt, ...) {
	extern char log_buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, args);
	shell_echo_str(log_buf);
	va_end(args);
}

static inline char shell_last_char() {
	return shell_cli.rx_buffer[shell_cli.rx_pos - 1];
}

static void shell_dispatch_cmd(char *argv[], int argc) {
	if (strcmp(argv[0], "q") == 0 ) {
		exit_shell = true;
		DEBUG_LOG("Exit shell mode!\n");
		return;
	}
	
	if (strcmp(argv[0], "h") == 0 || strcmp(argv[0], "help") == 0) {
		cli_print_help();
	} else if (strcmp(argv[0], "run") == 0) {
		if (argc >= 2) {
			int rid = str2int(argv[1]);
			run_test_by_id(rid);
		}
	} else if (strcmp(argv[0], "trigger") == 0) {
		if (argc >= 2) {
			int tid = str2int(argv[1]);
			handle_trigger_func(tid);
		} else {
			shell_printf("Usage: trigger [cve]\n");
		}
	} else if (strcmp(argv[0], "patch") == 0) {
		if (argc >= 2) {
			int pid = str2int(argv[1]);
			handle_patch_func(pid);
		} else {
			shell_printf("Usage: patch [cve]\n");
			show_patch_list();
		}
	} else if (strcmp(argv[0], "vm") == 0) {
		if (argc >= 2) {
			int tid = str2int(argv[1]);
			handle_eBPF_vm_run_patch(tid);
		} else {
			shell_printf("Usage: vm [cve]\n");
		}
	}else {
		DEBUG_LOG("Command not find: %s argc: %d\n", argv[0], argc);
	}
}

// shell implementation
static void shell_process() {
	if (shell_last_char() != '\n' && shell_last_char() != '\r') {
		return;
	}

	int argc = 0;
	const int max_args = 5;
	char *argv[max_args];
	char *next_arg = NULL;
	for (int i = 0; i < shell_cli.rx_pos && argc < max_args; i++) {
		char * cur = &shell_cli.rx_buffer[i];
		if (*cur == '\n' || *cur == ' ' || i == shell_cli.rx_pos - 1) {
			*cur = '\0';
			if (next_arg) {
				argv[argc++] = next_arg;
				next_arg = NULL;
			}
		} else if (next_arg == NULL){
			next_arg = cur;
		}
	}

	if (argc > 0) {
		shell_dispatch_cmd(argv, argc);
	}

	memset(&shell_cli, 0, sizeof(shell_cli));
	shell_echo_str(SHELL_PROMPT);
}

static void shell_receive_char(char c) {
	shell_put_char(c);

#ifdef DEV_QEMU
	if (shell_cli.rx_pos >= SHELL_BUFFER_SIZE) {
		return;
	}
#else
	if (c == '\r' || shell_cli.rx_pos >= SHELL_BUFFER_SIZE) {
		return;
	}
#endif

	shell_cli.rx_buffer[shell_cli.rx_pos++] = c;
	shell_process();
}

static void test_rt_task(void) {
	// init_rt_task_servo_motor();
}

static void test_debug_patch(void) {
	DEBUG_LOG("test_debug_patch\n");
#ifndef LINUX_TEST
	test_debugmon_patch();
#endif
	
}

static void test_patch_trigger(void) {
	//add_hw_bkpt((uintptr_t) dev_test);
	//show_dynamic_patch_points();
#ifndef LINUX_TEST
	trigger_debugmon();
#endif
}

static void reset_patch() {
	// show_cve_info();
	// test_dummy_cve();
#ifndef LINUX_TEST
	show_hw_bkpt();
	clear_all_hw_bkpt();
	destroy_patch_context();
#endif
}

/*
Fixed patch point test

*/
static void run_eva_test() {
	// test_dummy_cve();
	#ifndef DEV_QEMU
	run_ebpf_test();
	#endif
}

static void run_load_fixed_patch_point_test() {
	load_local_fixed_patch(2);
}

static void run_fixed_patch_point_test() {
	test_fixed_patch_point();
}

static void run_unbounded_loop_test() {
	load_local_fixed_patch(3);
	test_unbounded_loop();
}

struct cli_cmd {
	int idx;
	void (*func) (void);
	char *help;
};

static struct cli_cmd cmds[] = {
	{TEST_LOG, test_debug_patch, "Test FPB breakpoint add"},
	// {TEST_LOG, test_rt_task, "Test Realtime Task"},
	{TEST_FPB, test_patch_trigger, "Test FPB patch trigger"},
	{TEST_CLEAR, reset_patch, "Clear all bpkt and patch"},
	{TEST_EBPF, run_eva_test, "Run eva test"},
	{TEST_SVR, start_patch_service, "Start patch service"},
	{TEST_CVE_2020_10062_VUL_FUNC, run_fixed_patch_point_test, "Invoke the vulnerable function for CVE-2020-10062"},
	{TEST_LOAD_FIXED_PATCH_FOR_CVE_2020_10062, run_load_fixed_patch_point_test, "Load patch at the fixed patch point for CVE-2020-10062"},
	{TEST_CVE_2020_17445_VUL_FUNC, run_unbounded_loop_test, "Invoke the vulnerable function for CVE-2020-17445"},
};

static void cli_print_help() {
	int ncmd = sizeof(cmds) / sizeof(struct cli_cmd);
	shell_printf("IoTPatch Cli Usage: run [idx] | trigger [cve] | patch [cve] | vm [vid]\n");
	for (int i = 0; i < ncmd; i++) {
		shell_printf("run %d: %s\n", cmds[i].idx, cmds[i].help);
	}
}

void run_test_by_id(run_test_t cid) {
	int ncmd = sizeof(cmds) / sizeof(struct cli_cmd);
	if (cid > ncmd) {
		shell_printf("Unsupport command: %d\n", cid);
		cli_print_help();
	} else {
		struct cli_cmd *cmd = &cmds[cid];
		if (cmd->func != NULL) {
			shell_printf("run cmd: %d {%s}\n", cmd->idx, cmd->help);
			cmd->func();
		} else {
			shell_printf("cmd:%d {%s} is not implement\n", cmd->idx, cmd->help);
		}
	}
}

// static 
#include "cve_trigger.h"
static void handle_trigger_func(int tid) {

#if USE_DYNAMIC_PATCH
	int n = sizeof(dynamic_trigger_func_list) / sizeof(trigger_func);
	if (tid >= n) {
		DEBUG_LOG("ERROR: trigger index out of range %d >= %d\n", tid, n);
		return;
	}
	dynamic_trigger_func_list[tid]();
#elif USE_FIX_PATCH
	int n = sizeof(fixed_trigger_func_list) / sizeof(trigger_func);
	if (tid >= n) {
		DEBUG_LOG("ERROR: trigger index out of range %d >= %d\n", tid, n);
		return;
	}
	fixed_trigger_func_list[tid]();
#else 
	DEBUG_LOG("Not impl! handle_trigger_func\n");
#endif
}



static void show_patch_list() {
#if USE_DYNAMIC_PATCH
	DEBUG_LOG("Use Dynamic Patch Points\n");	
	show_local_patch_desc();
#elif USE_FIX_PATCH
	DEBUG_LOG("Use Fixed Patch Points\n");
	show_fixed_patch_desc();
#else
	DEBUG_LOG("Not impl. show_patch_list\n");
#endif
}

// static bool has_init = false;
static void handle_patch_func(int pid) {
	show_patch_list();
#if USE_DYNAMIC_PATCH // dynamic patch
	set_patch_mode(CORTEX_DEB_MON_PATCH);
	// set_patch_mode(CORTEX_FPB_PATCH);
	//show_local_patch_desc();
	// if (use_fpb) return;
	read_local_patch(pid);
	show_hw_bkpt();
	show_all_patches();
	// use_fpb = true;
#elif USE_FIX_PATCH // fixed patch points
	load_local_fixed_patch(pid);
#else 
	// 
#endif
}

static void handle_eBPF_vm_run_patch(int vid) {
#ifndef LINUX_TEST
	//jit_dummy_cve();
#endif
	run_ebpf_eva(vid);
}


void run_shell_cli(void) {
	// extern uint32_t SystemCoreClock;
	// shell_printf("SystemCoreClock: %d\n", SystemCoreClock);
	memset(&shell_cli, 0, sizeof(shell_cli));
	cli_print_help();
	shell_echo_str(SHELL_PROMPT);
	exit_shell = false;
	while (!exit_shell) {
		char c = shell_get_char();
		shell_receive_char(c);
	}
}
