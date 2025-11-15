#include <zephyr/kernel.h>
#include <stdint.h>

#include "seven_seg_display.h"

/* Count-up thread and resources. */

K_TIMER_DEFINE(cyc_timer, NULL, NULL);

static void cyc_thread_entry(void *p1, void *p2, void *p3)
{
	uint16_t d = 0;

	seven_seg_display(d, 0);
	k_timer_start(&cyc_timer, K_MSEC(100), K_MSEC(5));

	for (;;) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 100; ++j) {
				k_timer_status_sync(&cyc_timer);
				seven_seg_display(d, (1 << i));
				++d;
			}
		}
	}
}
K_THREAD_DEFINE(cyc_thread, 1024, cyc_thread_entry, 0, 0, 0, 10, 0, 0);
