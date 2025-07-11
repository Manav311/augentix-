#ifndef FSINK_H
#define FSINK_H

#include <stddef.h>

/** @brief Struct that contains Sink operation functions.
 *      The struct contains function pointers to be assigned.
 *      The actual behavior of the functions should be defined by programmers.
 */
typedef struct SinkOpsType {
	int (*open)(void *p, int flags);
	size_t (*write)(void *p, void *src, size_t nbyte);
	int (*close)(void *p);
} SinkOps;

/** @brief Struct of Sink, the abstract bitstream output.
 */
typedef struct SinkType {
	char *name; /**< Name of the Sink. */
	void *info; /**< Pointer of programmer-defined Sink information. */
	SinkOps *ops; /**< Pointer of SinkOps, a struct of programmer-defined Sink operations. */
} Sink;

#endif /* _SINK_OPS_H */
