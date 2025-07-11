#define _GNU_SOURCE
#include "alarmout.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "agtx_types.h"

#define ALARMD_THREAD_GPIOD_NAME "alarmd_gpiod"

struct alarm_out_rule g_rule_head;

void get_gpio_table_timeout(struct timeval *timeout)
{
	struct alarm_out_rule *pos;
	struct timeval min_tv;

	min_tv.tv_sec = SEC_MAX;
	min_tv.tv_usec = 0;

	if (g_rule_head.next == NULL) {
		goto assign_timeout;
	}


	for (pos = g_rule_head.next; pos != NULL; pos = pos->next) {
		if (timercmp(&pos->duration, &min_tv, <)) {
			min_tv = pos->duration;
		}
	}

assign_timeout:
	*timeout = min_tv;
}

void update_gpio_table_time(struct timeval *tv)
{
	struct alarm_out_rule *pos;
	struct timeval diff_tv;

	if (g_rule_head.next == NULL) {
		return;
	}

	debug("%s: sec %d usec %d\n", __func__, (int)tv->tv_sec, (int)tv->tv_usec);
	for (pos = g_rule_head.next; pos != NULL; pos = pos->next) {
		if (timercmp(&pos->duration, tv, >)) {
			timersub(&pos->duration, tv, &diff_tv);
			pos->duration = diff_tv;
		} else {
			timerclear(&pos->duration);
		}
	}
}

int is_gpo(struct alarm_out_rule *rule)
{
	FILE *fp;
	char dir_path[80];
	char str[4];

	sprintf(dir_path, "%sgpio%d/direction", GPO_PATH, rule->gpio);

	fp = fopen(dir_path, "r");
	if (!fp) {
		return 0;
	}
	fscanf(fp, "%s", str);
	if (strcmp(str, "out")) {
		return 0;
	}
	fclose(fp);

	return 1;
}

void set_gpo_value(int8_t gpio, uint8_t polarity)
{
	FILE *fp;
	char val_path[80];

	sprintf(val_path, "%sgpio%d/value", GPO_PATH, gpio);

	fp = fopen(val_path, "wr");
	if (!fp) {
		return;
	}
	fprintf(fp, "%d", polarity);
	fclose(fp);
}

int update_gpio_table(struct alarm_out_rule *rule)
{
	struct alarm_out_rule *pre, *pos;
	struct alarm_out_rule *new;

	pre = &g_rule_head;
	if (g_rule_head.next == NULL) {
		goto add_rule;
	}

	for (pos = g_rule_head.next; pos != NULL; pos = pos->next) {
		debug("%s: pos GPIO %d, polarity %d, duration %d.%d\n", __func__,
		      pos->gpio, pos->polarity, (int)pos->duration.tv_sec, (int)pos->duration.tv_usec);
		if (pos->gpio == rule->gpio) {
			if (pos->polarity != rule->polarity) {
				printf("%s> Violating previous rule, polarity %d != %d\n", __func__, pos->polarity, rule->polarity);
				return 1;
			}
			if ((rule->duration.tv_sec > pos->duration.tv_sec) ||
				(rule->duration.tv_sec == pos->duration.tv_sec &&
			    rule->duration.tv_usec > pos->duration.tv_usec)) {
				*pos = *rule;
			}
			return 0;
		}
		pre = pos;
	}

add_rule:
	new = malloc(sizeof(struct alarm_out_rule));
	memcpy(new, rule, sizeof(struct alarm_out_rule));
	pre->next = new;
	/* toogle on gpio */
	set_gpo_value(new->gpio, new->polarity);
	return 0;
}

void update_gpio()
{
	struct alarm_out_rule *pre, *pos;

	if (g_rule_head.next == NULL) {
		return;
	}

	pre = &g_rule_head;
	for (pos = g_rule_head.next; pos != NULL; pos = pos->next) {
		if(pos->duration.tv_sec == 0 && pos->duration.tv_usec == 0) {
			/* turn off GPO */
			set_gpo_value(pos->gpio, pos->polarity^1);
			/* free rule */
			pre->next = pos->next;
			free(pos);
			pos = pre;
		}
		pre = pos;
	}
}

