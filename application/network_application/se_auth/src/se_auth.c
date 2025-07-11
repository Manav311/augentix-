#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h> /* Include this header for chmod() */
#include <getopt.h> /* Include for command line parsing */
#include <fcntl.h>
#include <errno.h>

#include "openssl/bio.h"
#include "openssl/evp.h"
#include "openssl/buffer.h"
#include "openssl/rand.h"
#include "json.h"
#include <selib/Core_SELib.h>

#include "log_define.h"
#include "secure_element.h"

#define HTTP_BUF_SIZE 2048
#define HTPASSWD_FILE "/etc/nginx/.htpasswd"
#define DAFAULT_PASSWORD_FLAG "/system/www/.default-passwd"
#define AUTH_SOCKET_PATH "/tmp/auth_socket"

typedef enum { CASE_PASSWD_AUTH = 0, CASE_ADD_USER, CASE_READ_SE, CASE_TYPE_MAX } CaseType;

/**
 * @brief Openssl SHA256 function
 * @details
 * @param[in] input
 * @param[in] input_len
 * @param[in] output
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int opensslSHA256(const unsigned char *input, size_t input_len, unsigned char *output)
{
	EVP_MD_CTX *mdctx = NULL;
	unsigned int output_len = 0;

	if ((mdctx = EVP_MD_CTX_new()) == NULL) {
		return -1;
	}

	if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
		EVP_MD_CTX_free(mdctx);
		return -1;
	}

	if (1 != EVP_DigestUpdate(mdctx, input, input_len)) {
		EVP_MD_CTX_free(mdctx);
		return -1;
	}

	if (1 != EVP_DigestFinal_ex(mdctx, output, &output_len)) {
		EVP_MD_CTX_free(mdctx);
		return -1;
	}

	/* Free resource */
	EVP_MD_CTX_free(mdctx);

	return 0;
}

#ifdef DEBUG
/**
 * @brief print hexadecimal data
 * @details
 * @param[in] content
 * @param[in] size
 * @arg
 * @see
 * @return
 * @retval 
 */
static void printHex(const unsigned char *content, const size_t size)
{
	for (size_t i = 0; i < size; i++) {
		printf("%02x", content[i]);
		if ((i + 1) % 2 == 0) {
			printf(" ");
		}
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
	if (size % 16 != 0) {
		printf("\n");
	}
}
#else
static void printHex(const unsigned char *content, const size_t size)
{
	(void)(content);
	(void)(size);
}
#endif

/**
 * @brief Base64 decoding function.
 * @details Decodes a Base64-encoded string into raw binary data.
 * @param[in] input Base64-encoded input string.
 * @param[out] output_len Pointer to a variable that stores the length of the decoded data.
 * @return Pointer to the decoded buffer.
 * @retval Non-NULL Successful decoding.
 * @retval NULL Failed to decode or memory allocation error.
 */
unsigned char *base64Decode(const char *input, size_t *output_len)
{
	BIO *b64, *bio;
	size_t input_len = strlen(input);

	/* Calculate the number of padding characters */
	size_t padding = 0;
	if (input_len >= 2) {
		if (input[input_len - 1] == '=')
			padding++;
		if (input[input_len - 2] == '=')
			padding++;
	}

	/* Estimate the decoded length */
	*output_len = (input_len * 3) / 4 - padding;
	unsigned char *output = malloc(*output_len + 1); /* +1 for null terminator */
	if (!output)
		return NULL;

	/* Create Base64 BIO */
	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); /* Disable newline processing */
	bio = BIO_new_mem_buf(input, -1); /* Use original input with automatic length calculation */
	bio = BIO_push(b64, bio);

	/* Read the decoded data */
	int len = BIO_read(bio, output, input_len);
	if (len <= 0) {
		free(output);
		output = NULL;
	} else {
		output[len] = '\0'; /* Null-terminate the decoded string */
		*output_len = len;
	}

	/* Free BIO resources */
	BIO_free_all(bio);
	return output;
}

