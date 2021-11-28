#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "ihp_porting.h"
#include "ihp_config.h"

char log_buf[256] = {0};

#ifdef LINUX_TEST

void log_print_porting(const char *fmt, va_list *args) {
	// printf("Linux > ");
	vprintf(fmt, *args);
}

char shell_get_char() {
	return getchar();
}

void shell_put_char(char c) {
	putchar(c);
}

#endif

#ifdef NRF52_NO_OS
// #include "nrf_log.h"
// #include "nrf_log_ctrl.h"
#include "app_uart.h"

char shell_get_char() {
	uint8_t c;
	while (app_uart_get(&c) != NRF_SUCCESS);
	return c;
}

void shell_put_char(char c) {
	while (app_uart_put(c) != NRF_SUCCESS);
}

void log_print_porting(const char *fmt, va_list *args) {
	memset(log_buf, 0, sizeof(log_buf));
	vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, *args);
	//NRF_LOG_INFO("nrf52 > %s", log_buf);
	//NRF_LOG_FLUSH();
	for (const char * c = log_buf; *c != 0; c++) {
		if (*c == '\n') {
			shell_put_char('\r');
			shell_put_char('\n');
		} else {
			shell_put_char(*c);
		}
	}
}

#endif

#if defined(STM32L475_NO_OS) || defined(STM32F407_NO_OS)
#include "usart.h"
#include "delay.h"

static char cpos = 0;
static int len = 0;

char shell_get_char() {
	//return getchar();
	while ((USART_RX_STA & 0x8000) == 0)
	{
		delay_ms(10);
	}

	if (len == 0) {
		len = USART_RX_STA & 0x3fff;
		USART_RX_BUF[len] = '\n';
	}

	char c = USART_RX_BUF[cpos++];
	// get data
	if (cpos > len) {
		USART_RX_STA = 0;
		cpos = 0;
		len = 0;
	}
	return c;
}

void shell_put_char(char c) {
	uart_putchar(c);
}

void log_print_porting(const char *fmt, va_list *args) {
	memset(log_buf, 0, sizeof(log_buf));
	vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, *args);
	printf("%s", log_buf);
	//vprintf(fmt, *args);
}

#endif

#ifdef ZEPHYR_OS
#include <zephyr.h>
#include <misc/printk.h>
#include <console.h>

#pragma message("USE ZEPHYR_OS print-----------------------")

char shell_get_char() {
	return console_getchar();
}

void shell_put_char(char c) {
	console_putchar(c);
}

void log_print_porting(const char *fmt, va_list *args) {
	vprintk(fmt, *args);
}
#endif

#ifdef NUTTX_OS
#include <stdio.h>

char shell_get_char() {
	return getchar();
}

void shell_put_char(char c) {
	putchar(c);
}

void log_print_porting(const char *fmt, va_list *args) {
	vprintf(fmt, *args);
}

#endif
