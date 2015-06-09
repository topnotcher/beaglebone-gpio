#include <stdint.h>
#ifndef _GPIOMAP_H
#define _GPIOMAP_H

// see: http://www.ti.com/lit/ug/spruh73l/spruh73l.pdf page 4877
typedef struct {
	uint32_t REVISION;
	uint8_t reserved1[0x10-0x04];
	uint32_t SYSCONFIG;
	uint8_t reserved2[0x20-0x14];
	uint32_t EOI;
	uint32_t IRQSTATUS_RAW[2];
	uint32_t IRQSTATUS[2];
	uint32_t IRQSTATUS_SET[2];
	uint32_t IRQSTATUS_CLR[2];
	uint32_t IRQWAKEN[2];
	uint8_t reserved3[0x114 - 0x4C];
	uint32_t SYSSTATUS;
	uint8_t reserved4[0x130 - 0x118];
	uint32_t CTRL;
	uint32_t OE;
	uint32_t DATAIN;
	uint32_t DATAOUT;
	uint32_t LEVELDETECT[2];
	uint32_t RISINGDETECT;
	uint32_t FALLINGDETECT;
	uint32_t DEBOUNCEENABLE;
	uint32_t DEBOUNCINGTIME;
	uint8_t reserved5[0x190-0x158];
	uint32_t CLEARDATAOUT;
	uint32_t SETDATAOUT;
} __attribute__((packed)) GPIO_t;

#define GPIO_NUM_BANKS 4

// Size to mmap per gpio bank - 1 4KiB page
#define GPIO_MEM_SIZE 0x1000

// CM_PER, page 178
// CM_WKUP immediately follows CM_PER, so one mmap will suffice
#define CM_OFFSET 0x44E00000

// size to mmap()
#define CM_PER_SIZE sysconf(_SC_PAGESIZE)
#define CM_WKUP_OFFSET 0x0100

// offsets relative to CM_OFFSET
#define CM_PER_GPIO1_CLKCTRL 0xAC
#define CM_PER_GPIO2_CLKCTRL 0xB0
#define CM_PER_GPIO3_CLKCTRL 0xB4

// offset relative to CM_OFFSET
#define CM_WKUP_GPIO0_CLKCTRL (CM_WKUP_OFFSET + 0x08)

// bit positions: page 1227, 1192
#define GPIO_CLKCTRL_MODULEMODE_BP 0
#define GPIO_CLKCTRL_MODULEMODE_BM (0x03<<GPIO_CLKCTRL_IDLEST_BP)
#define GPIO_CLKCTRL_IDLEST_BP 16
#define GPIO_CLKCTRL_IDLEST_BM (0x03<<GPIO_CLKCTRL_IDLEST_BP)

#define GPIO_CLKCTRL_MODULEMODE_ENABLE (0x02 << GPIO_CLKCTRL_MODULEMODE_BP)
#define GPIO_CLKCTRL_IDLEST_FUNCTIONAL (0x00 << GPIO_CLKCTRL_IDLEST_BP)

extern GPIO_t* gpio[GPIO_NUM_BANKS];
static uint32_t GPIO_ADDRS[GPIO_NUM_BANKS] = {0x44E07000, 0x4804C000, 0x481AC000, 0x481AE000};

//offsets of CLKCTRL registers relative to CM_OFFSET.
static uint32_t GPIO_CLKCTRL[GPIO_NUM_BANKS] = {
	CM_WKUP_GPIO0_CLKCTRL,
	CM_PER_GPIO1_CLKCTRL,
	CM_PER_GPIO2_CLKCTRL,
	CM_PER_GPIO3_CLKCTRL
};

#endif
