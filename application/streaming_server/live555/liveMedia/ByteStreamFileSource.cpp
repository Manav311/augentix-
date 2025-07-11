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
// A file source that is a plain byte stream (rather than frames)
// Implementation

#include "ByteStreamFileSource.hh"
#include "GroupsockHelper.hh"
#include "InputFile.hh"
#include "mpi_index.h"

#include <iostream>
using namespace std;
#define HC_STREAM 1

#ifdef HC_STREAM
#include "SampleStream.hh"
#endif

////////// ByteStreamFileSource //////////

ByteStreamFileSource *ByteStreamFileSource::createNew(UsageEnvironment &env, char const *fileName,
                                                      unsigned preferredFrameSize, unsigned playTimePerFrame)
{
	FILE *fid = OpenInputFile(env, fileName);
	if (fid == NULL)
		return NULL;
	ByteStreamFileSource *newSource = new ByteStreamFileSource(env, fid, preferredFrameSize, playTimePerFrame);
	newSource->fFileSize = GetFileSize(fileName, fid);
	return newSource;
}

/* Not used by testOnDemandSampleStream */
ByteStreamFileSource *ByteStreamFileSource::createNew(UsageEnvironment &env, FILE *fid, unsigned preferredFrameSize,
                                                      unsigned playTimePerFrame)
{
	if (fid == NULL)
		return NULL;

	ByteStreamFileSource *newSource = new ByteStreamFileSource(env, fid, preferredFrameSize, playTimePerFrame);
	newSource->fFileSize = GetFileSize(NULL, fid);
	printf("ByteStreamFileSource::createNew: preferredFrameSize = %d, "
	       "playTimePerFrame = %d, FileSize = %llu\n",
	       preferredFrameSize, playTimePerFrame, newSource->fFileSize);

	return newSource;
}

