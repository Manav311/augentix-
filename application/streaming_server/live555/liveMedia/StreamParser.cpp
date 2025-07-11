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
// Abstract class for parsing a byte stream
// Implementation

#include "StreamParser.hh"

#include <stdlib.h>
#include <string.h>

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_enc.h"

#define BANK_SIZE 1000000

typedef struct {
#ifdef HC1703_1723_1753_1783S
	MPI_STREAM_PARAMS_V2_S params;
#else
	MPI_STREAM_PARAMS_S params;
#endif
	unsigned char *p_sei_nalu;
	int sei_nalu_len;
} VIDEO_STREAM_DATA;


#define BAKK_SIZE_PTR sizeof(VIDEO_STREAM_DATA)

void StreamParser::flushInput()
{
	fCurParserIndex = fSavedParserIndex = 0;
	fSavedRemainingUnparsedBits = fRemainingUnparsedBits = 0;
	fTotNumValidBytes = 0;
}

StreamParser::StreamParser(FramedSource *inputSource, FramedSource::onCloseFunc *onInputCloseFunc,
                           void *onInputCloseClientData, clientContinueFunc *clientContinueFunc,
                           void *clientContinueClientData)
        : fInputSource(inputSource)
        , fClientOnInputCloseFunc(onInputCloseFunc)
        , fClientOnInputCloseClientData(onInputCloseClientData)
        , fClientContinueFunc(clientContinueFunc)
        , fClientContinueClientData(clientContinueClientData)
        , fSavedParserIndex(0)
        , fSavedRemainingUnparsedBits(0)
        , fCurParserIndex(0)
        , fRemainingUnparsedBits(0)
        , fTotNumValidBytes(0)
        , fHaveSeenEOF(False)
{
	fBank[0] = new unsigned char[BANK_SIZE];
	fCurBankNum = 0;
	fCurBank = fBank[fCurBankNum];

	fLastSeenPresentationTime.tv_sec = 0;
	fLastSeenPresentationTime.tv_usec = 0;
}

StreamParser::~StreamParser()
{
	delete[] fBank[0];
}

void StreamParser::saveParserState()
{
	fSavedParserIndex = fCurParserIndex;
	fSavedRemainingUnparsedBits = fRemainingUnparsedBits;
}

void StreamParser::restoreSavedParserState()
{
	fCurParserIndex = fSavedParserIndex;
	fRemainingUnparsedBits = fSavedRemainingUnparsedBits;
}

void StreamParser::skipBits(unsigned numBits)
{
	if (numBits <= fRemainingUnparsedBits) {
		fRemainingUnparsedBits -= numBits;
	} else {
		numBits -= fRemainingUnparsedBits;

		unsigned numBytesToExamine = (numBits + 7) / 8; // round up
		ensureValidBytes(numBytesToExamine);
		fCurParserIndex += numBytesToExamine;

		fRemainingUnparsedBits = 8 * numBytesToExamine - numBits;
	}
}

unsigned StreamParser::getBits(unsigned numBits)
{
	if (numBits <= fRemainingUnparsedBits) {
		unsigned char lastByte = *lastParsed();
		lastByte >>= (fRemainingUnparsedBits - numBits);
		fRemainingUnparsedBits -= numBits;

		return (unsigned)lastByte & ~((~0u) << numBits);
	} else {
		unsigned char lastByte;
		if (fRemainingUnparsedBits > 0) {
			lastByte = *lastParsed();
		} else {
			lastByte = 0;
		}

		unsigned remainingBits = numBits - fRemainingUnparsedBits; // > 0

		// For simplicity, read the next 4 bytes, even though we might not
		// need all of them here:
		unsigned result = test4Bytes();

		result >>= (32 - remainingBits);
		result |= (lastByte << remainingBits);
		if (numBits < 32)
			result &= ~((~0u) << numBits);

		unsigned const numRemainingBytes = (remainingBits + 7) / 8;
		fCurParserIndex += numRemainingBytes;
		fRemainingUnparsedBits = 8 * numRemainingBytes - remainingBits;

		return result;
	}
}

unsigned StreamParser::bankSize() const
{
	return BANK_SIZE;
}

#define NO_MORE_BUFFERED_INPUT 1

