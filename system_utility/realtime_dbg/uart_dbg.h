#include <stdio.h>
#include <stdint.h> //uintX_t
#include <sys/mman.h> //mmap
#include <fcntl.h> //open
#include <unistd.h> //close
#include <errno.h> //errno

#define UART_THR_OFFSET 0x00
#define UART_LSR_OFFSET 0x14
//this is used to get g_wait_fifo setting in kernel space
#define UART_SCR_OFFSET 0x1C
#define UART_LSR__THRE__UMASK (1 << 5)
#define UART_FIFO_SIZE 32

static int g_fd = -1;
static int g_wait_fifo = -1;
static volatile uint32_t *g_uart_base = NULL;
static volatile uint32_t *g_gicc_base = NULL;

static inline uint32_t get_uart_value(uint32_t offset)
{
	return *(volatile uint32_t *)(g_uart_base + offset / 4);
}

static inline void set_uart_value(uint32_t offset, uint32_t value)
{
	*(volatile uint32_t *)(g_uart_base + offset / 4) = value;
}

static void wait_uart_tx_fifo_empty(void)
{
	uint32_t csr;
	if (g_wait_fifo) {
		do {
			csr = get_uart_value(UART_LSR_OFFSET);
		} while ((csr & UART_LSR__THRE__UMASK) == 0);
	}
}

/**
 * uart_dbg_log_event - write event log to uart debugger
 * @event_id: number of event type
 *
 * event_id ranges from 0 to 127.
 */
static inline int uart_dbg_log_event(uint8_t event_id)
{
	if (g_fd == -1) {
		g_fd = open("/dev/uart_debugger", O_RDWR | O_CLOEXEC);
		if (g_fd == -1) {
			puts("Cannot open uart_debugger\n");
			return -errno;
		}

		g_uart_base = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
		if (g_uart_base == MAP_FAILED) {
			puts("uart mmap failed\n");
			return -errno;
		}
		g_wait_fifo = get_uart_value(UART_SCR_OFFSET);

		g_gicc_base = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 4096);
		if (g_gicc_base == MAP_FAILED) {
			puts("gicc mmap failed\n");
			return -errno;
		}
	}

	if (event_id > 127) {
		puts("event_id is invalid.\n");
		return -EINVAL;
	}
	/* Disable interrupt of local processor */
	*g_gicc_base = 0x0;

	/* Busy waiting until TX FIFO is empty*/
	wait_uart_tx_fifo_empty();

	/* Transmit the rest of data accroding to data length.*/
	set_uart_value(UART_THR_OFFSET, event_id);

	/* Enable interrupt of local processor */
	*g_gicc_base = 1;

	return 0;
}

/**
 * uart_dbg_log_data - write data log to uart debugger
 * @data_id: number of data type
 * @len: length of data
 * @data: data of log
 *
 * data_id ranges from 0 to 31.
 * len ranges from 1 to 4, which means data length.
 */
static inline int uart_dbg_log_data(uint8_t data_id, uint8_t len, uint8_t *data)
{
	uint8_t i;
	if (g_fd == -1) {
		g_fd = open("/dev/uart_debugger", O_RDWR | O_CLOEXEC);
		if (g_fd == -1) {
			puts("Cannot open uart_debugger\n");
			return -errno;
		}

		g_uart_base = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
		if (g_uart_base == MAP_FAILED) {
			puts("uart mmap failed\n");
			return -errno;
		}
		g_wait_fifo = get_uart_value(UART_SCR_OFFSET);

		g_gicc_base = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 4096);
		if (g_gicc_base == MAP_FAILED) {
			puts("gicc mmap failed\n");
			return -errno;
		}
	}

	if (len == 0 || len > 4) {
		puts("Data length is invalid.\n");
		return -EINVAL;
	}

	if (data_id > 31) {
		puts("Data ID is invalid.\n");
		return -EINVAL;
	}

	/* Disable interrupt of local processor */
	*g_gicc_base = 0x0;

	/* Busy waiting until TX FIFO is empty*/
	wait_uart_tx_fifo_empty();

	/*
	* The first byte tells data type and data length.
	* The data format is "0x1TTTTTKK",T = data_id, K = data length.
	*/
	set_uart_value(UART_THR_OFFSET, (1 << 7) | (data_id << 2) | (len - 1));

	/* Transmit the rest of data accroding to data length.*/
	i = len;
	for (; i > 0; i--) {
		set_uart_value(UART_THR_OFFSET, *data);
		data++;
	}

	/* Enable interrupt of local processor */
	*g_gicc_base = 0x1;

	return 0;
}
