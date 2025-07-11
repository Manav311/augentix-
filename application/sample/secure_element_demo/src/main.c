#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(SE_SQ7131)
#include "se_sq7131.h"
#elif defined(SE_SQ7131S)
#include "se_sq7131s.h"
#else
#error "No valid SE_SQ7131 or SE_SQ7131S defined"
#endif
#include "log.h"

/**
 * @brief help messages
 * @details
 * @param NULL
 * @return NULL
 * @retval NULL
 * @see
 */
static void help(const char *program_name)
{
#if defined(SE_SQ7131)
	printf("SE: SQ7131 is selected\n");
	printf("USAGE: %s [options] ...\n"
	       "Options:\n"
	       " -D <I2C bus ID> 			I2C bus ID. (Default = 0)\n"
	       " -P					Secure storage provision\n"
	       " -S					Store device secret in the secure element\n"
	       " -F					Store device secret in the flash\n"
	       " -A					Generate username and password for authentication\n"
	       " -U					Get the UID of the secure element\n"
	       " -C					Counter read\n"
	       " -I <Desired value>			Counter increase\n"
	       " -K <Slot ID>				Key provision for key lock demonstration\n"
	       " -L <Slot ID>				Lock slot\n"
	       " -B <Slot ID>				Lock status of slot\n"
	       " -s <device_secret.txt>			Device secret txt file\n"
	       " -p <product_id.txt>			Product ID txt file\n"
	       " -d <device_id.txt>			Device ID txt file\n"
	       " -e <encrypted_device_secret.bin>	Encrypted device secret bin file\n"
	       " -c <Counter ID>			Counter ID (Defalt = 1).\n"
	       " -t <plaintext.bin>			Plaintext\n"
	       "\n"
	       "Example:\n"
	       " $ %s -D <I2C Bus ID> -U                                                                           Get the UID of the secure element\n"
	       " $ %s -D <I2C Bus ID> -s <device_secret.txt> -S                                                    Store device secret in the secure element\n"
	       " $ %s -D <I2C Bus ID> -s <device_secret.txt> -e <encypted_device_secret.bin> -F                    Store the encrypted device secret in the flash\n"
	       " $ %s -D <I2C Bus ID> -s <device_secret.txt> -e <encypted_device_secret.bin> -V                    Decrypted device secret from a file\n"
	       " $ %s -D <I2C Bus ID> -P                                                                           Execute provision of secure storage\n"
	       " $ %s -D <I2C Bus ID> -p <product_id.txt> -d <device_id.txt> -A                                    Calculate username and password via device secret in secure element\n"
	       " $ %s -D <I2C Bus ID> -p <product_id.txt> -d <device_id.txt> -e <encrypted_device_secret.bin> -A   Calculate username and password via device secret in flash\n"
	       " $ %s -D <I2C Bus ID> -C -c <counter ID>                                                           Reade value of the counter\n"
	       " $ %s -D <I2C Bus ID> -I <desired value> -c <counter ID>                                           Increase value of the counter\n"
	       " $ %s -D <I2C Bus ID> -K <Slot ID> -t <app_magic_num.bin>                                          Key provision in key slot: <Slot ID>\n"
	       " $ %s -D <I2C Bus ID> -L <Slot ID>                                                                 Lock the slot: <Slot ID>\n"
	       " NOTE: For demonstration purposes, only SLOT_37 (Genral Data) can be locked.\n"
	       " $ %s -D <I2C Bus ID> -B <Slot ID>                                                                 Check the status of slot: <Slot ID>\n",
	       program_name, program_name, program_name, program_name, program_name, program_name, program_name,
	       program_name, program_name, program_name, program_name, program_name, program_name);
#elif defined(SE_SQ7131S)
	printf("SE: SQ7131S is selected\n");
	printf("USAGE: %s [options] ...\n"
	       "Options:\n"
	       " -D <I2C bus ID> 			I2C bus ID. (Default = 0)\n"
	       " -U					Get the UID of the secure element\n"
	       " -L <Slot ID>				Lock slot\n"
	       " -B <Slot ID>				Lock status of slot\n"
	       " -C					Counter read\n"
	       " -I <Desired value>			Counter increase\n"
	       " -K					IO proteciont key operation\n"
	       " -S 					Secret key operation\n"
	       " -E					Sensitive data operation\n"
	       " -t <Input bin>			        Input bin\n"
	       " -o <Output path>			Output path\n"
	       " -m <MAC bin>				Mac\n"
	       " -u <Username>				Username\n"
	       " -w <Password>                          Password\n"
	       "\n"
	       "Example:\n"
	       " $ %s -D <I2C Bus ID> -U                                                                               Get the UID of the secure element\n"
	       " $ %s -D <I2C Bus ID> -L <Slot ID>                                                                     Lock the slot: <Slot ID>\n"
	       " $ %s -D <I2C Bus ID> -B <Slot ID>                                                                     Check the status of slot: <Slot ID>\n"
	       " $ %s -D <I2C Bus ID> -C                                                                               Reade value of the counter in slot 36\n"
	       " $ %s -D <I2C Bus ID> -I <Desired value>                                                               Increment the counter to the desired value\n"
	       " $ %s -D <I2C Bus ID> -K                                                                               IO protection key for provision\n"
	       " $ %s -D <I2C Bus ID> -S                                                                               Generate the secret key in slot 13\n"
	       " $ %s -D <I2C Bus ID> -E -d <data.bin> -m <mac.bin> -g <cipher.bin>                                    Encypt sensitive data (Set) using slot 12\n"
	       " $ %s -D <I2C Bus ID> -E -t <cipher.bin> -m <mac.bin> -g <data.bin>                                    Decryt sensitive data (Get) using slot 12\n"
	       " $ %s -D <I2C Bus ID> -E -u <username> -w <password> -o <output path>                                  Encypt sensitive data in hash using slot 9\n"
	       " $ %s -D <I2C Bus ID> -E -u <username> -w <password>                                                   Compare sensitive data in hash using slot 9\n",
	       program_name, program_name, program_name, program_name, program_name, program_name, program_name,
	       program_name, program_name, program_name, program_name, program_name);
#endif
}

