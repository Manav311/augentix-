#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define HOSTS_PATH       "/etc/hosts"
#define HOSTNAME_PATH    "/etc/hostname"
#define RESOLV_CONF_PATH "/tmp/resolv.conf"
#define INTERFACES_PATH  "/etc/network/interfaces"

char* const short_options = "hs:i:m:g:d:n:b:";
static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "switch", required_argument, NULL, 's' },
	{ "ip", required_argument, NULL, 'i' },
	{ "mask", required_argument, NULL, 'm' },
	{ "gateway", required_argument, NULL, 'g' },
	{ "dns", required_argument, NULL, 'd' },
	{ "hostname", required_argument, NULL, 'n' },
	{ 0, 0, 0, 0 }
};

void help()
{
	printf("Usage:\n");
	printf("\tip_assign <Options> [args]\n");
	printf("\nOptions:\n");
	printf("\t-h                      help\n");
	printf("\t-s <dhcp/static>        switch inet dhcp/static\n");
	printf("\t-i <IP address>         set address <IP address>\n");
	printf("\t-m <Subnet mask>        set netmask <Subnet mask>\n");
	printf("\t-g <gateway>            set gateway <gateway>\n");
	printf("\t-d <\"DNS1 [DNS2]\">      set DNS <DNS1 [DNS2]>\n");
	printf("\t-n <Hostname>           set hostname <Hostname>\n");
}

int main(int argc, char *argv[])
{
	char command[120];
	int c = 0;

	if (argc < 2) {
		help();
		return 0;
	}

	while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
		switch (c) {
			case 'i':
				sprintf(command, "sed -i 's/address .*/address %s/' %s\n", optarg, INTERFACES_PATH);
				system(command);
				break;
			case 's':
				sprintf(command, "sed -i 's/iface eth0 inet .*/iface eth0 inet %s/' %s\n", optarg, INTERFACES_PATH);
				system(command);
				break;
			case 'm':
				sprintf(command, "sed -i 's/netmask .*/netmask %s/' %s\n", optarg, INTERFACES_PATH);
				system(command);
				break;
			case 'g':
				sprintf(command, "sed -i 's/gateway .*/gateway %s/' %s\n", optarg, INTERFACES_PATH);
				system(command);
				break;
			case 'd':
				sprintf(command, "sed -i 's/dns-nameservers .*/dns-nameservers %s/' %s\n", optarg, INTERFACES_PATH);
				system(command);
				break;
			case 'n':
				sprintf(command, "hostname %s\n", optarg);
				system(command);
				sprintf(command, "echo %s > %s\n", optarg, HOSTNAME_PATH);
				system(command);
				sprintf(command, "sed -i 's/127.0.1.1\t.*/127.0.1.1\t%s/' %s\n", optarg, HOSTS_PATH);
				system(command);
				break;
			case 'h':
			default:
				help();
				break;
		}
	}
	system("sync\n");

	return 0;
}
