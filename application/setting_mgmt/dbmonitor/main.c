#define _GNU_SOURCE

#include <stdio.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "sqlite3.h"
#include "agtx_types.h"

#define EPOLL_MAX_EVENTS    32

#define BUFFER_SIZE     1024
#define ARRAY_LENGTH    128
#define NAME_LENGTH     128

#define MAX_TIME (30 * 60 * 1000)
#define PER_SLEEP_TIME (10)

#define DBMON_THREAD_NAME "dbmonitor_th"

#define UPDATE_FLAG_FILE "/usrdata/update_file"
#define MERGEDB_DONE_FLAG_FILE "/tmp/db_merge_done"
#define BACKUP_PERIOD_USEC (5 * 1000 * 1000)

static struct epoll_event gEpollEventArray[EPOLL_MAX_EVENTS];
static char *src_file;
static char *dest_file;
pthread_t backup_processor_t;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;
pthread_t backup_thread;
int in_thread = 0;
int endapp_flag = 0;
const char *end_backupdb_path = "/tmp/end_backupdb";


int backupdb(const char *, const char *);
int backup_done = 0;

struct inotify_data_type {
	int fd;
	char self_type[16];
};

/*
 * Check if a file exist using stat() function
 * return 1 if the file exist otherwise return 0
 */

int fileexists(const char* path){
    struct stat buffer;
    int exist = stat(path,&buffer);

    if(exist == 0)
        return 1;
    else
        return 0;
}


int create_timestamp_file(const char *path)
{
    FILE *fp;
	time_t current_time;
    char* c_time_string;

	fp = fopen(path,"w");

	if(!fp)
	{
	   printf("can not create %s\n",path);
	   return -1;
	}

    current_time = time(NULL);

    if (current_time != ((time_t)-1))
    {
		c_time_string = ctime(&current_time);

       if (c_time_string != NULL)
       {
          fprintf(fp,"%s",c_time_string);
       }
	   else
	   {
	      fprintf(fp,"end_backupdb");
	   }

    }
    else
    {
    	fprintf(fp,"end_backupdb");
    }

    fclose(fp);

    return 0;
}


int delete_file(const char *path)
{
   int status;

   if(fileexists(path) == 0)
   	  return 0;

   status = remove(path);

   if(status != 0)
   {
      printf("unable to delete %s\n",path);
	  return -1;
   }
   else
   {
      return 0;
   }
}

void term_handler(int signum)
{
	AGTX_UNUSED(signum);

	endapp_flag = 1;

	sem_post(&semaphore);

	while (backup_done == 0) {
		usleep(1000 * 1000);
		printf("### %s:() backup_done = %d\n", __func__, backup_done);
	}
}

time_t get_cur_time() {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	t = mktime(&tm);

	return t;
}

time_t get_file_time(const char *path) {
	time_t t;
	struct tm *tm;
	struct stat statbuf;

	if (path == NULL) {
		return (time_t) 0;
	}

	stat(path, &statbuf);
	tm = localtime(&statbuf.st_mtime);
	t = mktime(tm);

	return t;
}


int watch_file(const char *file)
{

    int file_flag = 1;
    int total_time = 0;

	printf("watch %s\n",file);

	do
	{
        if(fileexists(file))
        {
           file_flag = 0;
		   break;
        }

		usleep(PER_SLEEP_TIME);
		total_time += PER_SLEEP_TIME;

	}while (total_time <= MAX_TIME);


	return file_flag;

}

void do_ota(const char *src_file, const char *dest_file)
{


     if(watch_file(MERGEDB_DONE_FLAG_FILE) == -1)
     {
          printf("error:backup db to timeout!!\n");
     }

	 if(backupdb(src_file,dest_file) == -1)
	 {
	    printf("error:backup db fail!!\n");
	 }

}

void *backup_processor(void *data) {
	AGTX_UNUSED(data);

	int i = 0;
	time_t cur_time;
	int retry_count = 30;

	pthread_detach(pthread_self());

wait:
	backup_done = 0; //test
	usleep(BACKUP_PERIOD_USEC);
	sem_wait(&semaphore);

	printf("start to backup database\n");

again:

	cur_time = get_cur_time();

	for (i = 0; i < retry_count; i++) {
		if (backupdb(src_file, dest_file) == -1) {
			printf("backup database fail!!!\n");
			usleep(100);
		} else {
			backup_done = 1; //test
			break;
		}
	}

	if (get_file_time(src_file) > cur_time) {
		goto again;
	}

	if (endapp_flag == 1) {
		goto end;
	}

	goto wait;

end:

	printf("backupdb thread end!!!\n");
	pthread_exit(NULL);

}

