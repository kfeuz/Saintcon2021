/*
 * flash.h
 *
 * Created: 8/18/2021 9:30:11 PM
 *  Author: Professor Plum
 */ 
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef FLASH_H_
#define FLASH_H_

#define FLASH_ID			0x94
#define FLASH_WRITE_EN		0x06
#define FLASH_WRITE_DE		0x04
#define FLASH_STATUS		0x05
#define FLASH_ERROR			0x07
#define FLASH_QPI_EN		0x38
#define FLASH_QPI_DE		0xF5
#define FLASH_READ			0x6B
#define FLASH_WRITE_PAGE	0x32
#define FLASH_ERASE_ALL		0xC7
#define FLASH_ERASE_64K		0xD8
#define FLASH_ERASE_32K		0x52
#define FLASH_ERASE_4K		0x20
#define FLASH_SLEEP			0xB9
#define FLASH_WAKE			0xAB
#define FLASH_WRITE_REG		0x01*/

//Prototype 2
#define FLASH_ID			0x9F
#define FLASH_WRITE_EN		0x06
#define FLASH_WRITE_DE		0x04
#define FLASH_STATUS		0x05
#define FLASH_ERROR			0x07
#define FLASH_QPI_EN		0x38
#define FLASH_QPI_DE		0xFF
#define FLASH_READ			0xEB
#define FLASH_WRITE_PAGE	0x02
#define FLASH_ERASE_ALL		0x60
#define FLASH_ERASE_BLOCK	0xD8
#define FLASH_ERASE_HBLOCK	0x52
#define FLASH_ERASE_SEC		0x20
#define FLASH_SLEEP			0xB9
#define FLASH_WAKE			0xAB
#define FLASH_WRITE_REG		0x01

void flash_init();
void flash_read_id(uint8_t *id);
void flash_read_status_2(uint8_t *id);
void flash_enable_write(bool enable);
void flash_erase_all();
void flash_erase_4k(uint32_t addr);
void flash_erase_32k(uint32_t addr);
void flash_read(uint32_t addr, void *out, size_t len);
void flash_write(uint32_t addr, void *buf, size_t len);

// Sample vcard
/*
BEGIN:VCARD
VERSION:2.1
N:Wiley;Bob;;;
FN:Bob Wiley
EMAIL;WORK:bob@home.com
TITLE:Your Title
END:VCARD
*/

void flash_save_vcard(char* vcard);
void flash_read_vcard(char* vcard);

#define BIRD_IMG             	 ((uint32_t)0X000000)
#define MACHINE_BKG_IMG      	 ((uint32_t)0X006400)
#define MACHINE_IMG          	 ((uint32_t)0X022800)
#define BUILD_IMG            	 ((uint32_t)0X255000)
#define PARTS_IMG            	 ((uint32_t)0X278C00)
#define MENU_IMG             	 ((uint32_t)0X27AC00)

#endif /* FLASH_H_ */