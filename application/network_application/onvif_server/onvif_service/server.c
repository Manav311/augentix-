#include "stdio.h"
#include "stdlib.h"
#include "DeviceBinding.nsmap"
#include "soapStub.h"
#include "stdsoap2.h"
#include "soapH.h"
#include "httpda.h"
#include "pthread.h"
#include "tev_process.h"

#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/un.h>
#include <json.h>
#include <signal.h>
#include <sys/file.h>
#include <errno.h>

#include "augentix.h"

//Globals:
int cmdTransFD = 0;;
int master_id = 0;

static void segmentFaltHandler(int seg)
{
	fprintf(stderr, "==> Segmentation fault!! <==\n");
	exit(1);
}

int http_get(struct soap *soap)
{
	char cmd[256] = { 0 };
	char *jpeg_path = "/tmp/snapshot.jpg";
	char *ptr = NULL;
	FILE *fp = NULL;
	int status = 0;
	int size = 0;
	int num = 0;
	AGTX_STRM_CONF_S strm = { 0 };

	soap->http_content = "text/html"; // HTTP header with text/html content

	if (aux_check_http_auth(soap) != SOAP_OK) {
		soap_send_empty_response(soap, 401); // HTTP authentication required
		return SOAP_OK;
	}

	if (strlen(soap->path) < strlen(SNAPSHOT_URI)) {
		soap_send_empty_response(soap, 404); // return HTTP not found error
		return SOAP_OK;
	}

	if ((ptr = strstr(soap->path, SNAPSHOT_URI)) != NULL) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
			return SOAP_FAULT;
		}

		num = strm.video_strm_cnt;

		if (isdigit(*(char *)(ptr + strlen(SNAPSHOT_URI)))) {
			num = atoi(ptr + strlen(SNAPSHOT_URI));
		}

		if (num >= (int)strm.video_strm_cnt) {
			soap_send_empty_response(soap, 404); // return HTTP not found error
			return SOAP_OK;
		}

		sprintf(cmd, "mpi_snapshot jpeg 1 %d /tmp/snapshot.jpg", num);
		printf("Executing command: %s\n", cmd);
		status = system(cmd);
		if (status < 0 || (status >> 8)) {
			printf("execute snapshot failed\n");
			return SOAP_FAULT;
		}

		fp = fopen(jpeg_path, "rb");
		if (!fp) {
			printf("open jpeg failed %d(%m)\n", errno);
			soap_send_empty_response(soap, 503); // return HTTP service unable
			return SOAP_OK;
		}

		soap->http_content = "image/jpeg"; // HTTP header with image/jpeg content
		if (soap_response(soap, SOAP_FILE) == SOAP_OK) {
			while (1) {
				size = fread(soap->tmpbuf, 1, sizeof(soap->tmpbuf), fp);
				if (!size || soap_send_raw(soap, soap->tmpbuf, size))
					break;
			}
		}

		fclose(fp);
		status = remove(jpeg_path);
		if (status) {
			printf("remove jpeg failed %d(%m)\n", errno);
		}

		soap_end_send(soap);
		return soap_closesock(soap);
	} else {
		soap_send_empty_response(soap, 404); // return HTTP not found error
		return SOAP_OK;
	}

	soap_send_empty_response(soap, 404); // return HTTP not found error
	return SOAP_OK;
}

void *process_request_thread(void *soap)
{
	pthread_detach(pthread_self());
	soap_serve((struct soap *)soap);
	soap_destroy((struct soap *)soap); // dealloc C++ data
	soap_end((struct soap *)soap); // dealloc data and clean up
	soap_done((struct soap *)soap); // detach soap struct
	free(soap);
	return NULL;
}

int onvif_service_thread(void *data)
{
	int m, s;
	struct soap soap;
//	struct soap *tsoap;
//	pthread_t tid;

	printf("onvif_service_thread start\n");
	master_id = 0;

	soap_init(&soap);
	soap_register_plugin(&soap, http_da);
	soap.fget = http_get;
	soap.send_timeout = 10;
	soap.recv_timeout = 10;
	soap.bind_flags = SO_REUSEADDR;
	// soap_set_namespaces(&add_soap, namespaces);
	m = soap_bind(&soap, NULL, 8899, 100);
	if (m < 0) {
		soap_print_fault(&soap, stderr);
		return -1;
	}

	cmdTransFD = AG_connect_to_cc();
	if (cmdTransFD < 0) {
		return -1;
	}

	if (aux_init_venc_res() < 0) {
		return -1;
	}

#if 1
//	single thread
	while (1) {
		s = soap_accept(&soap);
		if (!soap_valid_socket(s)) {
			if (soap.errnum) {
				soap_print_fault(&soap, stderr);
				return -1;
			}
			printf("server timed out\n");
			continue;
		}
		if (s < 0) {
			soap_print_fault(&soap, stderr);
			exit(-1);
		}
//		fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);
//		printf("ip %lu.%lu.%lu.%lu\n", soap.ip >> 24, (soap.ip << 8) >> 24, (soap.ip << 16) >> 24,
//		       (soap.ip << 24) >> 24);
		soap_serve(&soap);
//		printf("server close");
		soap_end(&soap);
//		usleep(500000);
	}
#else
	//multi thread
	while (1) {
		s = soap_accept(&soap);
		if (!soap_valid_socket(s)) {
			if (soap.errnum) {
				soap_print_fault(&soap, stderr);
				return -1;
			}
			printf("server timed out\n");
			continue;
		}
		if (s < 0) {
			soap_print_fault(&soap, stderr);
			return -1;
		}
		//		fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);
		//		printf("ip %lu.%lu.%lu.%lu\n",soap.ip >> 24,(soap.ip << 8) >> 24,
		//		       (soap.ip << 16) >> 24,(soap.ip << 24) >> 24);
		tsoap = soap_copy(&soap); // make a safe copy
		if (!tsoap) {
			printf("soap_copy fail\n");
			continue;
		}
		pthread_create(&tid, NULL, (void *(*)(void *))process_request_thread, (void *)tsoap);
//		usleep(500000);
	}
#endif
	soap_done(&soap); // detach soap struct
	close(cmdTransFD);
	//free(devOptions);
	return 0;
}

int main()
{
	pthread_t pid;
	signal(SIGSEGV, segmentFaltHandler);
	/*check for Single Instance of Onvif server*/
	int rval;
    char name[16] = "Onvif";
    char tmpname[16] = {0};
    int rc = 0;
	const char *pidFilePath = "/tmp/onvif_server.lock";
	int pid_file = open(pidFilePath, O_CREAT | O_RDWR, 0600);
	rval = flock(pid_file, LOCK_EX | LOCK_NB);

	if (rval) {
		if (EWOULDBLOCK == errno) {
			fprintf(stderr, "Deceted an other Onvif server instance ...!! \n");
			return rval;
		}
	} else {
		fprintf(stderr, "Starting Onvif Server ... \n");
	}

	if (pthread_create(&pid, NULL, (void *(*)(void *))onvif_service_thread, NULL)) {
		printf("ONVIF_init fail\n");
		return -1;
	}
    rc = pthread_setname_np (pid, name);
    sleep(1);
    if ( rc == 0 ) {
        if ( pthread_getname_np(pid, tmpname,sizeof(tmpname)) == 0) {
            printf("%s thread create [Done]",tmpname);
        } else {
            printf("%s thread create [Fail]",name);
        }
    }

	tev_process_init();

	enableTzUpdate();

	while (1) {
		sleep(1);
	}

	return 0;
}
