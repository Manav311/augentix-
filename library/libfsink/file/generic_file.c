#include "generic_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "fsink.h"

#define MAX_NAME_LENG 256

/** @brief Open an File Sink.
 *      Return 0 upon completion. Otherwise, return -1 when the file is
 *      not opened, and errno is set appropriately.
 *
 *  The function use the info previously stored in FileAttr to create a File Sink.
 *  @param[in, out] p        Pointer of the input FileAttr.
 *  @param[in]      flags    File access flags; unused for fopen here.
 */
static int openFileSink(void *p, int flags __attribute__((unused)))
{
	FileAttr *file_attr = (FileAttr *)p;
	SSD_PRINTF("%s\n", __func__);

	/* 'w' = O_WRONLY | O_CREAT | O_TRUNC, and 'b' is ignored in POSIX-conforming systems */
	file_attr->fptr = NULL;
	if (file_attr->fname == NULL) {
		return -1;
	}

	file_attr->fptr = fopen(file_attr->fname, "wb");
	return 0;
}

/** @brief Write data to an File Sink.
 *      Return size of data written to the file.
 *      Otherwise, return -1 on error, and errno is set appropriately.
 *
 *  @param[in] p     Pointer of the input FileAttr.
 *  @param[in] src   Pointer of the starting address of the source to be copied.
 *  @param[in] nbyte Number of bytes to be copied from src.
 */
size_t writeFileSink(void *p, void *src, size_t nbyte)
{
	FileAttr *file_attr = (FileAttr *)p;
	if (file_attr->fptr == NULL)
		return 0;
	return fwrite(src, sizeof(char), nbyte, file_attr->fptr);
}

/** @brief Close an File Sink.
 *      Return 0 upon completion.
 *      Otherwise, return -1 on error, and errno is set appropriately.
 *
 *  @param[in] p     Pointer of the input FileAttr.
 */
static int closeFileSink(void *p)
{
	int ret = 0;
	FileAttr *file_attr = (FileAttr *)p;
	SSD_PRINTF("%s\n", __func__);

	if (file_attr->fptr != NULL) {
		ret = fclose(file_attr->fptr);
		if (ret != 0) {
			perror("Failed to close file!");
			return ret;
		}
		file_attr->fptr = NULL;
	}

	return ret;
}

/** @brief Create an File Sink.
 *      Return the pointer to the created File Sink upon completion.
 *      Otherwise, return NULL on error, and errno is set appropriately.
 *
 *  The function creates File Sink, SinkOps and FileAttr,
 *  apply the input file name to the FileAttr,
 *  and binds them together in the created File Sink.
 *  @param[in] fname   Name of the created file.
 *  @param[in] flags   Unused.
 */
Sink *createFileSink(const char *const fname, const int flags)
{
	Sink *file_sink;
	SinkOps *file_sink_ops;
	FileAttr *file_sink_attr;

	file_sink = malloc(sizeof(*file_sink));
	if (file_sink == NULL) {
		perror("Failed to allocate File Sink!");
		goto exit_err;
	}

	file_sink->name = malloc(MAX_NAME_LENG * sizeof(*file_sink->name));
	if (file_sink->name == NULL) {
		perror("Failed to allocate File Sink name!");
		goto exit_name;
	}

	file_sink_ops = malloc(sizeof(*file_sink_ops));
	if (file_sink_ops == NULL) {
		perror("Failed to allocate file_sink_ops!");
		goto exit_ops;
	}
	/* Binding actual ops functions */
	{
		file_sink_ops->open = openFileSink;
		file_sink_ops->write = writeFileSink;
		file_sink_ops->close = closeFileSink;
	}

	file_sink_attr = malloc(sizeof(*file_sink_attr));
	if (file_sink_attr == NULL) {
		perror("Failed to allocate file_sink_attr!");
		goto exit_attr;
	}

	/* set file attributes */
	if (strlen(fname) != 0) {
		strcpy(file_sink->name, fname);
		file_sink_attr->fname = file_sink->name;
	} else {
		printf("[FileSink] Warning: File name is not assigned; Drop all video frames.\n");
		file_sink->name = NULL;
		file_sink_attr->fname = NULL;
	}
	file_sink_attr->flags = flags;

	file_sink->info = (void *)file_sink_attr;
	file_sink->ops = file_sink_ops;

	SSD_PRINTF("File sink created.\n");

	return file_sink;

exit_attr:
	free(file_sink_ops);
exit_ops:
	free(file_sink->name);
exit_name:
	free(file_sink);
exit_err:
	return NULL;
}

/** @brief Destroy an File Sink by freeing the allocated memory spaces.
 *
 *  @param[in] file_sink     Pointer to the File Sink to be destroyed.
 */
void releaseFileSink(Sink *file_sink)
{
	free(file_sink->info);
	free(file_sink->ops);
	free(file_sink->name);
	free(file_sink);
	SSD_PRINTF("File Sink released.\n");
}
