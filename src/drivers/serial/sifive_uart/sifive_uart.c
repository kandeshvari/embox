/**
 * @file
 *
 * @data 04 aug 2015
 * @author: Anton Bondarev
 */
//#include <stdint.h>
//
//#include <drivers/common/memory.h>
//#include <drivers/diag.h>
//#include <drivers/serial/uart_device.h>
//#include <drivers/serial/diag_serial.h>

#include <compiler.h>
#include <drivers/diag.h>

#include <framework/mod/options.h>
#include <module/embox/driver/serial/sifive_uart.h>

#include <framework/mod/options.h>
#include <embox/unit.h>

//EMBOX_UNIT_INIT(uart_init);

#define UART_BASE 	OPTION_GET(NUMBER,base_addr)
#define UART_CLOCK_FREQ OPTION_GET(NUMBER,clock_freq)
#define UART_BAUD_RATE	OPTION_GET(NUMBER,baud_rate)

/* UART Registers */
#define UART_REG_TXFIFO 0
#define UART_REG_RXFIFO 1
#define UART_REG_TXCTRL 2
#define UART_REG_RXCTRL 3
#define UART_REG_IE     4
#define UART_REG_IP     5
#define UART_REG_DIV    6

/* TXCTRL register */
#define UART_TXEN       1
#define UART_TXSTOP     2

/* RXCTRL register */
#define UART_RXEN       1

/* IP register */
#define UART_IP_TXWM    1
#define UART_IP_RXWM    2

//static int sifive_uart_setup(struct uart *dev, const struct uart_params *params) {
//	if (params->irq) {
//		REG_ORIN(UART_IMSC, IMSC_RXIM);
//	}
//
//	return 0;
//}
//
//static int sifive_uart_putc(struct uart *dev, int ch) {
//	while (REG_LOAD(UART_FR) & FR_TXFF) ;
//
//	REG_STORE(UART_DR, (uint32_t)ch);
//
//	return 0;
//}
//
//static int sifive_uart_getc(struct uart *dev) {
//	return REG_LOAD(UART_DR);
//}
//
//static int sifive_uart_has_symbol(struct uart *dev) {
//	return !(REG_LOAD(UART_FR) & FR_RXFE);
//}

/* uart routines */
static volatile int *uart;

//static void sifive_uart_setup(struct uart *dev, const struct uart_params *params) {
static int sifive_uart_setup(const struct diag *dev) {
	uart = (int *)UART_BASE;
	uart[UART_REG_DIV] = UART_CLOCK_FREQ / UART_BAUD_RATE - 1;
	uart[UART_REG_TXCTRL] = UART_TXEN;
	uart[UART_REG_RXCTRL] = UART_RXEN;
	uart[UART_REG_IE] = 0;

	return 0;
}

//static int sifive_uart_getc(struct uart *dev) {
static char sifive_uart_getc(const struct diag *dev) {
	int ch = uart[UART_REG_RXFIFO];
	if (ch < 0) return -1;
	return ch;
}

//static int sifive_uart_putc(struct uart *dev, int ch) {
static void sifive_uart_putc(const struct diag *dev, char ch) {
	while (uart[UART_REG_TXFIFO] < 0);
	uart[UART_REG_TXFIFO] = ch & 0xff;
}


/* common driver part */
/*
static const struct uart_ops sifive_uart_uart_ops = {
		.uart_getc = sifive_uart_getc,
		.uart_putc = sifive_uart_putc,
//		.uart_hasrx = sifive_uart_has_symbol,
		.uart_setup = sifive_uart_setup,
};

static struct uart uart0 = {
		.uart_ops = &sifive_uart_uart_ops,
//		.irq_num = IRQ_NUM,
		.base_addr = UART_BASE,
};

static const struct uart_params uart_defparams = {
		.baud_rate = UART_BAUD_RATE,
		.parity = 0,
		.n_stop = 1,
		.n_bits = 8,
		.irq = true,
};

static const struct uart_params uart_diag_params = {
		.baud_rate = UART_BAUD_RATE,
		.parity = 0,
		.n_stop = 1,
		.n_bits = 8,
		.irq = false,
};

const struct uart_diag DIAG_IMPL_NAME(__EMBUILD_MOD__) = {
		.diag = {
			.ops = &uart_diag_ops,
		},
		.uart = &uart0,
		.params = &uart_diag_params,
};

static int uart_init(void) {
	return uart_register(&uart0, &uart_defparams);
}

static const struct periph_memory_desc sifive_uart_mem = {
	.start = UART_BASE,
	.len   = 0x48,
};
*/
//PERIPH_MEMORY_DEFINE(sifive_uart_mem);


DIAG_OPS_DECLARE(
	.init = sifive_uart_setup,
	.putc = sifive_uart_putc,
	.getc = sifive_uart_getc,
);