/**
 * @brief Reads a device secret from a secure element (SE) data slot.
 *
 * This function reads the data stored in a specified secure element (SE) data slot
 * and copies it into the provided output buffer. It ensures that the data is successfully
 * read and logs the operation for debugging purposes.
 *
 * @param[in]  slot_id Identifier of the data slot to be read.
 * @param[out] output  Pointer to the buffer where the read data will be stored.
 * @param[in]  size    Size of the output buffer in bytes.
 *
 * @return 0 on success, error code on failure:
 *         - Error codes returned by `Core_read_slot_zone` in case of read failure.
 *
 * @note The function assumes `SLOT_LEN` defines the maximum size of the data slot. 
 *       It is the caller's responsibility to ensure the `output` buffer is large 
 *       enough to accommodate the data.
 * @warning If `size` is smaller than `SLOT_LEN`, this may lead to buffer overflows.
 */
static int readDeviceSecretInSe(const int slot_id, uint8_t *output, size_t size)
{
	int ret = 0;
	uint8_t buffer[SLOT_LEN] = { 0 };

	log_info("[SE][INFO] Data in Slot %d:\n", slot_id);

	if (size < SLOT_LEN) {
		log_err("please assign a buffer larger then slot len\n");
		return -EINVAL;
	}

	ret = Core_read_slot_zone(READ_MODE_DATA_ZONE_32, slot_id, 0, buffer);
	if (ret != CMD_RET_OK) {
		log_err("failed to read slot: %d\n", slot_id);
		return ret;
	}

	memcpy(output, buffer, SLOT_LEN);
	printHex(buffer, SLOT_LEN);
	return ret;
}

/**
 * @brief Stores a device secret into a secure element (SE) data slot.
 *
 * This function writes a given `device_secret` into a specific data slot 
 * within a secure element. It ensures the input data does not exceed 
 * the slot size and uses the SE API to perform the write operation.
 *
 * @param[in] device_secret Pointer to the device secret to be stored.
 * @param[in] size          Size of the device secret in bytes.
 * @param[in] slot_id       Identifier of the data slot where the secret will be stored.
 *
 * @return 0 on success, negative error code on failure:
 *         - -EINVAL if the size exceeds the data slot length.
 *         - Other error codes as returned by `Core_write_zone`.
 *
 * @note The function assumes `SLOT_LEN` defines the maximum size of a SE data slot.
 *       It logs errors and information for debugging purposes.
 */
static int storeDeviceSecretInSe(uint8_t *device_secret, size_t size, uint8_t slot_id)
{
	int ret = 0;
	uint8_t buffer[SLOT_LEN] = { 0 };

	/* Boundary checking */
	if (size > SLOT_LEN) {
		log_err("[SE][ERR] The size of the device_secret exceeds the size of the data slot.\n");
		return -EINVAL;
	}

	/* Write slot */
	memcpy(buffer, device_secret, size);
	ret = Core_write_zone(WRITE_MODE_DATA_32, slot_id, 0, buffer);
	if (ret) {
		log_err("[SE][ERR] Core_write_zone failed.\n");
		return ret;
	}
	log_info("[SE][INFO] Write slot [%d]:\n", slot_id);
	printHex(device_secret, size);

	return ret;
}

/**
 * @brief Find user in .htpasswd file
 * @details Searches for the specified username in the .htpasswd file and retrieves it.
 * @param[in] username Username to search for.
 * @arg
 * @see
 * @return Pointer to the hash string if found.
 * @retval Non-NULL Found the user
 * @retval NULL User not found or error occurred.
 */
static char *findUserInHtpasswd(const char *username)
{
	FILE *file = fopen(HTPASSWD_FILE, "r");
	if (!file) {
		perror("fopen");
		return NULL;
	}

	char line[MAX_USER_NAME_LEN];
	while (fgets(line, sizeof(line), file)) {
		/* Split line into username and hash */
		char *stored_username = strtok(line, ":");
		if (stored_username && strcmp(stored_username, username) == 0) {
			fclose(file);
			return strdup(stored_username); /* Return a copy of the username */
		}
	}

	fclose(file);
	return NULL; /* Username not found */
}

/**
 * @brief Save the username to the .htpasswd file.
 * @details 
 * - Opens the .htpasswd file in write mode, clearing its existing content.
 * - Writes the provided username followed by a null character to the file.
 * 
 * @param[in] username Pointer to the username to be saved.
 * 
 * @arg None
 * 
 * @see None
 * 
 * @return Status of the save operation.
 * @retval 0 Successfully saved the username.
 * @retval -1 Failed to save the username (e.g., file open error).
 */
