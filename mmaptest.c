#include <sys/mman.h>
#include <signal.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include "mmtest.h"

GPIO_t* gpio[NUM_GPIO_BANKS]; 

static void gpio_mmap(void);
static void gpio_munmap(void);
static void verify_gpio_map(void);
static void init_leds(void);
static void blink_leds(void);
static void kill_stupid_kernel_led_driver(void);
static void sigint_handler(int s);
static void leds_off(void);

volatile sig_atomic_t running = 1;

int main (void) {
	verify_gpio_map();
	gpio_mmap();
	init_leds();
	signal(SIGINT, sigint_handler);

	while (running)
		blink_leds();	
	
	leds_off();
	gpio_munmap();
	return 0;
}

static void verify_gpio_map() {
	assert( offsetof(GPIO_t, REVISION) == 0x00 );
	assert( offsetof(GPIO_t, SYSCONFIG) == 0x10 );
	assert( offsetof(GPIO_t, EOI) == 0x20 );
	assert( offsetof(GPIO_t, SYSSTATUS) == 0x114 );
	assert( offsetof(GPIO_t, CTRL) == 0x130 );
	assert( offsetof(GPIO_t, CLEARDATAOUT) == 0x190 );
}

static void gpio_mmap() {
	int fd = open("/dev/mem", O_RDWR);
	uint8_t *cm = mmap(NULL, CM_PER_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CM_OFFSET);

	for (int i = 0; i < NUM_GPIO_BANKS; ++i) {
		gpio[i] = mmap(NULL, GPIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_ADDRS[i]) ;
		assert(gpio[i] != MAP_FAILED);
		
		// enable clocks to GPIO and spin while not fully functional
		uint32_t *clkctrl= (uint32_t*)(cm+GPIO_CLKCTRL[i]);
		*clkctrl = (*clkctrl & (~GPIO_CLKCTRL_MODULEMODE_BM)) | GPIO_CLKCTRL_MODULEMODE_ENABLE;
		while ( ((*clkctrl) & GPIO_CLKCTRL_IDLEST_BM) != GPIO_CLKCTRL_IDLEST_FUNCTIONAL );

		// ungate clocks (page 4892)
		gpio[i]->CTRL &= (~0x01) | (~0x06);
	}
	
	munmap(cm, CM_PER_MEM_SIZE);
	close(fd);
}

static void gpio_munmap() {
	for (int i = 0; i < NUM_GPIO_BANKS; ++i)
		munmap(gpio[i], GPIO_MEM_SIZE);
}

static void kill_stupid_kernel_led_driver() {
	char path[] = "/sys/class/leds/beaglebone:green:usr_/trigger";
	char *led = strstr(path, "_");
	
	for (int i = 0; i < 4; ++i) {
		*led = 0x30+i;
		FILE *fd = fopen(path, "w");
		fprintf(fd, "none\n");
		fclose(fd);
	}
}

static void init_leds() {
	kill_stupid_kernel_led_driver();

	//usr0...3 = gpio1_21, 22, 23, 24
	for (int i = 0; i < 4; ++i) {
		gpio[1]->OE &= ~(1<<(21+i));
		gpio[1]->CLEARDATAOUT = 1<<(21+i);
	}
}

static void blink_leds() {
	for (int i = 0; i < 4; ++i) {
		gpio[1]->SETDATAOUT |= 1<<(21+i);
		sleep(1);
		gpio[1]->CLEARDATAOUT = 1<<(21+i);
	}	
}

static void leds_off() {
	for (int i = 0; i < 4; ++i)
		gpio[1]->CLEARDATAOUT = 1<<(21+i);
}

static void sigint_handler(int s) {
	running = 0;
}
