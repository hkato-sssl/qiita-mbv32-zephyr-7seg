#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "seven_seg_display.h"

static const struct device *gpio_anode = DEVICE_DT_GET(DT_NODELABEL(axi_gpio_7seg));
static const struct device *gpio_digit = DEVICE_DT_GET(DT_NODELABEL(axi_gpio_7seg_1));

static const uint8_t digit_table[] = {
	0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101,
	0b01111101, 0b00000111, 0b01111111, 0b01101111, 0b01110111, 0b01111100,
	0b00111001, 0b01011110, 0b01111001, 0b01110001,
};

static volatile uint8_t display_points;
static volatile uint16_t display_digits;

K_TIMER_DEFINE(seven_seg_timer, NULL, NULL);
K_MUTEX_DEFINE(seven_seg_mtx);

static void display_column(int col)
{
	uint8_t d;
	uint8_t points;
	uint16_t digits;
	uint32_t anode;

	k_mutex_lock(&seven_seg_mtx, K_FOREVER);
	digits = display_digits;
	points = display_points;
	k_mutex_unlock(&seven_seg_mtx);

	d = (digits >> (col * 4)) & 15;
	d = digit_table[d];

	if ((points >> col) & 1) {
		d |= 0x80;
	}

	anode = 1 << col;
	gpio_port_set_masked(gpio_anode, 0x0f, ~anode);
	gpio_port_set_masked(gpio_digit, 0xff, ~d);
}

static void seven_seg_thread_entry(void *p1, void *p2, void *p3)
{
	display_digits = 0;
	display_points = 0;
	k_timer_start(&seven_seg_timer, K_NO_WAIT, K_MSEC(10));

	for (;;) {
		for (int col = 0; col < 4; ++col) {
			k_timer_status_sync(&seven_seg_timer);
			display_column(col);
		}
	}
}
K_THREAD_DEFINE(seven_seg_thread, 1024, seven_seg_thread_entry, 0, 0, 0, 1, 0, 0);

void seven_seg_display(uint16_t digits, uint16_t points)
{
	k_mutex_lock(&seven_seg_mtx, K_FOREVER);
	display_digits = digits;
	display_points = points;
	k_mutex_unlock(&seven_seg_mtx);
}
