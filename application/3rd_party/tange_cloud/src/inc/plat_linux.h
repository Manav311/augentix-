#ifndef __plat_linux_h__
#define __plat_linux_h__

#include "basedef.h"

#if defined(__LINUX__) || defined(__ANDROID__) || defined(__MAC_OS__) || defined(__CYGWIN__)
//
// OS:  Linux / Android / Free BSD (MAC OS)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include <poll.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>
#ifdef __MAC_OS__
#include <dispatch/dispatch.h>
#else
#include <sys/prctl.h>
#include <semaphore.h>
#include <sys/vfs.h>
#endif

#include <assert.h>
#define _ASSERT(x) assert(x)

/* file system */
#define SA_FILE FILE
#define SA_fopen fopen
#define SA_fclose fclose
#define SA_fread fread
#define SA_fwrite fwrite
#define SA_fseek fseek
#define SA_rewind(fp) rewind(fp)
#define SA_ftell(fp) ftell(fp)
#define SA_feof(fp) feof(fp)
#define SA_remove remove
#define SA_rmdir(path) rmdir(path)
#define SA_fflush(fp) fflush(fp)
#define SA_fprintf fprintf
#define SA_setbuffer(fp, buff, size) setbuffer(fp, buff, size)
#define SA_sync() sync()

#define SA_DECL_FGETS_BUFFER(buff, size)
#define SA_fgets(buff, size, fp) fgets(buff, size, fp)

#define SA_srand(x) srandom(x)
#define SA_rand() random()

