#include "SampleStream.hh"
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <string>

using namespace std;
//#include "sensor.h"
#define BUFFER_SIZE 1024 * 768
/* C declarations */
extern "C" {

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "sensor.h"

#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_verbos_flag = 0;

INT32 __initSystem(void)
{
	//    INT32 ret = MPI_FAILURE;
	//
	//    printf("__exitSystem %d\n",__LINE__);
	//
	//    ret = MPI_initSystem();
	//    if (ret != MPI_SUCCESS) {
	//        fprintf(stderr, "Failed to initialise MPP system.\n");
	//        return MPI_FAILURE;
	//    }
	//
	return MPI_SUCCESS;
}

INT32 __exitSystem(void)
{
	//    INT32 ret = MPI_FAILURE;
	//
	//    printf("__exitSystem %d\n",__LINE__);
	//
	//    ret = MPI_exitSystem();
	//    if (ret != MPI_SUCCESS) {
	//        fprintf(stderr, "Failed to exit MPP system.\n");
	//        return MPI_FAILURE;
	//    }
	//
	return MPI_SUCCESS;
}

INT32 __initSensor(MPI_PATH path_idx, MPI_ECHN chn_idx)
{
	//    MPI_SENSOR_CALLBACK_S sensor_callback;
	//    MPI_SENSOR_PATH_S     sensor_path;
	//    INT32 ret = MPI_FAILURE;
	//
	//    sensor_callback.sensor_init = configSensInit;
	//    sensor_callback.sensor_exit = configSensExit;
	//    ret = MPI_regSensorCallback(path_idx, &sensor_callback);
	//    if (ret != MPI_SUCCESS)
	//    {
	//        fprintf(stderr, "Register sensor callback for sensor path %d
	//        failed.\n"
	//               , path_idx);
	//        return MPI_FAILURE;
	//    }
	//    sensor_path.res_type       = MPI_RES_TYPE_FHD;
	//    sensor_path.intf_conf.type = MPI_INTF_TYPE_LVDS;
	//    sensor_path.intf_conf.ptcl = MPI_INTF_PTCL_LVDS_SONY;
	//    sensor_path.bit_width      = MPI_BITS_12;
	//
	//    ret = MPI_setSensorPath(path_idx, &sensor_path);
	//    if(ret != MPI_SUCCESS) {
	//        fprintf(stderr, "Set sensor path %d failed.\n", path_idx);
	//        return MPI_FAILURE;
	//    }
	//
	//    ret = MPI_enableSensorPath(path_idx);
	//    if (ret != MPI_SUCCESS) {
	//        fprintf(stderr, "Enable sensor path %d failed.\n", path_idx);
	//        return MPI_FAILURE;
	//    }
	//
	return MPI_SUCCESS;
}

INT32 __exitSensor(MPI_PATH path_idx)
{
	//    INT32 ret = MPI_FAILURE;
	//
	//    ret = MPI_disableSensorPath(path_idx);
	//    if (ret != MPI_SUCCESS) {
	//        fprintf(stderr, "Disable sensor path %d failed.\n", path_idx);
	//        return MPI_FAILURE;
	//    }
	//
	//    ret = MPI_deregSensorCallback(path_idx);
	//    if(ret != MPI_SUCCESS) {
	//        fprintf(stderr,
	//            "Deregister sensor callback for sensor path %d failed.\n",
	//            path_idx);
	//        return MPI_FAILURE;
	//    }
	//
	return MPI_SUCCESS;
}

INT32 __initChannel(MPI_PATH path_idx, MPI_ECHN chn_idx)
{
	//    MPI_CHN_ATTR_S chn_attr;
	//    INT32 ret = MPI_FAILURE;
	//
	//    chn_attr.res_type_i = MPI_RES_TYPE_FHD;
	//    chn_attr.res_type_o = MPI_RES_TYPE_FHD;
	//    chn_attr.hdr_mode   = MPI_HDR_MODE_NONE;
	//    chn_attr.stitch_en  = 0;
	//    chn_attr.eis_en     = 0;
	//    chn_attr.rotate     = MPI_ROTATE_0;
	//    chn_attr.mirr_en    = 0;
	//    chn_attr.flip_en    = 0;
	//
	//    ret = MPI_setPhyChnAttr(chn_idx, &chn_attr);
	//    if (ret != MPI_SUCCESS) {
	//        fprintf(stderr, "Failed to set physical channel %d.\n", chn_idx);
	//        return MPI_FAILURE;
	//    }
	//
	//    ret = MPI_enablePhyChn(chn_idx);
	//    if (ret != MPI_SUCCESS) {
	//        fprintf(stderr, "Failed to enable physical channel %d.\n", chn_idx);
	//        return MPI_FAILURE;
	//    }
	//
	return MPI_SUCCESS;
}

INT32 __exitChannel(MPI_ECHN chn_idx)
{
	printf("__exitChannel %d\n", __LINE__);
	//    INT32 ret = MPI_FAILURE;
	//
	//    ret = MPI_disablePhyChn(chn_idx);
	//    if (ret != MPI_SUCCESS) {
	//        fprintf(stderr, "Disable physical channel %d failed.\n", chn_idx);
	//        return MPI_FAILURE;
	//    }
	//
	return MPI_SUCCESS;
}

INT32 __readBitStream(void *buf, MPI_BCHN chn_idx, MPI_STREAM_PARAMS_S *stream_params)
{
	INT32 ret = MPI_FAILURE;
	unsigned i;
	size_t offs;
	MPI_BUF_SEG_S *seg;
	ret = MPI_getBitStream(chn_idx, stream_params, 10000 /*ms*/);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to get stream param!");
		return MPI_FAILURE;
	}

	seg = stream_params->seg;
	if (seg == NULL)
		return MPI_FAILURE;

	for (i = 0, offs = 0; i < stream_params->seg_cnt; ++i) {
		seg = &stream_params->seg[i];
		memcpy((void *)((uint32_t)buf + offs), seg->uaddr, seg->size);
		offs += seg->size;
	}

	MPI_releaseBitStream(chn_idx, stream_params);

	return MPI_SUCCESS;
}

