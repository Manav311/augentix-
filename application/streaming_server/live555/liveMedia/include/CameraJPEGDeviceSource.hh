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
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/

// JPEG camera input device
// C++ header

#ifndef JPEG_DEVICE_SOURCE_HH
#define JPEG_DEVICE_SOURCE_HH

#include "JPEGVideoSource.hh"

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_enc.h"
#include "JPEGFrameParser.hh"

#include "AuxVidDeviceSource.hh"

#define JPEG_HEADER_SIZE 0x299 + 12
#define MAX_SIZE 1920 * 1080

class CameraJPEGDeviceSource : public JPEGVideoSource {
    public:
	static CameraJPEGDeviceSource *createNew(UsageEnvironment &env, unsigned clientSessionId);
	static CameraJPEGDeviceSource *createNew(UsageEnvironment &env, unsigned clientSessionId, unsigned int chn_idx, int timerfdInterval/*get max FPS from ccserver*/);

    protected:
	CameraJPEGDeviceSource(UsageEnvironment &env, unsigned int chn_idx, int timerfdInterval);
	virtual ~CameraJPEGDeviceSource();
	static void getNextFrame(void *ptr);
	void getBitStream();

    private:
	// redefined virtual functions:
	virtual void doGetNextFrame();
	virtual u_int8_t type();
	virtual u_int8_t qFactor();
	virtual u_int8_t width();
	virtual u_int8_t height();
	virtual u_int8_t const *quantizationTables(u_int8_t &precision, u_int16_t &length);

    private:
	UsageEnvironment &fEnv;
	void startCapture();
	void setParamsFromHeader();
	virtual unsigned maxFrameSize() const;

    private:
	JPEGFrameParser *fJpegFrameParser;
	struct timeval fLastCaptureTime;
	u_int8_t fType, fLastQFactor, fLastWidth, fLastHeight;
	unsigned char fJPEGHeader[JPEG_HEADER_SIZE];
	char CamThreadFrameBuffer[MAX_SIZE];
	int CamThreadFrameBufferSize;
	MPI_ECHN chn_idx;
	MPI_BCHN bchn_idx;
	UINT32 win_idx;
	int iva_win_idx;
	MPI_STREAM_PARAMS_S stream_params;
	MPI_VENC_TYPE_E enc_type;
	int isIframe;
	int isCreateChn;
	int init_fail;
	VIDEO_STREAM_DATA MJPEG_buf;
	unsigned long fFrameID;
	int fTimerfdInterval;
};

#endif