static int saveUserName(unsigned char *username)
{
	/* Open the .htpasswd file to write the new user (clearing existing content) */
	FILE *file = fopen(HTPASSWD_FILE, "w");
	if (!file) {
		perror("fopen");
		return -1;
	}

	fprintf(file, "%s:", username);
	fputc('\0', file);

	fclose(file);

	return 0;
}

/**
 * @brief Generate random bytes using the SE's True Random Number Generator (TRNG).
 * @details 
 * - Uses the SE's TRNG to generate random numbers in chunks of 32 bytes.
 * - Fills the provided buffer with the requested number of random bytes.
 * 
 * @param[out] buf Pointer to the buffer where the random bytes will be stored.
 * @param[in] num Number of random bytes to generate. Can be 1 to max of size_t
 * 
 * @arg Relies on the `Core_random` function for TRNG output.
 * 
 * @see None
 * 
 * @return Status of the random number generation.
 * @retval 1 Successfully generated the requested random bytes.
 * @retval 0 Failed to generate random bytes.
 */
static int customRandBytes(unsigned char *buf, size_t num)
{
	unsigned char random_output[32] = { 0 };
	size_t remaining_bytes = num, offset = 0;
	int ret;
	do {
		ret = Core_random(random_output);
		if (ret) {
			return 0;
		}
		if (remaining_bytes >= 32) {
			memcpy(buf + offset, random_output, 32);
			offset += 32;
			remaining_bytes -= 32;
		} else {
			memcpy(buf + offset, random_output, remaining_bytes);
			remaining_bytes = 0;
		}
	} while (remaining_bytes > 0);
	return 1;
}

/**
 * @brief Save the password to the SE (Secure Element) with hashing and salting.
 * @details 
 * - Generates a salt using the SE's True Random Number Generator (TRNG).
 * - Concatenates the password and salt, then hashes the result using SHA-256.
 * - Stores the resulting hash and salt in designated SE slots.
 * 
 * @param[in] password Pointer to the password to be saved.
 * @param[in] passwd_len Length of the password.
 * 
 * @arg Relies on `customRandBytes` for salt generation and `opensslSHA256` for hashing.
 * 
 * @see `customRandBytes`, `opensslSHA256`, `storeDeviceSecretInSe`
 * 
 * @return Status of the save operation.
 * @retval 0 Successfully saved the password and salt in the SE.
 */
static int savePasswdToSE(unsigned char *password, size_t passwd_len)
{
	unsigned char salt[SALT_SLOT_LEN];
	unsigned char passwd_merge[SHA256_INPUT_LEN];
	unsigned char passwd_hash[SHA256_OUTPUT_LEN];

	memset(salt, 0x00, sizeof(salt));
	memset(passwd_merge, 0x00, sizeof(passwd_merge));

	customRandBytes(salt, sizeof(salt)); /* create salt with SE TRNG*/

	memcpy(passwd_merge, password, passwd_len);
	memcpy(passwd_merge + passwd_len, salt, SALT_SLOT_LEN);

	opensslSHA256(passwd_merge, SHA256_INPUT_LEN, passwd_hash);

	/* write SALT and passwd to SLOT */
	storeDeviceSecretInSe(passwd_hash, SHA256_OUTPUT_LEN, PASSWD_HASH_SLOT);
	storeDeviceSecretInSe(salt, SALT_SLOT_LEN, SALT_SLOT);

	return 0;
}

/**
 * @brief Add new user to .htpasswd file, only allow 1 user once
 * @details Hashes the password using Argon2 and adds a new user to the .htpasswd file.
 * @param[in] username Username to add.
 * @param[in] password Password to hash and add.
 * @arg
 * @see
 * @return Result of adding user.
 * @retval 0 Successfully added user.
 * @retval -1 Failed to add user.
 */
static int addUserToHtpasswd(unsigned char *username, unsigned char *password)
{
	/* Check if the account password is the same as the old one */
	char *stored_username = findUserInHtpasswd((char *)username);
	if (stored_username != NULL) {
		log_warn("User exist: %s", stored_username);
	} else {
		free(stored_username);
	}

	/* Remove default-password flag */
	if (access(DAFAULT_PASSWORD_FLAG, F_OK) == 0) {
		if (remove(DAFAULT_PASSWORD_FLAG) == 0) {
			log_info("File '%s' deleted successfully.\n", DAFAULT_PASSWORD_FLAG);
		} else {
			perror("Error deleting file");
			return EXIT_FAILURE;
		}
	}

	saveUserName(username);
	savePasswdToSE(password, strlen((char *)password));

	return 0;
}

