#ifdef DEV_STM32L475

#include "stm32l4xx.h"
#include "include/flash_api.h"
#include <string.h>
#include "include/utils.h"

#define FLASH_START_ADRESS     ((uint32_t)0x08000000)
#define FLASH_END_ADDRESS      ((uint32_t)(0x08000000 + 512 * 1024))
#define FLASH_PAGE_NBPERBANK   128
#define FLASH_BANK_NUMBER      2

static int flash_get_blank(uint32_t addr);
static int write_directly(uint32_t addr, uint64_t *buf, int size);
static int earse_page(uint32_t secid);


uint32_t flash_read_word(uint32_t faddr) {
	return *(volatile uint32_t*) faddr;
}

void flash_port_read(uint32_t addr, uint8_t *buf, int size) {
	//uint8_t *buf_8 = (uint8_t *) buf;
//	uint8_t last = *(vu8*) (addr + size - 1);
//	buf_8[size - 1] = last;
	for (int i = 0; i < size; i++) {
		buf[i] = *(volatile uint8_t*) (addr + i);
	}
}

static void flash_read32(uint32_t addr, uint32_t *buf, int size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		buf[i] = *(volatile uint32_t*) addr;
		addr += 4;
	}
	//printf("exit: %d 0x%p 0x%p\n", i, buf + i, (void*)addr);
}

#define STM_SECTOR_SIZE	2048
static __align(8) uint64_t STMFLASH_BUF[STM_SECTOR_SIZE / 8]; // 2k
int flash_port_write(uint32_t faddr, uint8_t *buf, int size) {
	int ret = 0;
	
	// align 8
	uint32_t start_addr = faddr & 0xFFFFFFF8;
	uint32_t end_addr = (faddr + size + 8) & 0xFFFFFFF8;
	
	if(start_addr < FLASH_START_ADRESS || end_addr > FLASH_END_ADDRESS) {
		DEBUG_LOG("ERROR: write outrange flash size! addr is (0x%p)\n", (void*)(faddr + size));
		return -1;
	}
	uint32_t offaddr   = start_addr - FLASH_START_ADRESS;
	uint32_t secid     = offaddr / STM_SECTOR_SIZE;
	uint32_t secoff    = (offaddr % STM_SECTOR_SIZE) / 8;
	uint32_t sec_remain = STM_SECTOR_SIZE / 8 - secoff; // sector remain size
	uint32_t total_dw = (end_addr - start_addr) / 8; // total double world to write
	
	// can write in current sector
	uint32_t write_dw = total_dw <= sec_remain ? total_dw : sec_remain;
	HAL_FLASH_Unlock();
	int start_pos = secoff * 8 + (faddr - start_addr); // padding + offset

	//printf("STMFLASH_BUF: (0x%p)\n", (void *)STMFLASH_BUF);
	
	/*
	STMFLASH_BUF align(8)
	buffer start at any pos with any len
	         |       STMFLASH_BUF       |
	         |----<---|--------|----->--|
	left padding  |   buffer         | right padding  
	*/
	while (1) {
		// copy page
		// flash_port_read(secid * STM_SECTOR_SIZE + FLASH_START_ADRESS, (uint8_t*) STMFLASH_BUF, STM_SECTOR_SIZE);
		flash_read32(secid * STM_SECTOR_SIZE + FLASH_START_ADRESS, (uint32_t*)STMFLASH_BUF, STM_SECTOR_SIZE / 4);
		// left padding
		int write_bytes = write_dw * 8 - start_pos;
		// right padding
		if (size < write_bytes) write_bytes = size;
		for (int i = 0; i < write_bytes; i++) {
			((uint8_t *)STMFLASH_BUF)[start_pos + i] = buf[i];
		}
		// earse
		if (earse_page(secid) != 0) {
			ret = -1;
			break;
		}
		
		// write back
		if (write_directly(start_addr, STMFLASH_BUF, write_dw) != 0) {
			ret = -1;
			break;
		}

		if (write_dw == total_dw) {
			// finish
			break;
		} else {
			// need more sections
			secid++;
			start_pos = 0;
			size -= write_bytes;
			buf += write_bytes;
			start_addr += write_dw * 8;
			total_dw = (size + 8) / 8;
			if (size > STM_SECTOR_SIZE) {
				write_dw = STM_SECTOR_SIZE / 8;
			} else {
				write_dw = (size + 8) / 8;
			}
		}
	}
	HAL_FLASH_Lock();
	return ret;
}

static int flash_get_blank(uint32_t addr) {
	if(addr < (FLASH_BASE + FLASH_BANK_SIZE)){
		return FLASH_BANK_1;
	} else {
		return FLASH_BANK_2;
	}
}

static int earse_page(uint32_t secid) {
	uint32_t addr = secid * STM_SECTOR_SIZE + FLASH_START_ADRESS;
	FLASH_EraseInitTypeDef FlashEraseInit;
	FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashEraseInit.Banks = flash_get_blank(addr);
	FlashEraseInit.Page = secid;
	FlashEraseInit.NbPages = 1;

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);
	
	//HAL_FLASH_Unlock();
	uint32_t err;
	int ret = 0;
	if(HAL_FLASHEx_Erase(&FlashEraseInit, &err) != HAL_OK) {
		DEBUG_LOG("ERROR: earse flash failed: (0x%p)\n", (void *) addr);
		ret = -1;
	}
	return ret;
}

static int write_directly(uint32_t addr, uint64_t *buf, int size) {
	//__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);
	int ret = 0;
	for (int i = 0; i < size; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, buf[i]);
		uint64_t read_data = *((__IO uint64_t *) addr);
		if (read_data != buf[i]) {
			DEBUG_LOG("ERROR: write flash failed: (0x%p)\n", (void *) addr);
			ret = -1;
			break;
		}
		addr += 8;
	}
	return ret;
}

#include "include/profiling.h"
#define WRITE_BLOCK 2048
#define READ_TIMES 1000
char data[WRITE_BLOCK];

static void test_sram_speed() {
	uint32_t write_addr = 0x20002000;
	int start = get_start_tick();
	for (int i = 0; i < 10; i++) {
		uint32_t addr = write_addr + i * WRITE_BLOCK;
		memcpy((void *) addr, data, WRITE_BLOCK);
	}
	int end = get_cur_tick();
	
	DEBUG_LOG("test_sram_speed: %f\n", cycles2us((end - start) / 10));
}

static void test_flash_speed() {
	uint32_t write_addr = 0x08030000;
	int start = get_start_tick();
	for (int i = 0; i < 10; i++) {
		uint32_t addr = write_addr + i * WRITE_BLOCK;
		//memcpy((void *) addr, data, WRITE_BLOCK);
		flash_port_write(addr, (uint8_t *) data, WRITE_BLOCK);
	}
	int end = get_cur_tick();
	DEBUG_LOG("test_flash_speed: %f\n", cycles2us((end - start) / 10));
}


void test_flash_write_speed() {
	profile_add_event("rw-speed");
	test_sram_speed();
	test_flash_speed();
}

#endif