/*
struct STREAM_INFO_S *info = {
    MPI_CHN chn_idx;
    int frame_num;
};

void getStreamThread(void *p)
{
}

INT32 __initStreamThread(pthread_t *tid, MPI_CHN chn_idx);
{
}
*/

} /* C declarations*/

SampleStream *SampleStream::createNew(const MPI_PATH path_id, const MPI_ECHN chn_id)
{
	printf("SampleStream::%s\n", __func__);
	printf("%s: path_id = %d, chn_id = %d\n", __func__, path_id.path, chn_id.chn);
	return new SampleStream(path_id, chn_id);
}

SampleStream::SampleStream(const MPI_PATH path_id, const MPI_ECHN chn_id)
        : path_idx(path_id)
        , chn_idx(chn_id)
        , numBytesStored(0)
{
	fBuf = new unsigned char[BUFFER_SIZE];
	fBufPivot = fBuf;
}

SampleStream::~SampleStream()
{
	delete[] fBuf;
	fBufPivot = NULL;
}

MPI_PATH SampleStream::getPathIdx()
{
	return path_idx;
}

MPI_ECHN SampleStream::getChnIdx()
{
	return chn_idx;
}

INT32 SampleStream::setupStream(MPI_PATH path_idx, MPI_ECHN chn_idx)
{
	INT32 ret = MPI_FAILURE;

	//    cout << "SampleStream::" << __func__ <<endl;
	//
	//    ret = __initSystem();
	//    if (ret != MPI_SUCCESS)
	//    {
	//        perror("Failed to initialise MPP System");
	//        goto err_exit;
	//    }
	//
	//    ret = __initSensor(path_idx, chn_idx);
	//    if (ret != MPI_SUCCESS)
	//    {
	//        perror("Failed to initialise sensor path");
	//        goto err_sensor;
	//    }
	//
	//    ret = __initChannel(path_idx, chn_idx);
	//    if (ret != MPI_SUCCESS)
	//    {
	//        perror("Failed to initialise video channel");
	//        goto err_channel;
	//    }
	//
	//    /**
	//     * 1. open() encode device node
	//     * 2. Use ioctl to get bitstream buffer pool info
	//     * 3. mmap() to get the bitstream buffer pool address
	//     **/

	ret = MPI_initBitStreamSystem();
	if (ret != MPI_SUCCESS) {
		perror("Bitstream system initialisation failed");
		// goto err_bitstream;
		return ret;
	}
	//
	cout << "SampleStream::" << __func__ << ": Done." << endl;
	return ret;
	//
	// err_bitstream:
	//    __initChannel(path_idx, chn_idx);
	// err_channel:
	//    __exitSensor(path_idx);
	// err_sensor:
	//    __exitSystem();
	// err_exit:
	//    return MPI_FAILURE;
}

INT32 SampleStream::releaseStream(MPI_PATH path_idx, MPI_ECHN chn_idx)
{
	INT32 ret = MPI_FAILURE;
	printf("%s\n", __func__);
	//
	ret = MPI_exitBitStreamSystem();
	if (ret != MPI_SUCCESS) {
		perror("Failed to exit bitstream system");
	}
	printf("Bit stream system exited.\n");

	ret = MPI_destroyBitStreamChn(this->bchn_idx);
	if (ret != MPI_SUCCESS) {
		perror("Failed to exit bitstream system");
	}

	//
	//    ret = __exitChannel(chn_idx);
	//    if (ret != MPI_SUCCESS)
	//    {
	//        perror("Failed to exit video channel");
	//    }
	//    printf("Video channel exited.\n");
	//
	//    ret = __exitSensor(path_idx);
	//    if (ret != MPI_SUCCESS)
	//    {
	//        perror("Failed to exit sensor path");
	//    }
	//    printf("Sensor exited.\n");
	//
	//    ret = __exitSystem();
	//    if (ret != MPI_SUCCESS)
	//    {
	//        perror("Failed to exit MPP system");
	//    }
	//    printf("MPP channel exited.\n");
	//
	return MPI_SUCCESS;
}