/**
 * @brief Reset user credentials to default values.
 * @details 
 * - Sets the username to "admin".
 * - Saves the default username and password to secure storage.
 * 
 * @param None
 * 
 * @arg None
 * 
 * @see None
 * 
 * @return Status of the reset operation.
 * @retval 0 Successfully reset credentials to default.
 */
static int resetToDefault()
{
	unsigned char default_username[6];
	snprintf((char *)default_username, sizeof("admin"), "%s", "admin");
	saveUserName(default_username);
	savePasswdToSE((unsigned char *)DEFAULT_PASSWD, DEFAULT_PASSWD_LEN);
	return 0;
}

/**
 * @brief Create an empty file or update the timestamp of an existing file.
 * @details 
 * - Opens the specified file in write mode, creating it if it does not exist.
 * - Sets the file permissions to `0644` (read/write for owner, read-only for others).
 * - If the file exists, it updates its last modification timestamp.
 * 
 * @param[in] filename Path to the file to be created or updated.
 * 
 * @arg None
 * 
 * @see None
 * 
 * @return Status of the operation.
 * @retval 0 Successfully created or updated the file.
 * @retval -1 Failed to create or update the file (e.g., open error).
 */
static int touch(const char *filename)
{
	int fd = open(filename, O_CREAT | O_WRONLY, 0644);
	if (fd == -1) {
		perror("open");
		return -1;
	}
	close(fd);
	return 0;
}

/**
 * @brief Compares two hexadecimal arrays byte by byte.
 *
 * @param hex1 Pointer to the first hexadecimal array.
 * @param hex2 Pointer to the second hexadecimal array.
 * @param size Length of the arrays to compare.
 *
 * @return 0 if the arrays are equal, a negative value if hex1 < hex2,
 *         and a positive value if hex1 > hex2.
 */
int hexCmp(const uint8_t *hex1, const uint8_t *hex2, size_t size)
{
	for (size_t i = 0; i < size; ++i) {
		if (hex1[i] != hex2[i]) {
			return hex1[i] - hex2[i];
		}
	}
	return 0;
}

/**
 * @brief Handle incoming client request
 * @details Reads and processes client socket requests, performing authentication.
 * @param[in] client_socket Socket descriptor for the client.
 * @arg
 * @see
 * @return void.
 */
