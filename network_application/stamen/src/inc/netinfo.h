#ifndef STAMEN_NETINFO_H_
#define STAMEN_NETINFO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DHCP_DISABLE 0
#define DHCP_ENABLE 1

char *get_ip();
char *get_netmask();
char *get_mac_addr();

char *get_net_settings();
int set_net_setting(int dhcp_status, char *ip, char *netmask, char *gateway, char *dns1, char *dns2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STAMEN_NETINFO_H_ */