// We need to read some more bytes from the input source.
// First, clarify how much data to ask for:
void StreamParser::ensureValidBytes1(unsigned numBytesNeeded)
{
	unsigned maxInputFrameSize = fInputSource->maxFrameSize();
	if (maxInputFrameSize > numBytesNeeded)
		numBytesNeeded = maxInputFrameSize;

	// First, check whether these new bytes would overflow the current
	// bank.  If so, start using a new bank now.
	if (fCurParserIndex + numBytesNeeded > BANK_SIZE) {
		// Swap banks, but save any still-needed bytes from the old bank:
		unsigned numBytesToSave = fTotNumValidBytes - fSavedParserIndex;
		unsigned char const *from = &curBank()[fSavedParserIndex];

		fCurBankNum = (fCurBankNum + 1) % 2;
		fCurBank = fBank[fCurBankNum];
		memmove(curBank(), from, numBytesToSave);
		fCurParserIndex = fCurParserIndex - fSavedParserIndex;
		fSavedParserIndex = 0;
		fTotNumValidBytes = numBytesToSave;
	}

	// ASSERT: fCurParserIndex + numBytesNeeded > fTotNumValidBytes
	//      && fCurParserIndex + numBytesNeeded <= BANK_SIZE
	if (fCurParserIndex + numBytesNeeded > BANK_SIZE) {
		// If this happens, it means that we have too much saved parser state.
		// To fix this, increase BANK_SIZE as appropriate.
		fInputSource->envir() << "StreamParser internal error (" << fCurParserIndex << " + " << numBytesNeeded
		                      << " > " << BANK_SIZE << ")\n";
		fInputSource->envir().internalError();
	}

	// Try to read as many new bytes as will fit in the current bank:
	unsigned maxNumBytesToRead = BANK_SIZE - fTotNumValidBytes;
	fInputSource->getNextFrame(&curBank()[fTotNumValidBytes], maxNumBytesToRead, afterGettingBytes, this,
	                           onInputClosure, this);

	throw NO_MORE_BUFFERED_INPUT;
}

void StreamParser::afterGettingBytes(void *clientData, unsigned numBytesRead, unsigned /*numTruncatedBytes*/,
                                     struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
	StreamParser *parser = (StreamParser *)clientData;
	if (parser != NULL)
		parser->afterGettingBytes1(numBytesRead, presentationTime);
}

void StreamParser::afterGettingBytes1(unsigned numBytesRead, struct timeval presentationTime)
{
	// Sanity check: Make sure we didn't get too many bytes for our bank:
	if (fTotNumValidBytes + numBytesRead > BANK_SIZE) {
		fInputSource->envir() << "StreamParser::afterGettingBytes() warning: read " << numBytesRead
		                      << " bytes; expected no more than " << BANK_SIZE - fTotNumValidBytes << "\n";
	}

	fLastSeenPresentationTime = presentationTime;

	unsigned char *ptr = &curBank()[fTotNumValidBytes];
	fTotNumValidBytes += numBytesRead;

	// Continue our original calling source where it left off:
	restoreSavedParserState();
	// Sigh... this is a crock; things would have been a lot simpler
	// here if we were using threads, with synchronous I/O...
	fClientContinueFunc(fClientContinueClientData, ptr, numBytesRead, presentationTime);
}

void StreamParser::onInputClosure(void *clientData)
{
	StreamParser *parser = (StreamParser *)clientData;
	if (parser != NULL)
		parser->onInputClosure1();
}

void StreamParser::onInputClosure1()
{
	if (!fHaveSeenEOF) {
		// We're hitting EOF for the first time.  Set our 'EOF' flag, and continue
		// parsing, as if we'd just read 0 bytes of data.
		// This allows the parser to re-parse any remaining unparsed data (perhaps
		// while testing for EOF at the end):
		fHaveSeenEOF = True;
		afterGettingBytes1(0, fLastSeenPresentationTime);
	} else {
		// We're hitting EOF for the second time.  Now, we handle the source input
		// closure:
		fHaveSeenEOF = False;
		if (fClientOnInputCloseFunc != NULL)
			(*fClientOnInputCloseFunc)(fClientOnInputCloseClientData);
	}
}

void StreamParserByPtr::flushInput()
{
	fCurParserIndex = fSavedParserIndex = 0;
	fSavedRemainingUnparsedBits = fRemainingUnparsedBits = 0;
	fTotNumValidBytes = 0;
}

StreamParserByPtr::StreamParserByPtr(FramedSource *inputSource, FramedSource::onCloseFunc *onInputCloseFunc,
                                     void *onInputCloseClientData, clientContinueFunc *clientContinueFunc,
                                     void *clientContinueClientData)
        : fInputSource(inputSource)
        , fClientOnInputCloseFunc(onInputCloseFunc)
        , fClientOnInputCloseClientData(onInputCloseClientData)
        , fClientContinueFunc(clientContinueFunc)
        , fClientContinueClientData(clientContinueClientData)
        , fSavedParserIndex(0)
        , fSavedRemainingUnparsedBits(0)
        , fCurParserIndex(0)
        , fRemainingUnparsedBits(0)
        , fTotNumValidBytes(0)
        , fHaveSeenEOF(False)
{
	fprintf(stderr, "create StreamParserByPtr class\n");
	fBank[0] = new unsigned char[BAKK_SIZE_PTR];
	fBank[1] = new unsigned char[BAKK_SIZE_PTR];
	fCurBankNum = 0;
	fCurBank = fBank[fCurBankNum];

	fLastSeenPresentationTime.tv_sec = 0;
	fLastSeenPresentationTime.tv_usec = 0;
}