int main(int argc, char **argv)
{
	int ret = 0;
	int opt = 0;
	CaseType case_type = -1;
#if defined(SE_SQ7131)
	Meta_data meta_data = { .i2c_bus_id = 0,
		                .counter_id = 1,
		                .slot_id = 0,
		                .desired_counter_val = 0,
		                .f_product_id = 0,
		                .f_device_id = 0,
		                .f_device_secret = NULL,
		                .f_encrypted_device_secret = NULL,
		                .f_plaintext = NULL };
#elif defined(SE_SQ7131S)
	Meta_data meta_data = { .i2c_bus_id = 0,
		                .slot_id = 0,
		                .counter_id = 0x24,
		                .desired_counter_val = 0,
		                .input = NULL,
		                .output_path = NULL,
		                .mac = NULL,
		                .username = NULL,
		                .password = NULL,
		                .data = NULL,
		                .inverse_data = NULL };
#endif

	/* Get parameters */
#if defined(SE_SQ7131)
	while ((opt = getopt(argc, argv, "D:UPFVSACIK:L:B:s:p:d:e:c:t:h")) != -1) {
		switch (opt) {
		case 'D':
			meta_data.i2c_bus_id = atoi(optarg);
			break;
		case 'U':
			case_type = CASE_UID;
			break;
		case 'F':
			case_type = CASE_DEVICE_SECRET_FLASH;
			break;
		case 'V':
			case_type = CASE_DECRYPTED_DEVICE_SECRET_FLASH;
			break;
		case 'S':
			case_type = CASE_DEVICE_SECRET_SE;
			break;
		case 'A':
			case_type = CASE_AUTHENTICATION;
			break;
		case 'P':
			case_type = CASE_SECURE_STORAGE_PROVISION;
			break;
		case 'C':
			case_type = CASE_COUNTER_READ;
			break;
		case 'I':
			case_type = CASE_COUNTER_INCREASE;
			meta_data.desired_counter_val = atoi(optarg);
			break;
		case 'K':
			case_type = CASE_KEY_PROVISION;
			meta_data.slot_id = atoi(optarg);
			break;
		case 'L':
			case_type = CASE_LOCK_SLOT;
			meta_data.slot_id = atoi(optarg);
			break;
		case 'B':
			case_type = CASE_SLOT_STATUS;
			meta_data.slot_id = atoi(optarg);
			break;
		case 'p':
			meta_data.f_product_id = optarg;
			break;
		case 'd':
			meta_data.f_device_id = optarg;
			break;
		case 's':
			meta_data.f_device_secret = optarg;
			break;
		case 'e':
			meta_data.f_encrypted_device_secret = optarg;
			break;
		case 'c':
			meta_data.counter_id = atoi(optarg);
			break;
		case 't':
			meta_data.f_plaintext = optarg;
			break;
		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;
		default:
			help(argv[0]);
			return EXIT_FAILURE;
		}
	}
#elif defined(SE_SQ7131S)
	while ((opt = getopt(argc, argv, "D:UL:B:CI:KSEt:om:u:w:d:g:h")) != -1) {
		switch (opt) {
		case 'D':
			meta_data.i2c_bus_id = atoi(optarg);
			break;
		case 'U':
			case_type = CASE_UID;
			break;
		case 'L':
			case_type = CASE_LOCK_SLOT;
			meta_data.slot_id = atoi(optarg);
			break;
		case 'B':
			case_type = CASE_SLOT_STATUS;
			meta_data.slot_id = atoi(optarg);
			break;
		case 'C':
			case_type = CASE_COUNTER_READ;
			break;
		case 'I':
			case_type = CASE_COUNTER_UPDATE;
			meta_data.desired_counter_val = atoi(optarg);
			break;
		case 'K':
			case_type = CASE_IO_PROTECTION_KEY;
			break;
		case 'S':
			case_type = CASE_SECRET_KEY;
			break;
		case 'E':
			case_type = CASE_SENSITIVE_DATA;
			break;
		case 't':
			meta_data.input = optarg;
			break;
		case 'o':
			meta_data.output_path = "/usrdata";
			break;
		case 'm':
			meta_data.mac = optarg;
			break;
		case 'u':
			meta_data.username = optarg;
			break;
		case 'w':
			meta_data.password = optarg;
			break;
		case 'd':
			meta_data.data = optarg;
			break;
		case 'g':
			meta_data.inverse_data = optarg;
			break;
		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;
		default:
			help(argv[0]);
			return EXIT_FAILURE;
		}
	}
#else
	printf("[SE][ERR] No valid SE_SQ7131 or SE_SQ7131S defined.\n");
	return -EINVAL;
#endif

	/* Execute demonstration */
	printf("[SE][INFO] Secure element demo start.\n");

	if (case_type < 0 || case_type > CASE_NUM) {
		ret = -EINVAL;
		printf("[SE][INFO] Case type is not assigned.\n");
	} else {
		ret = SE_executeDemo(case_type, meta_data);
	}

	if (ret) {
		printf("[SE][ERR] Secure element demo error.\n");
	} else {
		printf("[SE][INFO] Secure element demo end.\n");
	}

	return 0;
}