int backupdb(const char *src_file, const char *dest_file) {
	int rc, rc1;
	sqlite3 *pSb;
	sqlite3 *pFile;
	sqlite3_backup *pBackup;
	int res = 0;

	rc1 = sqlite3_open(src_file, &pSb);

	if (rc1 != SQLITE_OK) {
		printf("open %s failure!!!\n", src_file);
		goto end;
	}

	rc = sqlite3_open(dest_file, &pFile);

	if (rc == SQLITE_OK) {
		pBackup = sqlite3_backup_init(pFile, "main", pSb, "main");

		if (pBackup) {

		    delete_file(end_backupdb_path);

			while((rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK){}

			sqlite3_backup_finish(pBackup);

			if( rc==SQLITE_DONE ){
     			 res = 0;
				 //printf("update db successfully\n");
    		}else{
				printf("Error: %s\n", sqlite3_errmsg(pFile));
      			res = -1;
    		}

		}
	}

end:

	sqlite3_close(pSb);
	sqlite3_close(pFile);
	create_timestamp_file(end_backupdb_path);
	return res;


}

static int add_to_epoll(int epoll_fd, int file_fd) {
	int result;
	struct epoll_event eventItem;
	memset(&eventItem, 0, sizeof(eventItem));
	eventItem.events = EPOLLIN;
	eventItem.data.fd = file_fd;
	result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, file_fd, &eventItem);
	return result;

}

static void __attribute__((unused)) remove_epoll(int epoll_fd, int file_fd)
{
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, file_fd, NULL);
}

static int inotify_event_handler(int epoll_fd, int notify_fd) {
	AGTX_UNUSED(epoll_fd);

	char InfoBuf[BUFFER_SIZE];
	struct inotify_event *event;
	char *p;

	memset(InfoBuf, 0, BUFFER_SIZE);

	int result = read(notify_fd, InfoBuf, BUFFER_SIZE);

	for (p = InfoBuf; p < InfoBuf + result;) {
		event = (struct inotify_event *) (p);

		if (event->mask & IN_CREATE) {

			printf("add file:%s\n", event->name);

		} else if (event->mask & IN_MODIFY) {

			printf("modify file:%s\n", event->name);
			sem_post(&semaphore);

		} else //delete file
		{

		}

		p += sizeof(struct inotify_event) + event->len;
	}

	return 0;
}

int main(int argc, char** argv) {

	int mInotifyId;
	int mEpollId;
	char buf[128] = { 0 };
	const char th_name[] = DBMON_THREAD_NAME;


	if (argc != 3) {
		printf("Paramter Error\n");
		exit(1);
	}

	src_file = argv[1];
	dest_file = argv[2];


	if(fileexists(UPDATE_FLAG_FILE))
	{
	    printf("dbmonitor:do ota\n");
        do_ota(src_file,dest_file);
	}



	/* SIG handler  */

	signal ( SIGTERM, term_handler );
	signal ( SIGQUIT, term_handler );
	signal ( SIGKILL, term_handler );
	signal ( SIGHUP, term_handler );
	signal ( SIGINT, term_handler );



	create_timestamp_file(end_backupdb_path);

	sprintf(buf, "touch %s", src_file);

	system(buf);

	sem_init(&semaphore, 0, 0);



	mEpollId = epoll_create(1);

	if (mEpollId == -1) {
		printf("Epoll Create Error\n");
		exit(1);
	}

	mInotifyId = inotify_init();

	int result = inotify_add_watch(mInotifyId, src_file, IN_MODIFY);

	if (result == -1) {
		printf("File add watch failure!\n");
		exit(1);
	}

	if (pthread_create(&backup_thread, NULL, backup_processor, NULL) < 0) {
		close(mInotifyId);
		exit(1);
	}
	pthread_setname_np(backup_thread, th_name);

	add_to_epoll(mEpollId, mInotifyId);

	while (1) {
		result = epoll_wait(mEpollId, gEpollEventArray, EPOLL_MAX_EVENTS, -1);

		if (result == -1 || errno == EINTR) {
			int j;
			for (j = 0; j < 10; j++) {
				printf("db stop\n");
				goto end;
			}
		} else {
			printf("file event happen\n");
			int i = 0;

			for (i = 0; i < result; i++) {
				if (gEpollEventArray[i].data.fd == mInotifyId) {

					if (inotify_event_handler(mEpollId, mInotifyId) == -1) {
						printf("inotify handler error\n");
						return -1;
					}
				} else {

				}
			}
		}
	}

end:
	endapp_flag = 1;
	sem_post(&semaphore);
	pthread_join(backup_thread, NULL);
	printf("db monitor end\n");
	return 0;
}