StreamParserByPtr::~StreamParserByPtr()
{
	delete[] fBank[0];
	delete[] fBank[1];
}

void StreamParserByPtr::saveParserState()
{
	fSavedParserIndex = fCurParserIndex;
	fSavedRemainingUnparsedBits = fRemainingUnparsedBits;
}

void StreamParserByPtr::restoreSavedParserState()
{
	fCurParserIndex = fSavedParserIndex;
	fRemainingUnparsedBits = fSavedRemainingUnparsedBits;
}

void StreamParserByPtr::skipBits(unsigned numBits)
{
	if (numBits <= fRemainingUnparsedBits) {
		fRemainingUnparsedBits -= numBits;
	} else {
		numBits -= fRemainingUnparsedBits;

		unsigned numBytesToExamine = (numBits + 7) / 8; // round up
		ensureValidBytes(numBytesToExamine);
		fCurParserIndex += numBytesToExamine;

		fRemainingUnparsedBits = 8 * numBytesToExamine - numBits;
	}
}

unsigned StreamParserByPtr::getBits(unsigned numBits)
{
	if (numBits <= fRemainingUnparsedBits) {
		unsigned char lastByte = *lastParsed();
		lastByte >>= (fRemainingUnparsedBits - numBits);
		fRemainingUnparsedBits -= numBits;

		return (unsigned)lastByte & ~((~0u) << numBits);
	} else {
		unsigned char lastByte;
		if (fRemainingUnparsedBits > 0) {
			lastByte = *lastParsed();
		} else {
			lastByte = 0;
		}

		unsigned remainingBits = numBits - fRemainingUnparsedBits; // > 0

		// For simplicity, read the next 4 bytes, even though we might not
		// need all of them here:
		unsigned result = test4Bytes();

		result >>= (32 - remainingBits);
		result |= (lastByte << remainingBits);
		if (numBits < 32)
			result &= ~((~0u) << numBits);

		unsigned const numRemainingBytes = (remainingBits + 7) / 8;
		fCurParserIndex += numRemainingBytes;
		fRemainingUnparsedBits = 8 * numRemainingBytes - remainingBits;

		return result;
	}
}

unsigned StreamParserByPtr::bankSize() const
{
	return BAKK_SIZE_PTR;
}

#define NO_MORE_BUFFERED_INPUT 1

void StreamParserByPtr::ensureValidBytes1(unsigned numBytesNeeded)
{
	// bank.  If so, start using a new bank now.
	//if (fCurParserIndex + numBytesNeeded > BAKK_SIZE_PTR) {
	fCurParserIndex = 0;
	fSavedParserIndex = 0;
	fTotNumValidBytes = 0;
	//}
	// Try to read as many new bytes as will fit in the current bank:
	//unsigned maxNumBytesToRead = BAKK_SIZE_PTR - fTotNumValidBytes;
	fInputSource->getNextFrame(&curBank()[fTotNumValidBytes], 0, afterGettingBytes, this, onInputClosure, this);
	throw NO_MORE_BUFFERED_INPUT;
}

void StreamParserByPtr::afterGettingBytes(void *clientData, unsigned numBytesRead, unsigned /*numTruncatedBytes*/,
                                          struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
	StreamParserByPtr *parser = (StreamParserByPtr *)clientData;
	if (parser != NULL)
		parser->afterGettingBytes1(numBytesRead, presentationTime);
}

void StreamParserByPtr::afterGettingBytes1(unsigned numBytesRead, struct timeval presentationTime)
{
	// Sanity check: Make sure we didn't get too many bytes for our bank:
	fLastSeenPresentationTime = presentationTime;

	//unsigned char *ptr = &curBank()[fTotNumValidBytes];
	fTotNumValidBytes += numBytesRead;

	// Continue our original calling source where it left off:
	restoreSavedParserState();
	// Sigh... this is a crock; things would have been a lot simpler
	// here if we were using threads, with synchronous I/O...
	fClientContinueFunc(fClientContinueClientData, NULL, numBytesRead, presentationTime);
}

void StreamParserByPtr::onInputClosure(void *clientData)
{
	StreamParserByPtr *parser = (StreamParserByPtr *)clientData;
	if (parser != NULL)
		parser->onInputClosure1();
}

void StreamParserByPtr::onInputClosure1()
{
	if (!fHaveSeenEOF) {
		// We're hitting EOF for the first time.  Set our 'EOF' flag, and continue
		// parsing, as if we'd just read 0 bytes of data.
		// This allows the parser to re-parse any remaining unparsed data (perhaps
		// while testing for EOF at the end):
		fHaveSeenEOF = True;
		afterGettingBytes1(0, fLastSeenPresentationTime);
	} else {
		// We're hitting EOF for the second time.  Now, we handle the source input
		// closure:
		fHaveSeenEOF = False;
		if (fClientOnInputCloseFunc != NULL)
			(*fClientOnInputCloseFunc)(fClientOnInputCloseClientData);
	}
}