/* bitstream file open for bitstream channel frame/buffer read */
INT32 SampleStream::openStreamChannel(MPI_ECHN chn_idx)
{
	printf("openStreamChannel\n");
	/* Get an indexed stream file descriptor for the bitstream channel */

	this->bchn_idx = MPI_createBitStreamChn(chn_idx);
	if (!VALID_MPI_ENC_BCHN(this->bchn_idx)) {
		fprintf(stderr, "Failed to create bitstream channel %d.\n", chn_idx.chn);
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 SampleStream::closeStreamChannel(MPI_ECHN chn_idx)
{
	//    INT32 ret = MPI_FAILURE;
	//
	//    printf("%s\n", __func__);
	//    ret = MPI_exitBitStreamSystem();
	//    if (ret != MPI_SUCCESS)
	//    {
	//        perror("Bitstream system exit failed");
	//        return MPI_FAILURE;
	//    }
	//
	return MPI_SUCCESS;
}

unsigned SampleStream::readBytes(void *buffer, MPI_ECHN chn_idx, unsigned maxBytesRead)
{
	unsigned i = 0;
	unsigned bytesRead, frameSize;
#ifdef DEBUG
	printf("SampleStream::%s: fMaxSize = %d\n", __func__, maxBytesRead);
	printf("Buffer status: fBuf = %p, fBufPivot = %p, numBytesStored = %u\n", fBuf, fBufPivot, numBytesStored);
#endif

	if (numBytesStored != 0) {
		/* if the remaining data byte is less than maximum byte required, return all
     * the
     * data */
		if (maxBytesRead >= numBytesStored) {
#ifdef DEBUG
			printf("Case 1: Read all remaining data in buffer.\n");
#endif
			memmove(buffer, fBufPivot, numBytesStored);
			bytesRead = numBytesStored;
			numBytesStored = 0;
			fBufPivot = fBuf;
		} else {
#ifdef DEBUG
			printf("Case 2: Read %u byte of data from buffer.\n", maxButesRead);
#endif
			memmove(buffer, fBufPivot, maxBytesRead);
			bytesRead = maxBytesRead;
			numBytesStored -= maxBytesRead;
			fBufPivot += bytesRead;
		}
	} else {
		/* No data in the buffer; read a new frame */
		if (__readBitStream(fBuf, this->bchn_idx, &stream_params) != MPI_SUCCESS) {
			perror("Failed to read the frame!\n");
			return 0;
		}
		for (i = 0, frameSize = 0; i < stream_params.seg_cnt; ++i)
			frameSize += stream_params.seg[i].size;
#ifdef DEBUG
		printf("Read %u bytes from encoder...\n", frameSize);
#endif

		bytesRead = frameSize;
		/* Sanity check to make sure the buffer size is large enough */
		if (bytesRead > BUFFER_SIZE) {
			perror("Buffer overflow!\n");
			return 0;
		}

		if (bytesRead > maxBytesRead) {
#ifdef DEBUG
			printf("Case 3: Read from encoder to buffer, and take %u bytes from "
			       "buffer\n",
			       maxBytesRead);
#endif
			memmove(buffer, fBufPivot, maxBytesRead);
			numBytesStored = bytesRead - maxBytesRead;
			bytesRead = maxBytesRead;
			fBufPivot = fBuf + bytesRead;
#ifdef DEBUG
			printf("bytesRead = %u, numBytesStored = %u\n", bytesRead, numBytesStored);
#endif
		} else {
#ifdef DEBUG
			printf("Case 4: Read from encoder to buffer, and take all data from "
			       "buffer\n");
#endif
			memmove(buffer, fBuf, frameSize);
			bytesRead = frameSize;
		}
	}
#ifdef DEBUG
	printf("SampleStream::%s(): %u bytes read.\n", __func__, bytesRead);
#endif
	return bytesRead;
}

size_t SampleStream::readNextFrame(void *buffer, MPI_ECHN chn_idx)
{
	unsigned i;
	unsigned frameSize;
	if (__readBitStream(buffer, this->bchn_idx, &stream_params) != MPI_SUCCESS) {
		perror("Failed to read the frame!\n");
		return 0;
	}

	for (i = 0, frameSize = 0; i < stream_params.seg_cnt; ++i)
		frameSize += stream_params.seg[i].size;
	return frameSize;
}

size_t SampleStream::readNextBuffer(void *buffer, MPI_ECHN chn_idx)
{
	unsigned i;
	unsigned frameSize;
	if (__readBitStream(buffer, this->bchn_idx, &stream_params) != MPI_SUCCESS) {
		perror("Failed to read the frame!\n");
		return 0;
	}

	for (i = 0, frameSize = 0; i < stream_params.seg_cnt; ++i)
		frameSize += stream_params.seg[i].size;
	return frameSize;
}
