#ifndef THERMAL_PROTD_OV4689_H_
#define THERMAL_PROTD_OV4689_H_
#if CONFIG_THERMAL_PROTECTION_I2C_CHIP == OV4689

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define OV4689_I2C_BUF_SIZE 3
#define OV4689_I2C_W_LEN 3
#define OV4689_I2C_R_ADDR_LEN 2
#define OV4689_I2C_R_DATA_LEN 1
#define OV4689_TPM_TRIGGER_ADDR 0x4D12
#define OV4689_TPM_TRIGGER_TRIGGER 0x1
#define OV4689_TPM_READ_ADDR    0x4D13
#define OV4689_TPM_OFFSET -64

static int _readI2c(int i2c_fd, int *temperature)
{
	int len = 0;
	char buf[OV4689_I2C_BUF_SIZE];

	if (ioctl(i2c_fd, I2C_SLAVE, SENSOR_I2C_SLAVE_ADDR) < 0) {
		fprintf(stderr, "ioctl error: %s\n", strerror(errno));
		return -1;
	}

	len = OV4689_I2C_W_LEN;
	buf[0] = (OV4689_TPM_TRIGGER_ADDR & 0xFF00) >> 8;
	buf[1] = OV4689_TPM_TRIGGER_ADDR & 0xFF;
	buf[2] = OV4689_TPM_TRIGGER_TRIGGER;

	/* Trigger temperature sensor */
	if (write(i2c_fd, buf, len) != len) {
		perror("i2c-write");
		return -1;
	}

	len = OV4689_I2C_R_ADDR_LEN;
	buf[0] = (OV4689_TPM_READ_ADDR & 0xFF00) >> 8;
	buf[1] = OV4689_TPM_READ_ADDR & 0xFF;

	/* Read temperature sensor */
	if (write(i2c_fd, buf, len) != len) {
		perror("i2c-write");
		return -1;
	}

	len = OV4689_I2C_R_DATA_LEN;
	if (read(i2c_fd, buf, len) != len) {
		perror("i2c-read");
		return -1;
	}

#ifdef DEBUG
	for (int i = 0; i < len; ++i) {
		printf("%02X ", buf[i]);
	}
	printf("\n");
#endif

	/* Transfer to degree */
	*temperature = atoi(buf) + OV4689_TPM_OFFSET;

	return 0;
}

#endif
#endif /* THERMAL_PROTD_OV4678_H_ */