void ByteStreamFileSource::seekToByteAbsolute(u_int64_t byteNumber, u_int64_t numBytesToStream)
{
	printf("%s\n", __func__);
	SeekFile64(fFid, (int64_t)byteNumber, SEEK_SET);

	fNumBytesToStream = numBytesToStream;
	fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void ByteStreamFileSource::seekToByteRelative(int64_t offset, u_int64_t numBytesToStream)
{
	printf("%s\n", __func__);
	SeekFile64(fFid, offset, SEEK_CUR);

	fNumBytesToStream = numBytesToStream;
	fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void ByteStreamFileSource::seekToEnd()
{
	SeekFile64(fFid, 0, SEEK_END);
}

ByteStreamFileSource::ByteStreamFileSource(UsageEnvironment &env, FILE *fid, unsigned preferredFrameSize,
                                           unsigned playTimePerFrame)
        : FramedFileSource(env, fid)
        , fFileSize(0)
        , fPreferredFrameSize(preferredFrameSize)
        , fPlayTimePerFrame(playTimePerFrame)
        , fLastPlayTime(0)
        , fHaveStartedReading(False)
        , fLimitNumBytesToStream(False)
        , fNumBytesToStream(0)
{
#ifndef READ_FROM_FILES_SYNCHRONOUSLY
	makeSocketNonBlocking(fileno(fFid));
#endif
	videoStream = SampleStream::createNew(MPI_INPUT_PATH(0, 0), MPI_ENC_CHN(0));
	videoStream->setupStream(videoStream->getPathIdx(), videoStream->getChnIdx());
	videoStream->openStreamChannel(videoStream->getChnIdx());
	printf("ByteStreamFileSource:\n\tfPreferredFrameSize = "
	       "%d\n\tfPlayTimePerFrame = %d\n\t",
	       fPreferredFrameSize, fPlayTimePerFrame);

	// Test whether the file is seekable
	fFidIsSeekable = FileIsSeekable(fFid);
}

ByteStreamFileSource::~ByteStreamFileSource()
{
	if (fFid == NULL)
		return;

#ifndef READ_FROM_FILES_SYNCHRONOUSLY
	envir().taskScheduler().turnOffBackgroundReadHandling(fileno(fFid));
#endif
	videoStream->closeStreamChannel(videoStream->getChnIdx());
	videoStream->releaseStream(videoStream->getPathIdx(), videoStream->getChnIdx());
	/*
  CloseInputFile(fFid);
  */
}

void ByteStreamFileSource::doGetNextFrame()
{
	if (feof(fFid) || ferror(fFid) || (fLimitNumBytesToStream && fNumBytesToStream == 0)) {
		handleClosure();
		return;
	}

#ifdef READ_FROM_FILES_SYNCHRONOUSLY
	doReadFromFile();
#else
	if (!fHaveStartedReading) {
		// Await readable data from the file:
		envir().taskScheduler().turnOnBackgroundReadHandling(
		        fileno(fFid), (TaskScheduler::BackgroundHandlerProc *)&fileReadableHandler, this);
		fHaveStartedReading = True;
	}
#endif
}

void ByteStreamFileSource::doStopGettingFrames()
{
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
#ifndef READ_FROM_FILES_SYNCHRONOUSLY
	envir().taskScheduler().turnOffBackgroundReadHandling(fileno(fFid));
	fHaveStartedReading = False;
#endif
}

void ByteStreamFileSource::fileReadableHandler(ByteStreamFileSource *source, int /*mask*/)
{
	if (!source->isCurrentlyAwaitingData()) {
		source->doStopGettingFrames(); // we're not ready for the data yet
		return;
	}
	source->doReadFromFile();
}

void ByteStreamFileSource::doReadFromFile()
{
	// Try to read as many bytes as will fit in the buffer provided (or
	// "fPreferredFrameSize" if less)
	if (fLimitNumBytesToStream && fNumBytesToStream < (u_int64_t)fMaxSize) {
		fMaxSize = (unsigned)fNumBytesToStream;
	}
	if (fPreferredFrameSize > 0 && fPreferredFrameSize < fMaxSize) {
		fMaxSize = fPreferredFrameSize;
	}
/* Since the device node is neither seekable or readable, we directly call our
 * MPI */
#ifdef HC_STREAM
	/*
  unsigned newFrameSize = videoStream->readNextBuffer(fTo,
  videoStream->getChnIdx());
  printf("Frame: reads %d bytes; fMaxSize = %d\n", newFrameSize, fMaxSize);
  if (newFrameSize > fMaxSize) {
    fFrameSize = fMaxSize;
    fNumTruncatedBytes = newFrameSize - fMaxSize;
    printf("Truncated %d bytes\n", fNumTruncatedBytes);
  } else {
    fFrameSize = newFrameSize;
  }
  */

	fFrameSize = videoStream->readBytes(fTo, videoStream->getChnIdx(), fMaxSize);

#else

#ifdef READ_FROM_FILES_SYNCHRONOUSLY
	fFrameSize = fread(fTo, 1, fMaxSize, fFid);
#else
	if (fFidIsSeekable) {
		fFrameSize = fread(fTo, 1, fMaxSize, fFid);
	} else {
		// For non-seekable files (e.g., pipes), call "read()" rather than
		// "fread()", to ensure that the read doesn't block:
		fFrameSize = read(fileno(fFid), fTo, fMaxSize);
	}
#endif /* READ_FROM_FILES_SYNCHRONOUSLY */

#endif /* HC_STREAM */

	if (fFrameSize == 0) {
		handleClosure();
		return;
	}

	/* TODO: What should we do with this line? */
	fNumBytesToStream -= fFrameSize;

	// Set the 'presentation time':
	if (fPlayTimePerFrame > 0 && fPreferredFrameSize > 0) {
		if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
			// This is the first frame, so use the current time:
			gettimeofday(&fPresentationTime, NULL);
		} else {
			// Increment by the play time of the previous data:
			unsigned uSeconds = fPresentationTime.tv_usec + fLastPlayTime;
			fPresentationTime.tv_sec += uSeconds / 1000000;
			fPresentationTime.tv_usec = uSeconds % 1000000;
		}

		// Remember the play time of this data:
		fLastPlayTime = (fPlayTimePerFrame * fFrameSize) / fPreferredFrameSize;
		fDurationInMicroseconds = fLastPlayTime;
	} else {
		// We don't know a specific play time duration for this data,
		// so just record the current time as being the 'presentation time':
		gettimeofday(&fPresentationTime, NULL);
	}

// Inform the reader that he has data:
#ifdef READ_FROM_FILES_SYNCHRONOUSLY
	// To avoid possible infinite recursion, we need to return to the event loop
	// to do this:
	nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc *)FramedSource::afterGetting, this);
#else
	// Because the file read was done from the event loop, we can call the
	// 'after getting' function directly, without risk of infinite recursion:
	FramedSource::afterGetting(this);
#endif
}
