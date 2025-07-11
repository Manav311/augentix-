#include "stdio.h"
#include "stdlib.h"
#include "wsdd.nsmap"
#include "soapStub.h"
#include "stdsoap2.h"
#include "soapH.h"

int main()
{
	struct soap soap;
	struct ip_mreq mcast;
	int count = 0;

	soap_init1(&soap, SOAP_IO_UDP | SOAP_XML_IGNORENS);

	//soap_set_namespaces(&soap, namespaces);

	if (!soap_valid_socket(soap_bind(&soap, NULL, 3702, 10))) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}

	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	mcast.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(soap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mcast, sizeof(mcast)) < 0) {
		printf("setsockopt error! %d(%m)\n", errno);
		return 0;
	}

	while (1) {
		if (soap_serve(&soap)) {
			soap_print_fault(&soap, stderr);
		}

		soap_destroy(&soap);
		soap_end(&soap);

		printf("Accepted count %d, connection from IP = %lu.%lu.%lu.%lu socket = %d \r\n", count,
		       ((soap.ip) >> 24) & 0xFF, ((soap.ip) >> 16) & 0xFF, ((soap.ip) >> 8) & 0xFF, (soap.ip) & 0xFF,
		       (soap.socket));
		count++;
	}

	soap_done(&soap);

	return 0;
}