#ifdef __cplusplus
extern "C" {
#endif

#define SA_Perror(x) perror(x)

/*
 *  SOCKET Functions
 */
#define SA_NetLibInit()
#define SA_NetLibUninit()

#define SA_IOVEC struct iovec
#define SA_IoVecGetPtr(pvec) ((pvec)->iov_base)
#define SA_IoVecGetLen(pvec) ((pvec)->iov_len)
#define SA_IoVecSetPtr(pvec, ptr) (pvec)->iov_base = (void *)(ptr)
#define SA_IoVecSetLen(pvec, len) (pvec)->iov_len = len

#define SA_SocketClose close
#define SA_Send(s, data, len, flag) send(s, data, len, flag | MSG_NOSIGNAL)
#define SA_SendTo sendto
#define SA_Recv recv
#define SA_RecvFrom(s, buf, size, flags, paddr, paddr_len) recvfrom(s, buf, size, flags, paddr, (socklen_t *)paddr_len)
#define SA_GetSockName(s, paddr, paddr_len) getsockname(s, paddr, (socklen_t *)paddr_len)
#define SA_GetPeerName(s, paddr, paddr_len) getpeername(s, paddr, (socklen_t *)paddr_len)
#define SA_Accept(s, paddr, paddr_len) accept(s, paddr, (socklen_t *)paddr_len)
#define SA_GetSockOpt(s, level, optname, optval, optlen) getsockopt(s, level, optname, optval, (socklen_t *)optlen)
#define SA_SetSockOpt setsockopt
#define SA_shutdown shutdown

#define SA_SOCKET int
#define INVALID_SOCKET -1
#define SA_SocketIsValid(s) (s >= 0)
#define SA_SocketGetError(s) errno
#define SA_SocketSetError(s, err) errno = err
#define SA_SOCKET_ERROR -1 //return value of socket operations
#define SA_SocketStrError(err) strerror(err)

int poll_fd(int fd, int *events, int ms);

/*
 *
 */
#define SA_IsValidHandle(fd) (fd >= 0)
#define SA_INVALID_HANDLE -1

/*
 *  Synchronous Objects
 */
#define SA_MUTEX pthread_mutex_t
#define SA_PIPE int

#define SA_DEFINEMUTEX(x) pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
#define SA_MutexInit(x) pthread_mutex_init(&x, NULL)
#define SA_MutexUninit(x) pthread_mutex_destroy(&x)
#define SA_MutexLock(x) pthread_mutex_lock(&x)
#define SA_MutexUnlock(x) pthread_mutex_unlock(&x)
#define SA_MutexTryLock(x) (pthread_mutex_trylock(&x) == 0)

#ifdef HAVE_SPIN_T
#define SA_SPIN pthread_spinlock_t
#define SA_SpinInit(x) pthread_spin_init(&x, PTHREAD_PROCESS_PRIVATE)
#define SA_SpinUninit(x) pthread_spin_destroy(&x)
#define SA_SpinLock(x) pthread_spin_lock(&x)
#define SA_SpinTryLock(x) pthread_spin_trylock(&x)
#define SA_SpinUnlock(x) pthread_spin_unlock(&x)
#else
#define SA_SPIN pthread_mutex_t
#define SA_SpinInit(x) pthread_mutex_init(&x, NULL)
#define SA_SpinUninit(x) pthread_mutex_destroy(&x)
#define SA_SpinLock(x) pthread_mutex_lock(&x)
#define SA_SpinTryLock(x) pthread_mutex_trylock(&x)
#define SA_SpinUnlock(x) pthread_mutex_unlock(&x)
#endif

#ifdef __MAC_OS__
/* semaphore in MAC OS (BSD) */
#define SA_EVENT dispatch_semaphore_t
#define SA_SEM dispatch_semaphore_t

#define SA_EventInit(e)                           \
	do {                                      \
		e = dispatch_semaphore_create(1); \
		dispatch_semaphore_wait(e, 0);    \
	} while (0)
#define SA_EventUninit(e)                     \
	do {                                  \
		dispatch_semaphore_signal(e); \
		dispatch_release(e);          \
	} while (0)
#define SA_EventSet(e) dispatch_semaphore_signal(e)
#define SA_EventWait(e) dispatch_semaphore_wait(e, DISPATCH_TIME_FOREVER)
SA_BOOL __macos_EventWaitTimed(SA_EVENT e, DWORD ms);
#define SA_EventWaitTimed(e, ms) __macos_EventWaitTimed(e, ms)

#define SA_SemInit(sem, max_value)                          \
	do {                                                \
		sem = dispatch_semaphore_create(max_value); \
	} while (0)
#define SA_SemUninit(sem) dispatch_release(sem)
#define SA_SemWait(sem) dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER)
#define SA_SemPost(sem) dispatch_semaphore_signal(sem)

#else
/**/
typedef sem_t SA_EVENT;

#define SA_EventInit(e) sem_init(&e, 0, 0)
#define SA_EventUninit(e) sem_destroy(&e)
#define SA_EventSet(e) sem_post(&e)
#define SA_EventWait(e) sem_wait(&e)
SA_BOOL __linux_EventWaitTimed(SA_EVENT *e, DWORD ms);
#define SA_EventWaitTimed(e, ms) __linux_EventWaitTimed(&e, ms)

typedef sem_t SA_SEM;
#define SA_SemInit(sem, init_value) sem_init(&sem, 0, init_value)
#define SA_SemUninit(sem) sem_destroy(&sem)
#define SA_SemWait(sem) sem_wait(&sem)
#define SA_SemPost(sem) sem_post(&sem)
#endif

/*
 *  Threads
 */
#define SA_HTHREAD pthread_t
#define SA_HTHREAD_NULL 0L
#define SA_HTHREAD_IS_VALID(h) (h)
#define SA_HTHREAD_CLEAR(h) h = 0
#define SA_THREAD_RETTYPE void *
#define SA_THREAD_RETVALUE(r) (void *)((long)r)
typedef SA_THREAD_RETTYPE(__STDCALL *SA_ThreadRoutine)(void *);

#define DECL_THREAD(func, param) void *func(void *param)
#define RET_THREAD(v) return (void *)v

SA_HTHREAD linux_ThreadCreateWithStackSize(SA_ThreadRoutine routine, void *arg, int stack_size);
/** SA_BOOL SA_ThreadCreateWithStackSize(SA_HTHREAD handle, SA_ThreadRoutine routine, void *arg, ...) */
#define SA_ThreadCreateWithStackSize(handle, routine, arg, stack_size, name) \
	(handle = linux_ThreadCreateWithStackSize(routine, arg, stack_size))
#define SA_ThreadCreate(handle, routine, arg, name) (handle = linux_ThreadCreateWithStackSize(routine, arg, 0))
#ifdef __MAC_OS__
#define SA_SET_CURRENT_THREAD_NAME(name) pthread_setname_np(name)
#else
#define SA_SET_CURRENT_THREAD_NAME(name) prctl(PR_SET_NAME, name)
#endif

#define SA_ThreadGetCurrentHandle() pthread_self()
#define SA_ThreadCloseHandle(hThread) pthread_detach(hThread)
void *SA_ThreadWaitUntilTerminate(SA_HTHREAD hThread);

int SA_ThreadSuspend(SA_HTHREAD hThread);
int SA_ThreadResume(SA_HTHREAD hThread);

void SA_Sleep(unsigned int ms); //Milliseconds
time_t SA_time(time_t *t);

#define INFINITE 0xFFFFFFFF //for all {timeout} parameters

#ifndef __ANDROID__
/*
 * Read-Write Lock
 */

#define SA_RWLOCK pthread_rwlock_t
#define SA_RWLockInit(x) pthread_rwlock_init(&x, NULL)
#define SA_RWLockUninit(x) pthread_rwlock_destroy(&x)
SA_BOOL _RWLockLockR(SA_RWLOCK *x, DWORD timeout);
SA_BOOL _RWLockLockW(SA_RWLOCK *x, DWORD timeout);
#define SA_RWLockLockR(x) pthread_rwlock_rdlock(&x)
#define SA_RWLockLockW(x) pthread_rwlock_wrlock(&x)
#define SA_RWLockFailed(op) (op != 0)
#define SA_RWLockLockRTimed(x, timeout) _RWLockLockR(&x, timeout)
#define SA_RWLockLockWTimed(x, timeout) _RWLockLockR(&x, timeout)
#define SA_RWLockUnlock(x) pthread_rwlock_unlock(&x)
#endif

/*
 *  String functions
 */
#define SA_StrCaseCmp strcasecmp
#define SA_StrNCaseCmp strncasecmp
#define SA_StrNCmp strncmp

/*
 *  File Operations
 */
#define SA_HFILE int
#define SA_Write write
#define SA_Read read
#define SA_FileIsValid(h) (h >= 0)
#define SA_FileClose(f) close(f)
SA_BOOL SA_DeleteFile(const char *fn);
int SA_GetFileLength(const char *path);
SA_BOOL SA_FileExisted(const char *path);

//typedef mode_t SA_FATTR;
/* return: 0:ok; -1:failed */
int SA_GetFileAttr(const char *path, int *attr);
#define SA_ISREG(attr) S_ISREG(attr)
#define SA_ISDIR(attr) S_ISDIR(attr)

/*
 *  Time
 */
DWORD SA_GetTickCount();
#define SA_Tick2Ms(tick) (tick)

/*
 *  Common Wrapper
 */
int SA_SocketSetNBlk(SA_SOCKET s, SA_BOOL b);
int SA_SocketSetLinger(SA_SOCKET s, int onoff, int linger);

/*
 *  Pipe functions
 */
SA_BOOL SA_PipeCreate(SA_PIPE *pHPipeRd, SA_PIPE *pHPipeWrt);
SA_BOOL SA_PipeClose(SA_PIPE hPipe);

typedef struct {
	int pipefd[2];
} SA_QUEUE, SA_HQUEUE;
SA_BOOL SA_QueueCreate(SA_HQUEUE *pHQueue);
SA_BOOL SA_QueueDestroy(SA_HQUEUE hQueue);
int SA_QueueWrite(SA_HQUEUE hQueue, const void *mem, int size);
int SA_QueueRead(SA_HQUEUE hQueue, void *mem, int size);
int SA_QueueReadTimed(SA_HQUEUE hQueue, void *mem, int size, unsigned int timeout);

/*
 *  Debug
 */
#ifdef _DEBUG
#define dbg_msg printf
#define PRINTF printf
void dbg_bin(const char *title, const void *p, int size);
#else
#define dbg_msg(fmt, args...)
#define dbg_bin(x, y, z)
#endif

#ifdef __ANDROID__
#include <android/log.h>
void android_log(int level, const char *tag, const char *sfmt, ...);
#define LOG(sfmt, args...) android_log(ANDROID_LOG_INFO, __FUNCTION__, sfmt, ##args)
#define LOGW(sfmt, args...) android_log(ANDROID_LOG_WARN, __FUNCTION__, sfmt, ##args)
#define LOGE(sfmt, args...) android_log(ANDROID_LOG_ERROR, __FUNCTION__, sfmt, ##args)

#undef dbg_msg
#define dbg_msg LOG

#else //if defined(__LINUX__) || defined(__MAC_OS__)

#define LOG(sfmt, args...)             \
	do {                           \
		dbg_msg(sfmt, ##args); \
		dbg_msg("\n");         \
	} while (0)
#define LOGW(sfmt, args...)            \
	do {                           \
		dbg_msg(sfmt, ##args); \
		dbg_msg("\n");         \
	} while (0)
#define LOGE(sfmt, args...)            \
	do {                           \
		dbg_msg(sfmt, ##args); \
		dbg_msg("\n");         \
	} while (0)

#endif

#include <dirent.h>

//Get total/free space. Unit: M
int SA_GetDiskSpace(const char *root_path, uint32_t *total, uint32_t *free);

typedef DIR *SA_DIRSCAN;
typedef struct dirent *SA_DENTRY;

/** SA_BOOL SA_ScanBegin(SA_DIRSCAN scan, const char *path);
 * @param ext !!!
 */
#define SA_ScanBegin(scan, path) (scan = opendir(path))

//ext: extension
//有的实现很烂的文件系统要求提供扫描文件的扩展名, 但在linux/liteos下不需要此参数。
//为了兼容, 写成带扩展名的形式
#define SA_ScanBegin_(scan, path, ext) (scan = opendir(path))

/** SA_ScanEnd(SA_DIRSCAN scan) */
#define SA_ScanEnd(scan) closedir(scan)

/** SA_BOOL SA_ScanNext(SA_DIRSCAN scan, SA_DENTRY de); 
 */
#define SA_ScanNext(scan, de) (de = readdir(scan))

/** const char *SA_DENAME(SA_HDENTRY de);
 */
#define SA_DENAME(de) de->d_name

#ifdef __cplusplus
}
#endif

#endif //__LINUX__/__MAC_OS__/__ANDROID__

#endif //#ifndef __plat_linux_h__