void *gpio_daemon(void *data)
{
	AGTX_UNUSED(data);
	int ret;
	int sockfd;
	struct sockaddr_un addr;
	struct alarm_out_rule *almo_rule;
	char *buffer;
	struct timeval timeout, remain_timeout, diff_tv;
	int fdmax;
	fd_set master, read_fds;

	buffer = malloc(sizeof(struct alarm_out_rule));
	memset(buffer, 0, sizeof(struct alarm_out_rule));
	memset(&addr, 0, sizeof(addr));

	g_rule_head.gpio = -1;
	g_rule_head.next = NULL;

	sockfd = socket(PF_UNIX, SOCK_STREAM, 0);

	debug("%s> sockfd = %d\n", __func__, sockfd);

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, ALARM_OUT_SOCKET);

	unlink(ALARM_OUT_SOCKET);

	ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret) {
		perror("bind");
		goto failed;
	}

	ret = listen(sockfd, 10);
	if (ret) {
		goto failed;
	}
	timerclear(&timeout);
	timerclear(&remain_timeout);
	timerclear(&diff_tv);

	FD_ZERO(&master);
	FD_SET(sockfd, &master);
	fdmax = sockfd;

	while(1) {
		int i;
		int clientfd;
		struct sockaddr_un client_addr;
		socklen_t addrlen = sizeof(client_addr);

		/* 1. get timeout from GPIO table */
		get_gpio_table_timeout(&timeout);
		remain_timeout = timeout;

		debug("timeout sec = %d, usec = %d\n", (int)timeout.tv_sec, (int)timeout.tv_usec);

		/* 2. listen fd                       */
		/*      success -> update gpio table  */
		/*      timeout -> goto timeout       */
		read_fds = master;
		ret = select(fdmax + 1, &read_fds, NULL, NULL, &remain_timeout);
		switch (ret) {
			case 0:
				debug("%s: select timeout\n", __func__);

				/* 4. update time in gpio table */
				update_gpio_table_time(&timeout);
				goto timeout;
				break;
			case 1:
				/* 3. recieve data from socket */
				i = sockfd;
				if (FD_ISSET(i, &read_fds)) {
					//				if (i == sockfd) {
					clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
					if (clientfd < 0) {
						perror("accept");
						break;
					}

					recv(clientfd, buffer, sizeof(struct alarm_out_rule), 0);
					close(clientfd);
				}
				almo_rule = (struct alarm_out_rule *)buffer;
				almo_rule->next = NULL;

				timersub(&timeout, &remain_timeout, &diff_tv);

				/* 4. update time in gpio table */
				update_gpio_table_time(&diff_tv);

				/* 5. update gpio table */
				if (is_gpo(almo_rule)) {
					ret = update_gpio_table(almo_rule);
					if (ret) {
						printf("%s: update gpio table failed\n", __func__);
					}
				} else {
					printf("%s: GPIO%d is not a GPO\n", __func__, almo_rule->gpio);
				}

				break;
			default:
				perror("select");
				goto failed;
				break;
		}

timeout:
		/* 6. set gpio by table */
		update_gpio(&diff_tv);
	}

failed:
	close(sockfd);
	free(buffer);

	return NULL;
}

int main()
{
	pthread_t gpiod_thread;
	int ret;

	ret = pthread_create(&gpiod_thread, NULL, gpio_daemon, ALARMD_THREAD_GPIOD_NAME);
	if (ret)
	{
		fprintf(stderr, "pthread_create() reutrn %d\n", ret);
		exit(EXIT_FAILURE);
	}
	pthread_setname_np(gpiod_thread, ALARMD_THREAD_GPIOD_NAME);

	pthread_join(gpiod_thread, NULL);
	return 0;
}
