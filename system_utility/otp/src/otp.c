#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

#define DEVFILE "/dev/otp_agtx"

struct otp_vendor {
	uint32_t vendor_id[1];
};

struct otp_user {
	uint32_t user_custom[4];
};

struct custom_field {
	uint32_t model : 5;
	uint32_t customer_id : 6;
	uint32_t product_type : 9;
	uint32_t produce_date : 12;
	uint32_t sequence_id[2];
	uint32_t reserved[1];
};

#define OTP_ID 'f'
#define OTP_READ_VENDOR_ID _IOR(OTP_ID, 0, struct otp_vendor)
#define OTP_READ_USER _IOR(OTP_ID, 1, struct otp_user)
#define OTP_WRITE_USER _IOW(OTP_ID, 2, struct otp_user)

int main(void)
{
	int fd, ret;
	struct otp_vendor otp_vendor;
	struct otp_user otp_user;
	struct custom_field *cf = (struct custom_field *)&otp_user;

	fd = open(DEVFILE, O_RDWR);
	if (fd == -1) {
		perror("open");
	}

	/* Read VENDOR_ID */
	memset(&otp_vendor, 0, sizeof(otp_vendor));

	ret = ioctl(fd, OTP_READ_VENDOR_ID, &otp_vendor);
	if (ret == -1) {
		perror("ioctl");
	}
	printf("[app_otp_t] read_vendor[0]: 0x%08x\n", otp_vendor.vendor_id[0]);

	/* Read otp before program */
	memset(cf, 0, sizeof((struct otp_user *)cf));

	ret = ioctl(fd, OTP_READ_USER, (struct otp_user *)cf);
	if (ret == -1) {
		perror("ioctl");
	}
	printf("[app_otp_t] (before write) cf->model: 0x%08x\n", cf->model);
	printf("[app_otp_t] (before write) cf->customer_id: 0x%08x\n", cf->customer_id);
	printf("[app_otp_t] (before write) cf->product_type: 0x%08x\n", cf->product_type);
	printf("[app_otp_t] (before write) cf->produce_date: 0x%08x\n", cf->produce_date);
	printf("[app_otp_t] (before write) cf->sequence_id[0]: 0x%08x\n", cf->sequence_id[0]);
	printf("[app_otp_t] (before write) cf->sequence_id[1]: 0x%08x\n", cf->sequence_id[1]);
	printf("[app_otp_t] (before write) cf->reserved[0]: 0x%08x\n", cf->reserved[0]);

	/* Program otp */
	cf->model = 0x7;
	cf->customer_id = 0xC;
	cf->product_type = 0x1E1;
	cf->produce_date = 0x5E6;
	cf->sequence_id[0] = 0x12345678;
	cf->sequence_id[1] = 0x90abcdef;
	cf->reserved[0] = 0x1b2b3c4d;

	ret = ioctl(fd, OTP_WRITE_USER, (struct otp_user *)cf);
	if (ret == -1) {
		perror("ioctl");
	}

	/* Read otp after program */
	memset(cf, 0, sizeof((struct otp_user *)cf));

	ret = ioctl(fd, OTP_READ_USER, (struct otp_user *)cf);
	if (ret == -1) {
		perror("ioctl");
	}
	printf("[app_otp_t] (after write) cf->model: 0x%08x\n", cf->model);
	printf("[app_otp_t] (after write) cf->customer_id: 0x%08x\n", cf->customer_id);
	printf("[app_otp_t] (after write) cf->product_type: 0x%08x\n", cf->product_type);
	printf("[app_otp_t] (after write) cf->produce_date: 0x%08x\n", cf->produce_date);
	printf("[app_otp_t] (after write) cf->sequence_id[0]: 0x%08x\n", cf->sequence_id[0]);
	printf("[app_otp_t] (after write) cf->sequence_id[1]: 0x%08x\n", cf->sequence_id[1]);
	printf("[app_otp_t] (after write) cf->reserved[0]: 0x%08x\n", cf->reserved[0]);

	ret = close(fd);
	if (fd == -1) {
		perror("close");
	}

	return 0;
}
