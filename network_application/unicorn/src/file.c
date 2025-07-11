#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "action.h"
#include "utils.h"
#include "agtx_types.h"

bool fileExists(const char *filePath)
{
	if (access(filePath, F_OK) != -1)
		return true;
	return false;
}

unsigned long int getFileSize(const char *filePath)
{
	FILE *fp = fopen(filePath, "r");
	if (fp == NULL) {
		DBG_MED("File %s not found!\n", filePath);
		return ACT_FAILURE;
	}
	fseek(fp, 0L, SEEK_END);
	unsigned long int size = ftell(fp);
	fclose(fp);
	DBG_MED("file %s size = '%lu'\n", filePath, size);
	return size;
}

int recvTimeout(int sockfd, char *buf, int len, int timeout)
{
	AGTX_UNUSED(buf);
	AGTX_UNUSED(len);

	fd_set fds;
	int ret;
	struct timeval tv;

	// setting file descriptor set
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	// setting timeout struct timeval
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	// wait to timeout or receive data
	ret = select(sockfd + 1, &fds, NULL, NULL, &tv);

	if (ret == 0) {
		return ACT_TIMEOUT; // timeout!
	}

	if (ret == -1) {
		return ACT_FAILURE; // error
	}

	return ACT_SUCCESS;
}

int recvFile(int sockfd, char *fPath, long fSize, bool convert_crlf)
{
	unsigned long int fRead = 0;
	int fd;
	int readbytes;
	char buffer[BUFSIZE];
	int recv_ret = -1;

	DBG_MED("===> %s():\n", __func__);
	DBG_MED("\t Ready to recive file %s\n", fPath);
	DBG_MED("\t with size %lu from client %d to DUT...\n", fSize, sockfd);

	recv_ret = recvTimeout(sockfd, buffer, BUFSIZE, TIMEOUT);
	if (recv_ret == ACT_FAILURE) {
		ERR("recvTimeout timeout !!");
		return ACT_FAILURE;
	} else if (recv_ret == ACT_TIMEOUT) {
		ERR("Recv file timed out.\n");
		return ACT_FAILURE;
	} else {
		fd = open(fPath, O_WRONLY | O_CREAT);
		if (fd == -1) {
			ERR("open during recvFile");
			return ACT_FAILURE;
		}

		while (true) {
			readbytes = read(sockfd, buffer, BUFSIZE);
			fRead += readbytes;
			DBG_MED("fRead = %ld, fSize = %ld\n", fRead, fSize);
			if (readbytes == -1) {
				ERR("read sockfd");
				return ACT_FAILURE;
			}

			DBG_MED("reading %d bytes from socket\n", readbytes);

			if (write(fd, buffer, readbytes) == -1) {
				ERR("write");
				return ACT_FAILURE;
			}

			if (fRead == (unsigned)fSize || fRead == 0) {
				break;
			}
		}
		close(fd);

		if (convert_crlf) {
			char dos2unix[256] = "dos2unix ";
			strcat(dos2unix, fPath);
			int ret = system(dos2unix);
			if (ret == -1) {
				ERR("system dos2unix");
			}
		}
	}

	return ACT_SUCCESS;
}

int recvBinaryData(int sockfd, long fSize, int16_t *recvData)
{
	unsigned long int fRead = 0;
	char buffer[BUFSIZE];

	DBG_MED("===> %s():\n", __func__);
	DBG_MED("\t with size %lu from client %d to DUT...\n", fSize, sockfd);

	int recv_ret = recvTimeout(sockfd, buffer, BUFSIZE, TIMEOUT);
	if (recv_ret == ACT_FAILURE) {
		ERR("recvTimeout timeout !!");
		return ACT_FAILURE;
	} else if (recv_ret == ACT_TIMEOUT) {
		ERR("Recv file timed out.\n");
		return ACT_FAILURE;
	} else {
		while (true) {
			int readbytes = read(sockfd, buffer, BUFSIZE);
			fRead += readbytes;
			DBG_MED("fRead = %ld, fSize = %ld\n", fRead, fSize);
			if (readbytes == -1) {
				ERR("read sockfd");
				return ACT_FAILURE;
			}

			memcpy(recvData + (fRead - readbytes) / 2, buffer, readbytes);

			if (fRead == (unsigned)fSize || fRead == 0) {
				break;
			}
		}
	}
	return ACT_SUCCESS;
}

