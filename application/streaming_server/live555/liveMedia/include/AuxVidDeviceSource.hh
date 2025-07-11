/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2014 Live Networks, Inc.  All rights reserved.
// A template for a MediaSource encapsulating an audio/video input device
//
// NOTE: Sections of this code labeled "%%% TO BE WRITTEN %%%" are incomplete, and needto be written by the programmer
// (depending on the features of the particulardevice).
// C++ header

#ifndef _AUXVID_DEVICE_SOURCE_HH
#define _AUXVID_DEVICE_SOURCE_HH

#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

#ifndef _DEVICE_SOURCE_HH
#include "DeviceSource.hh"
#endif
class DeviceSource;

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_enc.h"

typedef struct {
#ifdef HC1703_1723_1753_1783S
	MPI_STREAM_PARAMS_V2_S params;
#else
	MPI_STREAM_PARAMS_S params;
#endif
	unsigned char *p_sei_nalu;
	int sei_nalu_len;
} VIDEO_STREAM_DATA;

class AuxVidDeviceSource : public DeviceSource {
    public:
	static DeviceSource *createNew(UsageEnvironment &env, UINT32 chn_idx, int timerfdInterval/*get max FPS from ccserver*/);

    protected:
	AuxVidDeviceSource(UsageEnvironment &env, UINT32 chn_idx, int timerfdInterval/*get max FPS from ccserver*/);
	virtual ~AuxVidDeviceSource();
	static void getNextFrame(void *ptr);
	void getBitStream();
	int setH264SEIdata();
	int setH265SEIdata();

    private:
	virtual void doGetNextFrame();
	virtual unsigned maxFrameSize() const;

#define MAX_SEI_SIZE 4096
#define SEI_UUID_SIZE 16
#define NALU_TYPE_SUFFIX_SEI 40
#define MAX_RETRY_TIMES 100

	/*For DUMP*/
	unsigned int fWriteTrigger;
	unsigned int fSplitFileSize;

	/*For IVA*/
	MPI_WIN fIvaIdx;
	MPI_WIN fIdx;
	MPI_RECT_S fSrcRect, fDstRect;
	MPI_RECT_S fSrcRoi, fDstRoi;
	UINT32 win_idx;
	char fUuid[SEI_UUID_SIZE];
	char fTmpSEIStr[MAX_SEI_SIZE];
	char fSeiNalu[MAX_SEI_SIZE];

	MPI_ECHN chn_idx;
	MPI_BCHN bchn_idx;
#ifdef HC1703_1723_1753_1783S
	MPI_STREAM_PARAMS_V2_S stream_params;
#else
	MPI_STREAM_PARAMS_S stream_params;
#endif
	MPI_VENC_TYPE_E enc_type;
	int isIframe;
	int isCreateChn;
	int init_fail;
	int fFirstRelease;
	unsigned long fFrameID;
	int fTimerfdInterval;
};

#endif
