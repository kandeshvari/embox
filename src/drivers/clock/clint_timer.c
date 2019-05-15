/**
 * @file
 * @brief RISCV: SiFive CLINT timer
 *
 * @date 15.05.2019
 * @author Dmitry Kubatov
 */

#include <stdint.h>
#include <errno.h>
#include <util/array.h>
#include <unistd.h>
#include <stdio.h>

#include <kernel/time/clock_source.h>
#include <kernel/time/ktime.h>
#include <hal/system.h>

#include <asm/regs.h>

#include <embox/unit.h>

#include <kernel/printk.h>

static int clint_timer_init(void);

/* Read Time Stamp Counter Register */
static inline unsigned long long rdtsc(void) {
	return read_csr(cycle);
	// unsigned n;
	// __asm__ __volatile__ (
		// "rd %0"
	// : "=r" (n));
//
	// return n;

	// unsigned hi, lo, tmp;
	// __asm__ __volatile__ (
		// "1:\n"
		// "rdtimeh %0\n"
		// "rdtime %1\n"
		// "rdtimeh %2\n"
		// "bne %0, %2, 1b"
			// : "=&r" (hi), "=&r" (lo), "=&r" (tmp));
	// return ((unsigned long long)hi << 32) | lo;

	// __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	// return (((unsigned long long) lo) | (((unsigned long long) hi) << 32));
}

static struct time_counter_device clint_timer = {
	.read = rdtsc
};

static struct clock_source clint_timer_clock_source = {
	.name = "TSC",
	.event_device = NULL,
	.counter_device = &clint_timer,
	.read = clock_source_read /* attach default read function */
};

static int clint_timer_init(void) {
	clint_timer.cycle_hz = 10000000;
	printk("CPU frequency: %u\n", clint_timer.cycle_hz);
	clock_source_register(&clint_timer_clock_source);
	return ENOERR;
}

EMBOX_UNIT_INIT(clint_timer_init);
