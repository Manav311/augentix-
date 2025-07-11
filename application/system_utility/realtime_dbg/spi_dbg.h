#include <stdio.h>
#include <stdint.h> //uintX_t
#include <sys/mman.h> //mmap
#include <fcntl.h> //open
#include <unistd.h> //close
#include <errno.h> //errno

#define FIFO_LEVEL 0x20 //Transmit FIFO Level Register, offset = 0x20
#define TRIGGER_SPI 0x10 //Slave Enable Register,which also triggers data transmission, offset = 0x10
#define LOG 0x60 //Data Register, offset = 0x60

#define SPI_FIFO_SIZE 8

static volatile uint32_t *spi_address = NULL;
static volatile uint32_t *gicc_address = NULL;
static int fd = -1;

static inline uint32_t get_reg_value(unsigned int offset)
{
	return *(volatile uint32_t *)(spi_address + (offset / 4));
}

static inline void set_reg_value(unsigned int offset, unsigned int value)
{
	*(volatile uint32_t *)(spi_address + (offset / 4)) = value;
}

/** 
 * spi_dbg_log_event - write event log to spi debugger
 * @event_id: number of event type
 * 
 * event_id ranges from 0 to 127.
 */
static inline uint32_t spi_dbg_log_event(uint8_t event_id)
{
	if (fd < 0) {
		fd = open("/dev/spi_debugger", O_RDWR | O_CLOEXEC);
		if (fd < 0) {
			puts("Cannot open \"spi_debugger\"\n");
			return -errno;
		}

		spi_address = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (spi_address == MAP_FAILED) {
			puts("spi mmap failed\n");
			return -errno;
		}

		gicc_address = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 4096);
		if (gicc_address == MAP_FAILED) {
			puts("gicc mmap failed\n");
			return -errno;
		}
	}

	if (event_id > 127) {
		puts("event_id is not correct.\n");
		return -EINVAL;
	}
	/* Disable interrupt of local processor */
	*gicc_address = 0;

	/* Busy waiting until FIFO is not full*/
	while (get_reg_value(FIFO_LEVEL) > (uint32_t)(SPI_FIFO_SIZE - 1)) {
		set_reg_value(TRIGGER_SPI, 0xff);
	}

	/* The bit-7 is 0 which represents event log */
	set_reg_value(LOG, (event_id & ~(1 << 7)));
	set_reg_value(TRIGGER_SPI, 0xff);

	/* Enable interrupt of local processor */
	*gicc_address = 1;

	return 0;
}

/** 
 * spi_dbg_log_data - write data log to spi debugger
 * @data_id: number of data type
 * @len: length of data
 * @data: data of log
 * 
 * data_id ranges from 0 to 31.
 * len ranges from 1 to 4, which means data length.
 */
static inline uint32_t spi_dbg_log_data(uint8_t data_id, uint8_t len, uint8_t *data)
{
	int count = 0;

	if (fd < 0) {
		fd = open("/dev/spi_debugger", O_RDWR | O_CLOEXEC);
		if (fd < 0) {
			puts("Cannot open \"spi_debugger\"\n");
			return -errno;
		}

		spi_address = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (spi_address == MAP_FAILED) {
			puts("spi mmap failed\n");
			return -errno;
		}

		gicc_address = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 4096);
		if (gicc_address == MAP_FAILED) {
			puts("gicc mmap failed\n");
			return -errno;
		}
	}

	if (len < 1 || len > 4) {
		puts("Data length is not correct.\n");
		return -EINVAL;
	}

	if (data_id > 31) {
		puts("Data ID is not correct.\n");
		return -EINVAL;
	}

	/* Disable interrupt of local processor */
	*gicc_address = 0;

	/* Busy waiting until FIFO is not full*/
	while (get_reg_value(FIFO_LEVEL) > (uint32_t)(SPI_FIFO_SIZE - 1)) {
		set_reg_value(TRIGGER_SPI, 0xff);
	}

	/**
	 * The bit-7 is 1 which represents data log.
	 * The first byte tells the information about data type and data length. 
	 * The data format is "0x1TTTTTKK."
	 * T=data_id, K=data length.
	 */
	set_reg_value(LOG, (1 << 7) | (data_id << 2) | (len - 1));
	set_reg_value(TRIGGER_SPI, 0xff);

	/* Transmit the rest of data accroding to data length.*/
	for (; count <= (len - 1); count++) {
		while (get_reg_value(FIFO_LEVEL) > (uint32_t)(SPI_FIFO_SIZE - 1)) {
			set_reg_value(TRIGGER_SPI, 0xff);
		}

		set_reg_value(LOG, data[count]);
		set_reg_value(TRIGGER_SPI, 0xff);
	}

	/* Enable interrupt of local processor */
	*gicc_address = 1;

	return 0;
}
