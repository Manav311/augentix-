
#include <dirent.h>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

using namespace std;
/* DUMP_FLAG =  0 stop Write,
 * DUMP_FLAG =  1 write to file Descriptor,
 * DUMP_FLAG =  2 write all  p frames till next iFrame and set DUMP_FLAG to 0
 * DUMP_FLAG =  3 file has no more write operations and has to be closed and set
 * DUMP_FLAG = 0 later */
extern int DUMP_FLAG;
extern FILE *xh264DumpFile;
extern char h264DumpFileName[48];
extern int SPLIT_FILE_SIZE; // Split save files size if set to 0  stop dump

void dumpH264Stream2File(int sig);
