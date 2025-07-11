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

#ifndef _AUXAUDIO_DEVICE_SOURCE_HH
#define _AUXAUDIO_DEVICE_SOURCE_HH

#ifndef _DEVICE_SOURCE_HH
#include "DeviceSource.hh"
#endif
#include "WAVAudioFileSource.hh"

class DeviceSource;

class AuxAudioDeviceSource : public DeviceSource {
    public:
	static DeviceSource *createNew(UsageEnvironment &env, unsigned int chn_idx, unsigned int codecType,
	                               unsigned int bitPerSample, unsigned int freq, unsigned int gain);

    protected:
	AuxAudioDeviceSource(UsageEnvironment &env, unsigned int chn_idx, unsigned int codecType,
	                     unsigned int bitPerSample, unsigned int freq, unsigned int gain);
	virtual ~AuxAudioDeviceSource();
	static void getNextFrame(void *ptr);
	void getBitStream();

    private:
	virtual void doGetNextFrame();
	virtual unsigned maxFrameSize() const;
	unsigned int fChn_idx;
	int isAudioframe;
	int init_fail;
	unsigned int fCodecType;
	unsigned int fSamplingBits;
	unsigned int fGain;
	unsigned int fFrequency;
	unsigned int fNumChannels;
	/*ALSA handle ptr*/
	void *fCaptureHandle;
	/*define samples per frame*/
	int fFrame;
	int fTimerfdInterval;
};

#endif
