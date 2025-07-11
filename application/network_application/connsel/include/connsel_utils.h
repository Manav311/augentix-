#ifndef CONNSEL_UTILS_H
#define CONNSEL_UTILS_H


#include "connsel_common.h"


void connsel_save_route_list(char *iface, connsel_route_entries *data);
void connsel_clear_route_list(char *iface, connsel_route_entries *data);
void connsel_add_route(char *iface, connsel_route_entries *data);
void connsel_del_route(char *iface, connsel_route_entries *data);
void connsel_update_metric(char *iface, connsel_route_entries *data, int met);
connsel_state connsel_get_iface_state(char *iface);
int connsel_get_inet_addr(const char *interface, char *ip);


#endif /* !CONNSEL_UTILS_H */
