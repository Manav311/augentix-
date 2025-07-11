#ifndef _SAMPLE_STREAM_H
#define _SAMPLE_STREAM_H

#include <cstdlib>
#include <pthread.h>
#include <string>

extern "C" {
#include "mpi_base_types.h"
#include "mpi_enc.h"
#include "mpi_sys.h"

extern int g_verbos_flag;
};

/**
 * @brief C++ encapsulation for functions in sample_sys.c
 **/

class SampleStream {
    public:
	static SampleStream *createNew(const MPI_PATH path_id, const MPI_ECHN chn_id);
	~SampleStream();

	INT32 setupStream(MPI_PATH path_idx, MPI_ECHN chn_idx);
	INT32 releaseStream(MPI_PATH path_idx, MPI_ECHN chn_idx);

	INT32 openStreamChannel(MPI_ECHN chn_idx);
	INT32 closeStreamChannel(MPI_ECHN chn_idx);

	MPI_PATH getPathIdx();
	MPI_ECHN getChnIdx();

	/*
  INT32 startStream(MPI_CHN chn_idx);
  void stopStream(MPI_CHN chn_idx);
  */

	unsigned readBytes(void *buffer, MPI_ECHN chn_idx, unsigned num_bytes);
	size_t readNextBuffer(void *buffer, MPI_ECHN chn_idx);
	size_t readNextFrame(void *buffer, MPI_ECHN chn_idx);

    protected:
	SampleStream(const MPI_PATH, const MPI_ECHN);

    private:
	void setStreamThread(void *p);
	void releaseStreamThread(pthread_t *tid);

	// std::string fname;
	MPI_PATH path_idx;
	MPI_ECHN chn_idx;
	MPI_BCHN bchn_idx;
	MPI_STREAM_PARAMS_S stream_params;

	unsigned char *fBuf;
	unsigned char *fBufPivot;
	unsigned numBytesStored;
	/*
  size_t buffer_size;
  pthread_t tid;
  */
};

#endif