void handleAuthenticationOfHttp(int client_socket)
{
	char buffer[HTTP_BUF_SIZE];
	ssize_t bytes_read = read(client_socket, buffer, HTTP_BUF_SIZE - 1);
	if (bytes_read <= 0) {
		close(client_socket);
		return;
	}
	buffer[bytes_read] = '\0';

	log_info("%s\n", buffer);

	/* Check for User-Agent: curl and Content-Type: application/json */
	char *user_agent = strstr(buffer, "User-Agent: curl");
	char *content_type = strstr(buffer, "Content-Type: application/json");

	if (user_agent == NULL || content_type == NULL) {
		goto se_authentication;
	}

	char *body = strstr(buffer, "\r\n\r\n");

	if (!body) {
		goto bad_request;
	}

	body += 4; /* Move past the header-body separator */
	/* Parse JSON content using json-c */
	struct json_object *parsed_json = json_tokener_parse(body);

	if (!parsed_json) {
		goto bad_request;
	}

	/* Extract "username", "password", and "agent" from JSON */
	struct json_object *json_username = NULL;
	struct json_object *json_password = NULL;
	struct json_object *json_agent = NULL;

	json_object_object_get_ex(parsed_json, "username", &json_username);
	json_object_object_get_ex(parsed_json, "password", &json_password);
	json_object_object_get_ex(parsed_json, "agent", &json_agent);

	const char *agent_str = json_object_get_string(json_agent);
	if (strcmp(agent_str, "passwd_secure.sh") != 0) {
		goto free_json;
	}

	/* In case of adding a new user */
	if (json_username && json_password && json_object_is_type(json_username, json_type_string) &&
	    json_object_is_type(json_password, json_type_string)) {
		const char *username_str = json_object_get_string(json_username);
		const char *password_str = json_object_get_string(json_password);

		if (addUserToHtpasswd((unsigned char *)username_str, (unsigned char *)password_str) == 0) {
			log_info("User '%s':%s added to .htpasswd successfully.\n", username_str, password_str);
		} else {
			log_err("Failed to add user '%s' to .htpasswd.\n", username_str);
			goto free_json;
		}
	}

	/* Respond with 200 OK and skip authentication if conditions are met */
	/* Clean up the JSON object */
	json_object_put(parsed_json);
	dprintf(client_socket, "HTTP/1.0 200 OK\r\n\r\n");
	close(client_socket);
	return;

free_json:
	/* Clean up the JSON object */
	json_object_put(parsed_json);

bad_request:
	dprintf(client_socket, "HTTP/1.1 400 Bad Request\r\n"
	                       "Content-Type: text/plain\r\n"
	                       "Content-Length: 0\r\n"
	                       "\r\n");
	close(client_socket);
	return;

	char *auth_header = NULL;
se_authentication:
	/* Find the Authorization header */
	auth_header = strstr(buffer, "Authorization: Basic ");
	if (!auth_header) {
		goto close_client;
	}

	/* Extract the Base64-encoded credentials */
	auth_header += strlen("Authorization: Basic ");
	char *end = strstr(auth_header, "\r\n");
	if (end)
		*end = '\0';

	/* Decode Base64 credentials */
	size_t decoded_len;
	unsigned char *decoded_credentials = base64Decode(auth_header, &decoded_len);
	if (!decoded_credentials) {
		log_warn("failed to find credentials\n");
		goto close_client;
		return;
	}

	log_info("decoded_credentials: %s", decoded_credentials);

	/* Split into username and password */
	char *input_username = strtok((char *)decoded_credentials, ":");
	char *input_password = strtok(NULL, ":");
	if (!input_username || !input_password) {
		log_warn("failed to find username and password\n");
		goto free_decoded_credentials;
	}

	log_info("HTTP user/password: %s %s, len %d\n", input_username, input_password, strlen(input_password));

	/* Find the user's hashed password in .htpasswd */
	char *stored_username = findUserInHtpasswd(input_username);
	if (!stored_username) {
		log_warn("failed tp find user in htpasswd\n");
		goto free_decoded_credentials;
	}

	unsigned char salt_buffer[SLOT_LEN];
	unsigned char input_passwd_hash_buffer[SLOT_LEN] = { 0 };
	unsigned char auth_passwd[SHA256_INPUT_LEN] = { 0 };
	unsigned char auth_passwd_hash[SHA256_OUTPUT_LEN] = { 0 };

	readDeviceSecretInSe(SALT_SLOT, salt_buffer, SALT_SLOT_LEN);

	/* Concatenates the password and salt, then hashes the result using SHA-256. */
	memcpy(auth_passwd, input_password, strlen(input_password));
	memcpy(auth_passwd + strlen(input_password), salt_buffer, SALT_SLOT_LEN);
	opensslSHA256(auth_passwd, SHA256_INPUT_LEN, auth_passwd_hash);
	readDeviceSecretInSe(PASSWD_HASH_SLOT, input_passwd_hash_buffer, SLOT_LEN);

	/* compare the hash result with stored hash*/
	int ret = hexCmp(input_passwd_hash_buffer, auth_passwd_hash, SLOT_LEN);

	if (ret == 0) {
		dprintf(client_socket, "HTTP/1.0 200 OK\r\n\r\n");
	} else {
		log_err("password verify failed: (%s, %d) ret: %d", input_password, strlen(input_password), ret);
		goto free_stored_hash;
	}

	free(decoded_credentials);
	free(stored_username);
	close(client_socket);

	return;

free_stored_hash:
	free(stored_username);

free_decoded_credentials:
	free(decoded_credentials);

close_client:
	dprintf(client_socket,
	        "HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\nWWW-Authenticate: Basic realm=\"Please input password\"\r\n\r\n");
	close(client_socket);

	return;
}



/**
 * @brief Display usage information
 * @details Prints the usage information for the command-line options.
 * @param[in] program_name Name of the executable program.
 * @arg
 * @see
 * @return void.
 */
static void help(const char *program_name)
{
	printf("USAGE: %s [options] ...\n"
	       "-l Listen %s to verify username and password\n"
	       "-e -u <username> -p <password>  Add username and password to %s",
	       program_name, AUTH_SOCKET_PATH, HTPASSWD_FILE);
}

