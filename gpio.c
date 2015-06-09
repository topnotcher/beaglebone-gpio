#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include "gpio.h"

static int gpio_mmap(int fd);
static int gpio_clock_enable(int fd);
static int gpio_init(void);
static void gpio_munmap(void);
static void verify_gpio_map(void);
static void init_leds(void);
static void blink_leds(void);
static void kill_stupid_kernel_led_driver(void);
static void sigint_handler(int s);
static void leds_off(void);

#define LED_GPIO_BANK 1
#define LED_USER0_BIT 21

GPIO_t  *gpio[GPIO_NUM_BANKS];
volatile sig_atomic_t running = 1;

int main (void) {
	verify_gpio_map();
	int ret = gpio_init();

	if (ret) {
		fprintf(stderr, "Failed to initialize GPIO\n");
		return ret;
	}

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

static int gpio_mmap(int fd) {
	for (int i = 0; i < GPIO_NUM_BANKS; ++i) {
		gpio[i] = mmap(NULL, GPIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_ADDRS[i]);

		if (gpio[i] == MAP_FAILED)
			return errno;
	}

	return 0;
}

static void gpio_munmap() {
	for (int i = 0; i < GPIO_NUM_BANKS; ++i)
		munmap(gpio[i], GPIO_MEM_SIZE);
}

static int gpio_clock_enable(int fd) {
	uint8_t *cm = mmap(NULL, CM_PER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CM_OFFSET);

	if (cm == MAP_FAILED)
		return errno;

	for (int i = 0; i < GPIO_NUM_BANKS; ++i) {
		// enable clocks to GPIO and spin while not fully functional
		volatile uint32_t *clkctrl= (uint32_t*)(cm+GPIO_CLKCTRL[i]);
		*clkctrl &= ~GPIO_CLKCTRL_MODULEMODE_BM;
		*clkctrl |= GPIO_CLKCTRL_MODULEMODE_ENABLE;

		while ( ((*clkctrl) & GPIO_CLKCTRL_IDLEST_BM) != GPIO_CLKCTRL_IDLEST_FUNCTIONAL ) ;

		// ungate clock (page 4892)
		gpio[i]->CTRL &= (~0x01) | (~0x06);
	}

	munmap(cm, CM_PER_SIZE);
	return 0;
}

static int gpio_init(void) {
	int fd = open("/dev/mem", O_RDWR);
	int status = 0;
	int ret;

	if (fd == -1) {
		fprintf(stderr, "failed to open /dev/mem: %s\n", strerror(errno));
		return errno;
	}

	ret = gpio_clock_enable(fd);
	if (ret) {
		fprintf(stderr, "failed to enable gpio clocks: %s\n", strerror(errno));
		status = errno;
		goto ret_close;
	}

	ret = gpio_mmap(fd);
	if (ret) {
		fprintf(stderr, "failed to mmap gpio banks: %s\n", strerror(errno));
		status = errno;
		goto ret_close;
	}

ret_close:
	close(fd);
	return status;
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
		gpio[LED_GPIO_BANK]->OE &= ~(1<<(LED_USER0_BIT+i));
		gpio[LED_GPIO_BANK]->CLEARDATAOUT = 1<<(LED_USER0_BIT+i);
	}
}

static void blink_leds() {
	for (int i = 0; i < 4; ++i) {
		gpio[LED_GPIO_BANK]->SETDATAOUT |= 1<<(LED_USER0_BIT+i);
		sleep(1);
		gpio[LED_GPIO_BANK]->CLEARDATAOUT = 1<<(LED_USER0_BIT+i);
	}
}

static void leds_off() {
	for (int i = 0; i < 4; ++i)
		gpio[LED_GPIO_BANK]->CLEARDATAOUT = 1<<(LED_USER0_BIT+i);
}

static void sigint_handler(int s) {
	running = 0;
}
