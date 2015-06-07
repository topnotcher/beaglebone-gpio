#include <stdint.h>
#ifndef _GPIOMAP_H
#define _GPIOMAP_H

// see: http://www.ti.com/lit/ug/spruh73l/spruh73l.pdf page 4877
typedef struct {
	uint32_t REVISION; //addr: 0x00
	uint8_t reserved1[0x10-0x04];
	uint32_t SYSCONFIG; //addr: 0x10
	uint8_t reserved2[0x20-0x14];
	uint32_t EOI; //addr: 0x20
	uint32_t IRQSTATUS_RAW[2]; //addr:0x24, 0x28
	uint32_t IRQSTATUS[2]; //addr: 0x2C, 0x30
	uint32_t IRQSTATUS_SET[2]; //addr: 0x34, 0x38
	uint32_t IRQSTATUS_CLR[2]; //addr: 0x3C, 0x40
	uint32_t IRQWAKEN[2]; //addr: 0x44, 0x48
	uint8_t reserved3[0x114 - 0x4C];
	uint32_t SYSSTATUS; //addr: 0x114
	uint8_t reserved4[0x130 - 0x118];
	uint32_t CTRL; //addr: 0x130
	uint32_t OE; //addr: 0x134
	uint32_t DATAIN; //addr: 0x138
	uint32_t DATAOUT; //addr: 0x13C
	uint32_t LEVELDETECT[2]; //addr: 0x140, 0x144
	uint32_t RISINGDETECT; //addr: 0x148
	uint32_t FALLINGDETECT; //addr: 0x14C
	uint32_t DEBOUNCEENABLE; //addr: 0x150	
	uint32_t DEBOUNCINGTIME; //addr: 0x154
	uint8_t reserved5[0x190-0x158];
	uint32_t CLEARDATAOUT; //addr: 0x190
	uint32_t SETDATAOUT; //addr: 0x194
} __attribute__((packed)) GPIO_t;

#define NUM_GPIO_BANKS 4
#define GPIO_MEM_SIZE 0x1000 // 1 4KiB page

// CM_PER, page 178
#define CM_OFFSET 0x44E00000

#define CM_PER_SIZE 0x0100 //1 KiB
#define CM_PER_MEM_SIZE getpagesize()

// offsets relative to CM_OFFSET
#define CM_PER_GPIO1_CLKCTRL 0xAC
#define CM_PER_GPIO2_CLKCTRL 0xB0
#define CM_PER_GPIO3_CLKCTRL 0xB4

// CM_WK immediately follows CM_PER. 
// offsets relative to CM_PER
#define CM_WKUP_GPIO0_CLKCTRL (CM_PER_SIZE + 0x08)

// bit positions: page 1227, 1192
#define GPIO_CLKCTRL_MODULEMODE_BP 0
#define GPIO_CLKCTRL_MODULEMODE_BM (0x03<<GPIO_CLKCTRL_IDLEST_BP)
#define GPIO_CLKCTRL_IDLEST_BP 16
#define GPIO_CLKCTRL_IDLEST_BM (0x03<<GPIO_CLKCTRL_IDLEST_BP)

// bit masks: page 1227, 1192
#define GPIO_CLKCTRL_MODULEMODE_ENABLE (0x02 << GPIO_CLKCTRL_MODULEMODE_BP)
#define GPIO_CLKCTRL_IDLEST_FUNCTIONAL (0x00 << GPIO_CLKCTRL_IDLEST_BP)


extern GPIO_t* gpio[NUM_GPIO_BANKS]; 
static uint32_t GPIO_ADDRS[NUM_GPIO_BANKS] = {0x44E07000, 0x4804C000, 0x481AC000, 0x481AE000};
static uint32_t GPIO_CLKCTRL[NUM_GPIO_BANKS] = {CM_WKUP_GPIO0_CLKCTRL, CM_PER_GPIO1_CLKCTRL, CM_PER_GPIO2_CLKCTRL, CM_PER_GPIO3_CLKCTRL};

#endif
