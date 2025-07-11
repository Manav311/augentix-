
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <sys/file.h>

#include "agtx_types.h"

#include "connsel_def.h"
#include "connsel_log.h"
#include "connsel_common.h"
#include "connsel_utils.h"


extern const connsel_iface_entries connsel_iface_table[];
extern const int connsel_iface_table_size;


#define CONNSEL_DAEMON_THREAD_NAME  "connsel_daemon"


typedef struct {
	pthread_t tid;
} connsel_ctrl;


static int g_lock_fd = -1;
static connsel_ctrl g_connsel_ctrl;


static void* connsel_daemon_thread(void *data)
{
	AGTX_UNUSED(data);
	int i;
	const connsel_iface_entries *iface = NULL;

	while (1) {
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0) {
			connsel_info("Set cancelstate DISABLE failed.\n");
		}

		for (i = 0; i < connsel_iface_table_size; ++i) {
			iface = &connsel_iface_table[i];

			if (iface->run) {
				iface->run();
			}
		}

		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0) {
			connsel_info("Set cancelstate ENABLE failed.\n");
		}

		pthread_testcancel();

		usleep(CONNSEL_THREAD_PERIOD * 1000); /* Sleep 1000 ms */
	}

	pthread_exit(0);
}

static void connsel_init_ctrl(connsel_ctrl *p)
{
	p->tid = -1;
}

static void connsel_deinit_ctrl(connsel_ctrl *p)
{
	p->tid = -1;
}

static int connsel_checkInstance(const char *lock_file)
{
	int ret = 0;
	int lock_fd;

	lock_fd = open(lock_file, O_CREAT | O_RDWR, 0600);
	if (lock_fd < 0) {
		connsel_info("Failed to open lock file\n");
		return -1;
	}

	ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if (ret && EWOULDBLOCK == errno) {
		 connsel_info("%s is already running ...!\n", CONNSEL_IDENT);
		 connsel_info("To restart it, you have to kill the program and delete the lock file %s\n",
			       lock_file);
		 close(lock_fd);
		 return -1;
	}

	connsel_info("Starting %s ...\n", CONNSEL_IDENT);

	g_lock_fd = lock_fd;

	return 0;
}

static void connsel_removeInstance(int code)
{
	int lock_fd = g_lock_fd;
	char *str = (code == 0) ? "normal terminate" : "signal caught";

	if (lock_fd >= 0) {
		flock(lock_fd, LOCK_UN);
		close(lock_fd);
	}

	g_lock_fd = -1;

	connsel_info("Stop %s due to %s\n", CONNSEL_IDENT, str);
}

static void connsel_handleSigno(int signo)
{
	int i, ret;
	const connsel_iface_entries *iface = NULL;
	connsel_ctrl *p = &g_connsel_ctrl;

	if (signo == SIGINT) {
		connsel_info("Caught %s!\n", strsignal(signo));
	} else if (signo == SIGTERM) {
		connsel_info("Caught %s!\n", strsignal(signo));
	} else if (signo == SIGQUIT) {
		connsel_info("Caught %s!\n", strsignal(signo));
	} else if (signo == SIGHUP) {
		connsel_info("Caught %s!\n", strsignal(signo));
	} else if (signo == SIGSEGV) {
		connsel_info("==> Segmentation fault! <==\n");
		exit(1);
	} else {
		connsel_info("Unexpected signal!\n");
		exit(1);
	}

	pthread_cancel(p->tid);

	ret = pthread_join(p->tid, NULL);
	connsel_info("Join connsel daemon thread, ret %d\n", ret);

	for (i = connsel_iface_table_size - 1; i >= 0; i--) {
		iface = &connsel_iface_table[i];

		if (iface->deinit) {
			iface->deinit();
		}
	}

	connsel_deinit_ctrl(p);

	connsel_removeInstance(1);
	connsel_exitLog();

	exit(0);
}

static int connsel_regSignalHandler(void)
{
	if (signal(SIGINT, connsel_handleSigno) == SIG_ERR) {
		connsel_info("Failed to register callback for SIGINT\n");
		return -1;
	}

	if (signal(SIGTERM, connsel_handleSigno) == SIG_ERR) {
		connsel_info("Failed to register callback for SIGTERM\n");
		return -1;
	}

	if (signal(SIGQUIT, connsel_handleSigno) == SIG_ERR) {
		connsel_info("Failed to register callback for SIGQUIT\n");
		return -1;
	}

	if (signal(SIGHUP, connsel_handleSigno) == SIG_ERR) {
		connsel_info("Failed to register callback for SIGHUP\n");
		return -1;
	}

	if (signal(SIGSEGV, connsel_handleSigno) == SIG_ERR) {
		connsel_info("Failed to register callback for SIGSEGV\n");
		return -1;
	}

	// When the client FD is broken while in Use linux rises SIGPIPE for this app we ignore
//	signal(SIGPIPE, SIG_IGN);

	return 0;
}

int main(void)
{
	int i, ret = 0;
	const connsel_iface_entries *iface = NULL;
	connsel_ctrl *p = &g_connsel_ctrl;

	connsel_initLog(CONNSEL_IDENT);

	ret = connsel_checkInstance(CONNSEL_LOCK_FILE);
	if (ret) {
		goto close_log;
	}

	ret = connsel_regSignalHandler();
	if (ret) {
		goto rm_inst;
	}

	connsel_init_ctrl(p);

	for (i = 0; i < connsel_iface_table_size; i++) {
		iface = &connsel_iface_table[i];

		if (iface->init) {
			iface->init();
		}
	}

	pthread_create(&p->tid, NULL, connsel_daemon_thread, p);
	pthread_setname_np(p->tid, CONNSEL_DAEMON_THREAD_NAME);

	ret = pthread_join(p->tid, NULL);
	connsel_info("Join connsel daemon thread, ret %d\n", ret);

	for (i = connsel_iface_table_size - 1; i >= 0; i--) {
		iface = &connsel_iface_table[i];

		if (iface->deinit) {
			iface->deinit();
		}
	}

	connsel_deinit_ctrl(p);

rm_inst:
	connsel_removeInstance(0);

close_log:
	connsel_exitLog();

	return ret;
}