int sendFile(int sockfd, char *params)
{
	unsigned long int fSize;
	int fd, ret;
	char fileSize[MAX_FILE_SIZE_BYTES];
	char *fPath;
	bool fExists;
	int is_fpath_malloc = 0;

	fPath = extractSingleMessage(params, strlen(params));

	DBG_MED("===> %s():\n", __func__);
	DBG_MED("\t Ready to send file %s\n", fPath);
	DBG_MED("\t from DUT to client %d ...\n", sockfd);

	char *str = "./";
	char *relDir = strstr(fPath, str);
	if (relDir != NULL) {
		is_fpath_malloc = 1;
		char *basename = relDir + strlen(str);
		char *unicornPath = realpath("/proc/self/exe", NULL);
		char *curDir = strndup(unicornPath, strlen(unicornPath) - (strlen(strrchr(unicornPath, '/')) - 1));
		fPath = malloc(strlen(curDir) + strlen(basename) + 1);
		sprintf(fPath, "%s%s", curDir, basename);
		DBG_MED("change file path to absolute path: %s\n", fPath);
		free(unicornPath);
		free(curDir);
	}
	fExists = fileExists(fPath);

	if (fExists) {
		fSize = getFileSize(fPath);
		snprintf(fileSize, sizeof(fileSize), "%lu", fSize);
		fileSize[MAX_FILE_SIZE_BYTES - 1] = '\0';
		for (int i = 0; i < MAX_FILE_SIZE_BYTES; ++i) {
			if (!isdigit(fileSize[i])) {
				fileSize[i] = '\0';
			}

			DBG_MED("fileSize = '%c'\n", fileSize[i]);
		}
	} else {
		for (int i = 0; i < MAX_FILE_SIZE_BYTES; i++) {
			fileSize[i] = 0;
		}
		fileSize[0] = '-';
		fileSize[1] = '1';
	}

	DBG_MED("Sending file size '%s' to peer\n", fileSize);

	if (send(sockfd, fileSize, sizeof(fileSize), 0) == -1) {
		ERR("fail send file size");
	}

	if (fExists) {
		fd = open(fPath, O_RDONLY, 0); // perms always set to 0
		if (fd == -1) {
			ERR("open file to send");
			return ACT_FAILURE;
		}
		int flag = 1;
		while (flag) {
			ret = sendfile(sockfd, fd, 0, BUFSIZE);
			//DBG_MED( "%d bytes sent from file. \n", ret);
			if (ret == -1) {
				DBG_MED("Error !!! sent file. \n");
				ERR("sendfile");
				if (is_fpath_malloc) {
					free(fPath);
				}
				return ACT_FAILURE;
			}

			if (ret == 0) {
				flag = ret;
			}
		}
		close(fd);
	} else {
		if (is_fpath_malloc) {
			free(fPath);
		}
		return ACT_FAILURE;
	}

	if (is_fpath_malloc) {
		free(fPath);
	}
	return ACT_SUCCESS;
}

int sendValueToPc(int fd, int value)
{
	// send file size
	char val_str[MAX_FILE_SIZE_BYTES];
	snprintf(val_str, MAX_FILE_SIZE_BYTES, "%d", value);

	if (sendData(fd, val_str, MAX_FILE_SIZE_BYTES) < 0) {
		ERR("send value to PC!");
		return ACT_FAILURE;
	}

	return ACT_SUCCESS;
}

/*
sockfd: socket descriptor
data: a ptr
size: data len in bytes
ref :http://beej-zhtw.netdpi.net/05-system-call-or-bust/5-7-send
*/
int sendData(int sockfd, char *data, int size)
{
	int ret = 0;
	while (size != 0) {
		ret = send(sockfd, data, size, 0);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			ERR("sendData");
			break;
		}
		size -= ret;
		data += ret;
	}
	return ret;
}
