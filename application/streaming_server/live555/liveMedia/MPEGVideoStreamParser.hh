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
// An abstract parser for MPEG video streams
// C++ header

#ifndef _MPEG_VIDEO_STREAM_PARSER_HH
#define _MPEG_VIDEO_STREAM_PARSER_HH

#ifndef _STREAM_PARSER_HH
#include "StreamParser.hh"
#endif
#ifndef _MPEG_VIDEO_STREAM_FRAMER_HH
#include "MPEGVideoStreamFramer.hh"
#endif


////////// MPEGVideoStreamParser definition //////////
class MPEGVideoStreamParser : public StreamParserByPtr {
    public:
	MPEGVideoStreamParser(MPEGVideoStreamFramer *usingSource, FramedSource *inputSource);
	virtual ~MPEGVideoStreamParser();

    public:
	void registerReadInterest(unsigned char *to, unsigned maxSize);

	virtual unsigned parse() = 0;
	// returns the size of the frame that was acquired, or 0 if none was
	// The number of truncated bytes (if any) is given by:
	unsigned numTruncatedBytes() const
	{
		return fNumTruncatedBytes;
	}

    protected:
	unsigned char *testNBytes(int size)
	{ // as above, but doesn't advance ptr
		ensureValidBytes(size);
		unsigned char *ptr = nextToParse();
		return ptr;
	}

	void setParseState()
	{
		fSavedTo = fTo;
		fSavedNumTruncatedBytes = fNumTruncatedBytes;
		saveParserState();
	}

	// Record "byte" in the current output frame:
	void saveByte(u_int8_t byte)
	{
		if (fTo >= fLimit) { // there's no space left
			++fNumTruncatedBytes;
			return;
		}

		*fTo++ = byte;
	}

	void save4Bytes(u_int32_t word)
	{
		if (fTo + 4 > fLimit) { // there's no space left
			fNumTruncatedBytes += 4;
			return;
		}
		//	unsigned char* tmp = (unsigned char*)&word;
		//	printf("word %x tmp %x %x %x %x\n",word,tmp[0],tmp[1],tmp[2],tmp[3]);
		//	printf("word %x %x %x %x\n",word>>24, word>>16,word>>8,word);
		*fTo++ = word >> 24;
		*fTo++ = word >> 16;
		*fTo++ = word >> 8;
		*fTo++ = word;
	}

	void save4Bytes1(char *ptr)
	{
		if (fTo + 4 > fLimit) { // there's no space left
			fNumTruncatedBytes += 4;
			return;
		}

		memcpy(fTo, ptr, 4);
		fTo += 4;
	}

	void saveNBytes(unsigned char *ptr, int size)
	{
		if (fTo + size > fLimit) { // there's no space left
			fNumTruncatedBytes += size;
			printf("drop data\n");
			return;
		}
		// printf("memcpy size %d\n",size);
		//    int len = (size/32) * 32;
		//    printf("len %d size %d\n",len,size);
		//
		//    if(len){
		//    	asm volatile(
		//    	"copy_loop8%=:\n"
		//    	"ldmia %[src]!, {r0-r7}\n"
		//    	"stmia %[dest]!, {r0-r7}\n"
		//    	"sub %[len], %[len], #32\n"
		//    	"cmp %[len], #0\n"
		//    	"bne copy_loop8%=\n"
		//    	: [src] "+r"(ptr), [dest] "+r"(fTo), [len] "+r"(len)
		//    	  :
		//    	  : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
		//    	);
		//    }
		//    if((size - len) > 0)

		memcpy(fTo, ptr, size);
		fTo += size;
	}
	// Save data until we see a sync word (0x000001xx):
	void saveToNextCode(u_int32_t &curWord)
	{
		saveByte(curWord >> 24);
		curWord = (curWord << 8) | get1Byte();
		while ((curWord & 0xFFFFFF00) != 0x00000100) {
			if ((unsigned)(curWord & 0xFF) > 1) {
				// a sync word definitely doesn't begin anywhere in "curWord"
				save4Bytes(curWord);
				curWord = get4Bytes();
			} else {
				// a sync word might begin in "curWord", although not at its start
				saveByte(curWord >> 24);
				unsigned char newByte = get1Byte();
				curWord = (curWord << 8) | newByte;
			}
		}
	}

	// Skip data until we see a sync word (0x000001xx):
	void skipToNextCode(u_int32_t &curWord)
	{
		curWord = (curWord << 8) | get1Byte();
		while ((curWord & 0xFFFFFF00) != 0x00000100) {
			if ((unsigned)(curWord & 0xFF) > 1) {
				// a sync word definitely doesn't begin anywhere in "curWord"
				curWord = get4Bytes();
			} else {
				// a sync word might begin in "curWord", although not at its start
				unsigned char newByte = get1Byte();
				curWord = (curWord << 8) | newByte;
			}
		}
	}

    protected:
	MPEGVideoStreamFramer *fUsingSource;

	// state of the frame that's currently being read:
	unsigned char *fStartOfFrame;
	unsigned char *fTo;
	unsigned char *fLimit;
	unsigned fNumTruncatedBytes;
	unsigned curFrameSize()
	{
		return fTo - fStartOfFrame;
	}
	unsigned char *fSavedTo;
	unsigned fSavedNumTruncatedBytes;

    private: // redefined virtual functions
	virtual void restoreSavedParserState();
};

#endif