/**
 * @brief Main function
 * @details Entry point of the program, handles argument parsing, socket creation, and request handling.
 * @param[in] argc Number of command-line arguments.
 * @param[in] argv Array of command-line argument strings.
 * @arg
 * @see
 * @return Exit status.
 * @retval 0 Successful execution.
 * @retval Non-zero Error occurred.
 */
int main(int argc, char **argv)
{
	int server_socket, client_socket;
	struct sockaddr_un server_addr;

	int opt, ret;
	unsigned char *username = NULL;
	unsigned char *password = NULL;
	CaseType case_type = CASE_TYPE_MAX;
	int i2c_bus_id = SE_I2C_BUS_ID;

	/* Initialize libse */
	ret = securityLib_init();
	if (ret) {
		log_err("Fail to initialize the secure element library.\n");
		return ret;
	}

	/* Set I2C bus ID */
	ret = securityLib_setI2CBus(i2c_bus_id);
	if (ret) {
		log_err("Fail to set the I2C bus ID.\n");
		return ret;
	}

	unsigned char buffer[SLOT_LEN];
	/* Parse command-line arguments */
	while ((opt = getopt(argc, argv, "leu:p:r:Rw")) != -1) {
		switch (opt) {
		case 'l':
			case_type = CASE_PASSWD_AUTH;
			break;
		case 'e':
			case_type = CASE_ADD_USER;
			break;
		case 'u':
			/* Set the username */
			username = (unsigned char *)optarg;
			break;
		case 'p':
			/* Set the password */
			password = (unsigned char *)optarg;
			break;
		case 'r':
			case_type = CASE_READ_SE;
			readDeviceSecretInSe(atoi(optarg), buffer, 32);
			readDeviceSecretInSe(atoi(optarg) + 1, buffer, 32);
			exit(1);
		case 'R':
			touch(DAFAULT_PASSWORD_FLAG);
			resetToDefault();
			exit(1);
		case 'w':
			memset(buffer, 0xff, 32);
			storeDeviceSecretInSe(buffer, 32, SALT_SLOT);
			storeDeviceSecretInSe(buffer, 32, PASSWD_HASH_SLOT);
			exit(1);
		case 'h':
		default:
			help(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/* reset to default if default flag exist. */
	if (access(DAFAULT_PASSWORD_FLAG, F_OK) == 0) {
		resetToDefault();
	}

	/* Forward Compatibility: if no default flag but passwd not stored in SE, it will reset to default username and passwd */
	/* TODO: need to check by OTA test*/
	memset(buffer, 0xff, sizeof(buffer));
	unsigned char salt_slot_storage[SLOT_LEN];
	readDeviceSecretInSe(SALT_SLOT, salt_slot_storage, SLOT_LEN);
	if (0 == strncmp((char *)buffer, (char *)salt_slot_storage, SLOT_LEN)) {
		touch(DAFAULT_PASSWORD_FLAG);
		resetToDefault();
	}

	/* If -e option is used, add user to .htpasswd and exit */
	if (case_type == CASE_ADD_USER && (username == NULL || password == NULL)) {
		log_err("Failed to add user '%s' to .htpasswd.\n", username);
		return -1;
	}

	if (case_type == CASE_ADD_USER && username && password) {
		if (addUserToHtpasswd(username, password) != 0) {
			log_err("Failed to add user '%s' to .htpasswd.\n", username);
			return -1;
		}
		return 0;
	}

	/* Create a Unix socket */
	server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Set up the socket address structure */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, AUTH_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

	/* Bind the socket */
	unlink(AUTH_SOCKET_PATH);
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		close(server_socket);
		exit(EXIT_FAILURE);
	}

	/* Listen for incoming connections */
	if (listen(server_socket, 5) < 0) {
		perror("listen");
		close(server_socket);
		exit(EXIT_FAILURE);
	}

	printf("Listening on %s...\n", AUTH_SOCKET_PATH);

	/* Set the permissions to 766 */
	if (chmod(AUTH_SOCKET_PATH, 0766) != 0) {
		perror("chmod");
		return 1;
	}

	/* Main loop to accept and handle requests */
	while (1) {
		client_socket = accept(server_socket, NULL, NULL);
		if (client_socket < 0) {
			perror("accept");
			continue;
		}

		handleAuthenticationOfHttp(client_socket);
	}

	close(server_socket);
	return 0;
}