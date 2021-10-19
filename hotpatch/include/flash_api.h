#ifndef FLASH_API_H_
#define FLASH_API_H_
#include <stdint.h>

uint32_t flash_read_word(uint32_t faddr);
void flash_port_read(uint32_t faddr, uint8_t *buf, int size);
int flash_port_write(uint32_t faddr, uint8_t *buf, int size);

void test_flash_write_speed();
#endif