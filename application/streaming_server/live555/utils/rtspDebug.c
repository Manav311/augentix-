/*
 * Debug tool to control the rtsp Stream Dump
 *
 * debug tool connects to rtsp server over unix socket /tmp/auxDump
 *
 *
 * Usage:
 *    interactive mode:
 *    ----------------
 *    	rtspDebug
 *    Manual Dump Mode:
 *    1. connect rtspDebug to /tmp/auxDump
 *    2. Make sure sd card is mounted under /mnt/sdcard
 *    3.issue the following commands to control the dump
 *    	> start   <= This will create a dump file with the current_date.h264
 *    	> stop    <= Stops the dump
 *    	> close   <= close the dump file
 *    	> quit    <= quit the application
 *
 *   File size Dump limit mode:
 *   to dump files with the specified file size
 *   1. connect rtspDebug to /tmp/auxDump
 *   2. Make sure sd card is mounted under /mnt/sdcard
 *   3. issue the fillowing commands to dump files with size limit.
 *   ( Note: Since we want every file should ending with complete gop, file
 * sizes may vary a little)
 *   4. isse the following commands to start and stop the dump
 *    	> size XXX Mega
 *    	> start   <= This will create a dump file with the current_date.h264
 *    	> stop    <= Stops the dump
 *    	> close   <= close the dump file
 *
 *    Script  mode:  ( this mode can be used from cron job or other shellscripts
 *    -------------
 */

#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

char *mountPath = "/mnt/sdcard";

static int isDriveMounted()
{
	int driveAvailable = 0;
	struct mntent *mountEntry;
	FILE *fp = setmntent("/proc/mounts", "r");
	do {
		mountEntry = getmntent(fp);
		if (mountEntry != NULL) {
			driveAvailable = !(strcmp(mountEntry->mnt_dir, mountPath));
		}
	} while ((!driveAvailable) && (mountEntry != NULL));

	if (fp != NULL) {
		fclose(fp);
	}
	return driveAvailable;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
void usage(char *cmd)
{
	fprintf(stderr, "manual mode usage: %s socketFile \n", cmd);
	fprintf(stderr, "script_cmd mode usage: %s socketFile  dbg_cmd_message\n", cmd);
	fprintf(stderr, "debug_cmd_message = 'start|stop|close|size=xx ( xx in KB)'\n");
}

int writeToSkt(int sockfd, char *buffer, int msglen)
{
	return write(sockfd, buffer, msglen);
}

int main(int argc, char *argv[])
{
	int sockfd, servlen;
	struct sockaddr_un serv_addr;
	int flag;
	char buffer[256];
	int n, doread;

	if (argc == 2) {
		flag = 1;
	} else if (argc < 3) {
		usage(argv[0]);
		exit(0);
	} else if (argc == 3) {
		sprintf(buffer, "%s", argv[2]);
		flag = 0;
	} else {
		usage(argv[0]);
		exit(0);
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, argv[1]);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		error("Creating socket");
	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
		error("Connecting Error");

	if (isDriveMounted()) {
		if (flag == 0) { // flag = 0 -> non-Interactive mode
			if ((strstr(buffer, "start") != NULL) || (strstr(buffer, "stop") != NULL) ||
			    (strstr(buffer, "close") != NULL) || (strstr(buffer, "size=") != NULL)) {
				n = writeToSkt(sockfd, buffer, strlen(buffer));
				if (n < 0) {
					error("ERROR writing to socket");
				}
				bzero(buffer, 256);
				n = read(sockfd, buffer, 255);
				if (n < 0) {
					error("ERROR reading from socket");
				}
				printf("<=== %s\n", buffer);
				return 0;
			} else {
				printf("===> %s: Unknown cmd <=== \n", buffer);
				return -1;
			}
		}
		while (flag) { // flag = 1 -> for non-interactive mode.
			bzero(buffer, 256);
			fgets(buffer, 255, stdin);
			printf("===> %s: ", buffer);

			if (strstr(buffer, "quit")) {
				close(sockfd);
				flag = 0;
			}
			if (strstr(buffer, "start") != NULL) {
				n = writeToSkt(sockfd, buffer, strlen(buffer));
				doread = 1;
			} else if (strstr(buffer, "stop") != NULL) {
				n = writeToSkt(sockfd, buffer, strlen(buffer));
				doread = 1;
			} else if (strstr(buffer, "close") != NULL) {
				n = writeToSkt(sockfd, buffer, strlen(buffer));
				doread = 1;
			} else if (strstr(buffer, "size=") != NULL) {
				// TODO: thread to monitor the file size and split the dumpfile to parts
				n = writeToSkt(sockfd, buffer, strlen(buffer));
				doread = 1;
			} else {
				fprintf(stderr, "<== Unknown command !\n");
				doread = 0;
			}

			if (doread) {
				if (n < 0) {
					error("ERROR writing to socket");
				}
				bzero(buffer, 256);
				n = read(sockfd, buffer, 255);
				if (n < 0) {
					error("ERROR reading from socket");
				}
				printf("<=== %s\n", buffer);
			} else {
			}
		}
	} else {
		fprintf(stderr, "SD card: not mounted \n");
	}

	return 0;
}
