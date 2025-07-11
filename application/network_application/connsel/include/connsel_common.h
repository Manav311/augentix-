#ifndef CONNSEL_COMMON_H
#define CONNSEL_COMMON_H


#define CONNSEL_RTF_UP            0x0001
#define CONNSEL_IFNAME_LEN        64
#define CONNSEL_THREAD_PERIOD     (1000) /* in unit of ms */
#define CONNSEL_ROUTE_LIST_NUM    4


typedef enum {
	CONNSEL_STATE_NONE,
	CONNSEL_STATE_OFF,
	CONNSEL_STATE_ON,
	CONNSEL_STATE_NUM,
} connsel_state;

typedef struct {
	char name[CONNSEL_IFNAME_LEN];
	int metric;
	unsigned long net;
	unsigned long gw;
	unsigned long mask;
} connsel_route_entries;

typedef struct {
	char *name;
	void *data;
	void (*init)(void);
	void (*deinit)(void);
	void (*run)(void);
} connsel_iface_entries;


#endif /* !CONNSEL_COMMON_H */
