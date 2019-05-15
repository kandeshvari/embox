/**
 * @file
 *
 * @date 09 may 2019
 * @author: Dmitry Kurbatov <dk@dimcha.ru>
 */

#include <compiler.h>
#include <drivers/diag.h>

#include <framework/mod/options.h>
//#include <module/embox/driver/serial/sifive_uart.h>

#include <embox/unit.h>

EMBOX_UNIT_INIT(sifive_uart_init);

#include <drivers/serial/uart_device.h>
#include <drivers/serial/diag_serial.h>



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

#define UART_REG(x)                                                     \
        unsigned char x;                                                \
        unsigned char postpad_##x[3];

#define UART_REG_INT32(x)                                                     \
	uint32_t x;

struct com {
        UART_REG(txfifo);          /* 0 */
        UART_REG(rxfifo);          /* 1 */
        UART_REG(txctrl);          /* 2 */
        UART_REG(rxctrl);          /* 3 */
        UART_REG(ie);  	           /* 4 */
        UART_REG(ip);              /* 5 */
        UART_REG_INT32(div);       /* 6 */
};

#define COM ((volatile struct com *)UART_BASE)
#define COM_TXF (COM->txfifo)
#define COM_RXF (COM->rxfifo)
#define COM_DIV (COM->div)
#define COM_TXC (COM->txctrl)
#define COM_RXC (COM->rxctrl)
#define COM_IE (COM->ie)

static int sifive_uart_setup(struct uart *dev, const struct uart_params *params) {
// static int sifive_uart_setup(const struct diag *dev) {
	// static volatile int *uart;
	// uart = (int *)dev->base_addr;
	COM_DIV = UART_CLOCK_FREQ / UART_BAUD_RATE - 1;
	COM_TXC = UART_TXEN;
	COM_RXC = UART_RXEN;
	COM_IE = 0;

	// uart[UART_REG_DIV] = UART_CLOCK_FREQ / UART_BAUD_RATE - 1;
	// uart[UART_REG_TXCTRL] = UART_TXEN;
	// uart[UART_REG_RXCTRL] = UART_RXEN;
	// uart[UART_REG_IE] = 0;

	 return 0;
}

static int sifive_uart_getc(struct uart *dev) {
//static char sifive_uart_getc(const struct diag *dev) {
	int ch = COM_RXF;
		// k(int *)dev->base_addr[UART_REG_RXFIFO];
	if (ch < 0) return -1;
	return ch;
}

static int sifive_uart_putc(struct uart *dev, int ch) {
//static void sifive_uart_putc(const struct diag *dev, char ch) {
	while (COM_TXF < 0);
	COM_TXF = ch & 0xff;
	// while ((int *)(dev->base_addr)[UART_REG_TXFIFO] < 0);
	// (int *)dev->base_addr[UART_REG_TXFIFO] = ch & 0xff;
	return 0;
}


static const struct uart_ops sifive_uart_uart_ops = {
	.uart_getc = sifive_uart_getc,
	.uart_putc = sifive_uart_putc,
//		.uart_hasrx = sifive_uart_has_symbol,
	.uart_setup = sifive_uart_setup,
};

static const struct uart_params uart_defparams = {
	.baud_rate = UART_BAUD_RATE,
	.parity = 0,
	.n_stop = 1,
	.n_bits = 8,
	.irq = true,
};

static struct uart uart0 = {
	.uart_ops = &sifive_uart_uart_ops,
//		.irq_num = IRQ_NUM,
	.base_addr = UART_BASE,
};


/* uart routines */
static int sifive_uart_init(void) {
	return uart_register(&uart0, &uart_defparams);
}

/* common driver part */

static const struct uart_params uart_diag_params = {
		.baud_rate = UART_BAUD_RATE,
		.parity = 0,
		.n_stop = 1,
		.n_bits = 8,
		.irq = false,
};

DIAG_SERIAL_DEF(&uart0, &uart_diag_params);


// static const struct periph_memory_desc sifive_uart_mem = {
	// .start = UART_BASE,
	// .len   = 0x1000,
// };
//
// PERIPH_MEMORY_DEFINE(sifive_uart_mem);
//

//DIAG_OPS_DECLARE(
//	.init = sifive_uart_setup,
//	.putc = sifive_uart_putc,
//	.getc = sifive_uart_getc,
//);
