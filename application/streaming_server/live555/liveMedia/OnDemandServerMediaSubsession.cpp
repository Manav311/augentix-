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
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand.
// Implementation

#include "OnDemandServerMediaSubsession.hh"
#include <GroupsockHelper.hh>

#include <iostream>
using namespace std;

OnDemandServerMediaSubsession::OnDemandServerMediaSubsession(UsageEnvironment &env, Boolean reuseFirstSource,
                                                             portNumBits initialPortNum)
        : ServerMediaSubsession(env)
        , fSDPLines(NULL)
        , fReuseFirstSource(reuseFirstSource)
        , fInitialPortNum(initialPortNum)
        , fLastStreamToken(NULL)
{
	printf("OnDemandServerMediaSubsession::%s: Create hash table...\n", __func__);
	fDestinationsHashTable = HashTable::create(ONE_WORD_HASH_KEYS);
	gethostname(fCNAME, sizeof fCNAME);
	fCNAME[sizeof fCNAME - 1] = '\0'; // just in case
	printf("OnDemandServerMediaSubsession::%s: hash table created...\n", __func__);
}

OnDemandServerMediaSubsession::~OnDemandServerMediaSubsession()
{
	delete[] fSDPLines;

	// Clean out the destinations hash table:
	while (1) {
		Destinations *destinations = (Destinations *)(fDestinationsHashTable->RemoveNext());
		if (destinations == NULL)
			break;
		delete destinations;
	}
	delete fDestinationsHashTable;
}

char const *OnDemandServerMediaSubsession::sdpLines()
{
	if (1) { /*every connect create new source then close it to get new SDP */
		// We need to construct a set of SDP lines that describe this
		// subsession (as a unicast stream).  To do so, we first create
		// dummy (unused) source and "RTPSink" objects,
		// whose parameters we use for the SDP lines:
		unsigned estBitrate = 0;
		printf("%s: Create new stream source; est. bitrate = %d...\n", __func__, estBitrate);
		FramedSource *inputSource = createNewStreamSource(0, estBitrate);
		if (inputSource == NULL) {
			fprintf(stderr, "Failed to creat FramedSource.\n");
			return NULL; // file not found
		}
		printf("%s: Stream source created.\n", __func__);

		struct in_addr dummyAddr;
		dummyAddr.s_addr = 0;
		Groupsock dummyGroupsock(envir(), dummyAddr, 0, 0);
		unsigned char rtpPayloadType = 96 + trackNumber() - 1; // if dynamic
		printf("%s: Create dummy RTPSink...\n", __func__);
		RTPSink *dummyRTPSink = createNewRTPSink(&dummyGroupsock, rtpPayloadType, inputSource);
		if (dummyRTPSink != NULL && dummyRTPSink->estimatedBitrate() > 0)
			estBitrate = dummyRTPSink->estimatedBitrate();

		printf("%s: Dummy RTPSink created. Set SDP Lines from the Sink...\n", __func__);
		setSDPLinesFromRTPSink(dummyRTPSink, inputSource, estBitrate);
		printf("%s: SDP Lines set.\n", __func__);
		Medium::close(dummyRTPSink);
		printf("%s: dummyRTPSink closed.\n", __func__);
		closeStreamSource(inputSource); /*close(release) source and sink class just for generate SDP*/
		printf("%s: Stream source closed.\n", __func__);
	}

	return fSDPLines;
}

void OnDemandServerMediaSubsession::getStreamParameters(unsigned clientSessionId, netAddressBits clientAddress,
                                                        Port const &clientRTPPort, Port const &clientRTCPPort,
                                                        int tcpSocketNum, unsigned char rtpChannelId,
                                                        unsigned char rtcpChannelId, netAddressBits &destinationAddress,
                                                        u_int8_t & /*destinationTTL*/, Boolean &isMulticast,
                                                        Port &serverRTPPort, Port &serverRTCPPort, void *&streamToken)
{
	if (destinationAddress == 0)
		destinationAddress = clientAddress;
	struct in_addr destinationAddr;
	destinationAddr.s_addr = destinationAddress;
	isMulticast = False;

	if (fLastStreamToken != NULL && fReuseFirstSource) {
		// Special case: Rather than creating a new 'StreamState',
		// we reuse the one that we've already created:
		serverRTPPort = ((StreamState *)fLastStreamToken)->serverRTPPort();
		serverRTCPPort = ((StreamState *)fLastStreamToken)->serverRTCPPort();
		++((StreamState *)fLastStreamToken)->referenceCount();
		streamToken = fLastStreamToken;
	} else {
		// Normal case: Create a new media source:
		unsigned streamBitrate;
		FramedSource *mediaSource = createNewStreamSource(clientSessionId, streamBitrate);
		if (mediaSource == NULL) {
			fprintf(stderr, "failed to create source, ignore %x\r\n", clientSessionId);
			return;
		}

		// Create 'groupsock' and 'sink' objects for the destination,
		// using previously unused server port numbers:
		RTPSink *rtpSink = NULL;
		BasicUDPSink *udpSink = NULL;
		Groupsock *rtpGroupsock = NULL;
		Groupsock *rtcpGroupsock = NULL;

		if (clientRTPPort.num() != 0 || tcpSocketNum >= 0) { // Normal case: Create destinations
			portNumBits serverPortNum;
			if (clientRTCPPort.num() == 0) {
				// We're streaming raw UDP (not RTP). Create a single groupsock:
				NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
				for (serverPortNum = fInitialPortNum;; ++serverPortNum) {
					struct in_addr dummyAddr;
					dummyAddr.s_addr = 0;

					serverRTPPort = serverPortNum;
					rtpGroupsock = new Groupsock(envir(), dummyAddr, serverRTPPort, 255);
					if (rtpGroupsock->socketNum() >= 0)
						break; // success
				}

				udpSink = BasicUDPSink::createNew(envir(), rtpGroupsock);
			} else {
				// Normal case: We're streaming RTP (over UDP or TCP).  Create a pair of
				// groupsocks (RTP and RTCP), with adjacent port numbers (RTP port
				// number even):
				NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
				for (portNumBits serverPortNum = fInitialPortNum;; serverPortNum += 2) {
					struct in_addr dummyAddr;
					dummyAddr.s_addr = 0;

					serverRTPPort = serverPortNum;
					rtpGroupsock = new Groupsock(envir(), dummyAddr, serverRTPPort, 255);
					if (rtpGroupsock->socketNum() < 0) {
						delete rtpGroupsock;
						continue; // try again
					}

					serverRTCPPort = serverPortNum + 1;
					rtcpGroupsock = new Groupsock(envir(), dummyAddr, serverRTCPPort, 255);
					if (rtcpGroupsock->socketNum() < 0) {
						delete rtpGroupsock;
						delete rtcpGroupsock;
						continue; // try again
					}

					break; // success
				}

				unsigned char rtpPayloadType = 96 + trackNumber() - 1; // if dynamic
				rtpSink = createNewRTPSink(rtpGroupsock, rtpPayloadType, mediaSource);
				if (rtpSink != NULL && rtpSink->estimatedBitrate() > 0)
					streamBitrate = rtpSink->estimatedBitrate();
			}

			// Turn off the destinations for each groupsock.  They'll get set later
			// (unless TCP is used instead):
			if (rtpGroupsock != NULL)
				rtpGroupsock->removeAllDestinations();
			if (rtcpGroupsock != NULL)
				rtcpGroupsock->removeAllDestinations();

			if (rtpGroupsock != NULL) {
				// Try to use a big send buffer for RTP -  at least 0.1 second of
				// specified bandwidth and at least 50 KB
				unsigned rtpBufSize = streamBitrate * 25 / 2; // 1 kbps * 0.1 s = 12.5 bytes
				if (rtpBufSize < 50 * 1024)
					rtpBufSize = 50 * 1024;
				increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), rtpBufSize);
			}
		}

		if (rtpSink != NULL) {
			// Set up the state of the stream.  The stream will get started later:
			streamToken = fLastStreamToken = new StreamState(*this, serverRTPPort, serverRTCPPort, rtpSink,
			                                                 udpSink, streamBitrate, mediaSource,
			                                                 rtpGroupsock, rtcpGroupsock);
		} else {
			fprintf(stderr, "failed to create Sink, ignore %x\r\n", clientSessionId);
			return;
		}
	}

	// Record these destinations as being for this client session id:
	Destinations *destinations;
	if (tcpSocketNum < 0) { // UDP
		destinations = new Destinations(destinationAddr, clientRTPPort, clientRTCPPort);
	} else { // TCP
		destinations = new Destinations(tcpSocketNum, rtpChannelId, rtcpChannelId);
	}
	fDestinationsHashTable->Add((char const *)clientSessionId, destinations);
}

void OnDemandServerMediaSubsession::startStream(unsigned clientSessionId, void *streamToken, TaskFunc *rtcpRRHandler,
                                                void *rtcpRRHandlerClientData, unsigned short &rtpSeqNum,
                                                unsigned &rtpTimestamp,
                                                ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
                                                void *serverRequestAlternativeByteHandlerClientData)
{
	printf("%s %x\n", __func__, clientSessionId);
	StreamState *streamState = (StreamState *)streamToken;
	Destinations *destinations = (Destinations *)(fDestinationsHashTable->Lookup((char const *)clientSessionId));
	if (streamState != NULL) {
		streamState->startPlaying(destinations, rtcpRRHandler, rtcpRRHandlerClientData,
		                          serverRequestAlternativeByteHandler,
		                          serverRequestAlternativeByteHandlerClientData);
		RTPSink *rtpSink = streamState->rtpSink(); // alias
		if (rtpSink != NULL) {
			rtpSeqNum = rtpSink->currentSeqNo();
			rtpTimestamp = rtpSink->presetNextTimestamp();
		}
	}
}

void OnDemandServerMediaSubsession::pauseStream(unsigned /*clientSessionId*/, void *streamToken)
{
	// Pausing isn't allowed if multiple clients are receiving data from
	// the same source:
	if (fReuseFirstSource)
		return;

	StreamState *streamState = (StreamState *)streamToken;
	if (streamState != NULL)
		streamState->pause();
}

void OnDemandServerMediaSubsession::seekStream(unsigned /*clientSessionId*/, void *streamToken, double &seekNPT,
                                               double streamDuration, u_int64_t &numBytes)
{
	numBytes = 0; // by default: unknown

	// Seeking isn't allowed if multiple clients are receiving data from the same
	// source:
	if (fReuseFirstSource)
		return;

	StreamState *streamState = (StreamState *)streamToken;
	if (streamState != NULL && streamState->mediaSource() != NULL) {
		seekStreamSource(streamState->mediaSource(), seekNPT, streamDuration, numBytes);

		streamState->startNPT() = (float)seekNPT;
		RTPSink *rtpSink = streamState->rtpSink(); // alias
		if (rtpSink != NULL)
			rtpSink->resetPresentationTimes();
	}
}

void OnDemandServerMediaSubsession::seekStream(unsigned /*clientSessionId*/, void *streamToken, char *&absStart,
                                               char *&absEnd)
{
	// Seeking isn't allowed if multiple clients are receiving data from the same
	// source:
	if (fReuseFirstSource)
		return;

	StreamState *streamState = (StreamState *)streamToken;
	if (streamState != NULL && streamState->mediaSource() != NULL) {
		seekStreamSource(streamState->mediaSource(), absStart, absEnd);
	}
}

void OnDemandServerMediaSubsession::nullSeekStream(unsigned /*clientSessionId*/, void *streamToken,
                                                   double streamEndTime, u_int64_t &numBytes)
{
	numBytes = 0; // by default: unknown

	StreamState *streamState = (StreamState *)streamToken;
	if (streamState != NULL && streamState->mediaSource() != NULL) {
		// Because we're not seeking here, get the current NPT, and remember it as
		// the new 'start' NPT:
		streamState->startNPT() = getCurrentNPT(streamToken);

		double duration = streamEndTime - streamState->startNPT();
		if (duration < 0.0)
			duration = 0.0;
		setStreamSourceDuration(streamState->mediaSource(), duration, numBytes);

		RTPSink *rtpSink = streamState->rtpSink(); // alias
		if (rtpSink != NULL)
			rtpSink->resetPresentationTimes();
	}
}

void OnDemandServerMediaSubsession::setStreamScale(unsigned /*clientSessionId*/, void *streamToken, float scale)
{
	// Changing the scale factor isn't allowed if multiple clients are receiving
	// data
	// from the same source:
	if (fReuseFirstSource)
		return;

	StreamState *streamState = (StreamState *)streamToken;
	if (streamState != NULL && streamState->mediaSource() != NULL) {
		setStreamSourceScale(streamState->mediaSource(), scale);
	}
}

float OnDemandServerMediaSubsession::getCurrentNPT(void *streamToken)
{
	do {
		if (streamToken == NULL)
			break;

		StreamState *streamState = (StreamState *)streamToken;
		RTPSink *rtpSink = streamState->rtpSink();
		if (rtpSink == NULL)
			break;

		return streamState->startNPT() +
		       (rtpSink->mostRecentPresentationTime().tv_sec - rtpSink->initialPresentationTime().tv_sec) +
		       (rtpSink->mostRecentPresentationTime().tv_sec - rtpSink->initialPresentationTime().tv_sec) /
		               1000000.0f;
	} while (0);

	return 0.0;
}

FramedSource *OnDemandServerMediaSubsession::getStreamSource(void *streamToken)
{
	if (streamToken == NULL)
		return NULL;

	StreamState *streamState = (StreamState *)streamToken;
	return streamState->mediaSource();
}

void OnDemandServerMediaSubsession::deleteStream(unsigned clientSessionId, void *&streamToken)
{
	printf("%s %x\n", __func__, clientSessionId);
	StreamState *streamState = (StreamState *)streamToken;

	// Look up (and remove) the destinations for this client session:
	Destinations *destinations = (Destinations *)(fDestinationsHashTable->Lookup((char const *)clientSessionId));
	if (destinations != NULL) {
		fDestinationsHashTable->Remove((char const *)clientSessionId);

		// Stop streaming to these destinations:
		if (streamState != NULL)
			streamState->endPlaying(destinations);
	}

	// Delete the "StreamState" structure if it's no longer being used:
	if (streamState != NULL) {
		if (streamState->referenceCount() > 0)
			--streamState->referenceCount();
		if (streamState->referenceCount() == 0) {
			delete streamState;
			streamToken = NULL;
		}
	}

	// Finally, delete the destinations themselves:
	delete destinations;
}

char const *OnDemandServerMediaSubsession::getAuxSDPLine(RTPSink *rtpSink, FramedSource * /*inputSource*/)
{
	// Default implementation:
	return rtpSink == NULL ? NULL : rtpSink->auxSDPLine();
}

void OnDemandServerMediaSubsession::seekStreamSource(FramedSource * /*inputSource*/, double & /*seekNPT*/,
                                                     double /*streamDuration*/, u_int64_t &numBytes)
{
	// Default implementation: Do nothing
	numBytes = 0;
}

void OnDemandServerMediaSubsession::seekStreamSource(FramedSource * /*inputSource*/, char *&absStart, char *&absEnd)
{
	// Default implementation: do nothing (but delete[] and assign "absStart" and
	// "absEnd" to NULL, to show that we don't handle this)
	delete[] absStart;
	absStart = NULL;
	delete[] absEnd;
	absEnd = NULL;
}

void OnDemandServerMediaSubsession::setStreamSourceScale(FramedSource * /*inputSource*/, float /*scale*/)
{
	// Default implementation: Do nothing
}

void OnDemandServerMediaSubsession::setStreamSourceDuration(FramedSource * /*inputSource*/, double /*streamDuration*/,
                                                            u_int64_t &numBytes)
{
	// Default implementation: Do nothing
	numBytes = 0;
}

void OnDemandServerMediaSubsession::closeStreamSource(FramedSource *inputSource)
{
	Medium::close(inputSource);
}

void OnDemandServerMediaSubsession::setSDPLinesFromRTPSink(RTPSink *rtpSink, FramedSource *inputSource,
                                                           unsigned estBitrate)
{
	if (rtpSink == NULL)
		return;

	char const *mediaType = rtpSink->sdpMediaType();
	unsigned char rtpPayloadType = rtpSink->rtpPayloadType();
	AddressString ipAddressStr(fServerAddressForSDP);
	char *rtpmapLine = rtpSink->rtpmapLine();
	char const *rangeLine = rangeSDPLine();
	char const *auxSDPLine = getAuxSDPLine(rtpSink, inputSource);
	if (auxSDPLine == NULL)
		auxSDPLine = "";

	char const *const sdpFmt = "m=%s %u RTP/AVP %d\r\n"
	                           "c=IN IP4 %s\r\n"
	                           "b=AS:%u\r\n"
	                           "%s"
	                           "%s"
	                           "%s"
	                           "a=control:%s\r\n";
	unsigned sdpFmtSize = strlen(sdpFmt) + strlen(mediaType) + 5 /* max short len */ + 3 /* max char len */
	                      + strlen(ipAddressStr.val()) + 20 /* max int len */
	                      + strlen(rtpmapLine) + strlen(rangeLine) + strlen(auxSDPLine) + strlen(trackId());
	char *sdpLines = new char[sdpFmtSize];
	sprintf(sdpLines, sdpFmt,
	        mediaType, // m= <media>
	        fPortNumForSDP, // m= <port>
	        rtpPayloadType, // m= <fmt list>
	        ipAddressStr.val(), // c= address
	        estBitrate, // b=AS:<bandwidth>
	        rtpmapLine, // a=rtpmap:... (if present)
	        rangeLine, // a=range:... (if present)
	        auxSDPLine, // optional extra SDP line
	        trackId()); // a=control:<track-id>
	delete[](char *) rangeLine;
	delete[] rtpmapLine;

	fSDPLines = strDup(sdpLines);
	delete[] sdpLines;
}

////////// StreamState implementation //////////

static void afterPlayingStreamState(void *clientData)
{
	StreamState *streamState = (StreamState *)clientData;
	if (streamState->streamDuration() == 0.0) {
		// When the input stream ends, tear it down.  This will cause a RTCP "BYE"
		// to be sent to each client, teling it that the stream has ended.
		// (Because the stream didn't have a known duration, there was no other
		//  way for clients to know when the stream ended.)
		streamState->reclaim();
	}
	// Otherwise, keep the stream alive, in case a client wants to
	// subsequently re-play the stream starting from somewhere other than the end.
	// (This can be done only on streams that have a known duration.)
}

StreamState::StreamState(OnDemandServerMediaSubsession &master, Port const &serverRTPPort, Port const &serverRTCPPort,
                         RTPSink *rtpSink, BasicUDPSink *udpSink, unsigned totalBW, FramedSource *mediaSource,
                         Groupsock *rtpGS, Groupsock *rtcpGS)
        : fMaster(master)
        , fAreCurrentlyPlaying(False)
        , fReferenceCount(1)
        , fServerRTPPort(serverRTPPort)
        , fServerRTCPPort(serverRTCPPort)
        , fRTPSink(rtpSink)
        , fUDPSink(udpSink)
        , fStreamDuration(master.duration())
        , fTotalBW(totalBW)
        , fRTCPInstance(NULL) /* created later */
        , fMediaSource(mediaSource)
        , fStartNPT(0.0)
        , fRTPgs(rtpGS)
        , fRTCPgs(rtcpGS)
{
}

StreamState::~StreamState()
{
	reclaim();
}

void StreamState::startPlaying(Destinations *dests, TaskFunc *rtcpRRHandler, void *rtcpRRHandlerClientData,
                               ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
                               void *serverRequestAlternativeByteHandlerClientData)
{
	if (dests == NULL)
		return;

	if (fRTCPInstance == NULL && fRTPSink != NULL) {
		// Create (and start) a 'RTCP instance' for this RTP sink:
		fRTCPInstance = RTCPInstance::createNew(fRTPSink->envir(), fRTCPgs, fTotalBW,
		                                        (unsigned char *)fMaster.fCNAME, fRTPSink,
		                                        NULL /* we're a server */);
		// Note: This starts RTCP running automatically
	}

	if (dests->isTCP) {
		// Change RTP and RTCP to use the TCP socket instead of UDP:
		if (fRTPSink != NULL) {
			fRTPSink->addStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
			RTPInterface::setServerRequestAlternativeByteHandler(
			        fRTPSink->envir(), dests->tcpSocketNum, serverRequestAlternativeByteHandler,
			        serverRequestAlternativeByteHandlerClientData);
			// So that we continue to handle RTSP commands from the client
		}
		if (fRTCPInstance != NULL) {
			fRTCPInstance->addStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);
			fRTCPInstance->setSpecificRRHandler(dests->tcpSocketNum, dests->rtcpChannelId, rtcpRRHandler,
			                                    rtcpRRHandlerClientData);
		}
	} else {
		// Tell the RTP and RTCP 'groupsocks' about this destination
		// (in case they don't already have it):
		if (fRTPgs != NULL)
			fRTPgs->addDestination(dests->addr, dests->rtpPort);
		if (fRTCPgs != NULL)
			fRTCPgs->addDestination(dests->addr, dests->rtcpPort);
		if (fRTCPInstance != NULL) {
			fRTCPInstance->setSpecificRRHandler(dests->addr.s_addr, dests->rtcpPort, rtcpRRHandler,
			                                    rtcpRRHandlerClientData);
		}
	}

	if (fRTCPInstance != NULL) {
		// Hack: Send an initial RTCP "SR" packet, before the initial RTP packet, so
		// that receivers will (likely) be able to
		// get RTCP-synchronized presentation times immediately:
		fRTCPInstance->sendReport();
	}

	if (!fAreCurrentlyPlaying && fMediaSource != NULL) {
		if (fRTPSink != NULL) {
			fRTPSink->startPlaying(*fMediaSource, afterPlayingStreamState, this);
			fAreCurrentlyPlaying = True;
		} else if (fUDPSink != NULL) {
			fUDPSink->startPlaying(*fMediaSource, afterPlayingStreamState, this);
			fAreCurrentlyPlaying = True;
		}
	}
}

void StreamState::pause()
{
	if (fRTPSink != NULL)
		fRTPSink->stopPlaying();
	if (fUDPSink != NULL)
		fUDPSink->stopPlaying();
	fAreCurrentlyPlaying = False;
}

void StreamState::endPlaying(Destinations *dests)
{
#if 0
  // The following code is temporarily disabled, because it erroneously sends RTCP "BYE"s to all clients if multiple
  // clients are streaming from the same data source (i.e., if "reuseFirstSource" is True), and we don't want that to happen
  // if we're being called as a result of a single one of these clients having sent a "TEARDOWN" (rather than the whole stream
  // having been closed, for all clients).
  // This will be fixed for real later.
  if (fRTCPInstance != NULL) {
    // Hack: Explicitly send a RTCP "BYE" packet now, because the code below will prevent that from happening later,
    // when "fRTCPInstance" gets deleted:
    fRTCPInstance->sendBYE();
  }
#endif

	if (dests->isTCP) {
		if (fRTPSink != NULL) {
			fRTPSink->removeStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
		}
		if (fRTCPInstance != NULL) {
			fRTCPInstance->removeStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);
			fRTCPInstance->unsetSpecificRRHandler(dests->tcpSocketNum, dests->rtcpChannelId);
		}
		/*delete socket*/
		if (fRTPSink != NULL) {
			fRTPSink->deleteSocketDescriptor(dests->tcpSocketNum);
		}
	} else {
		// Tell the RTP and RTCP 'groupsocks' to stop using these destinations:
		if (fRTPgs != NULL)
			fRTPgs->removeDestination(dests->addr, dests->rtpPort);
		if (fRTCPgs != NULL)
			fRTCPgs->removeDestination(dests->addr, dests->rtcpPort);
		if (fRTCPInstance != NULL) {
			fRTCPInstance->unsetSpecificRRHandler(dests->addr.s_addr, dests->rtcpPort);
		}
	}
}

void StreamState::reclaim()
{
	// Delete allocated media objects
	Medium::close(fRTCPInstance) /* will send a RTCP BYE */;
	fRTCPInstance = NULL;
	Medium::close(fRTPSink);
	fRTPSink = NULL;
	Medium::close(fUDPSink);
	fUDPSink = NULL;

	fMaster.closeStreamSource(fMediaSource);
	fMediaSource = NULL;
	if (fMaster.fLastStreamToken == this)
		fMaster.fLastStreamToken = NULL;

	delete fRTPgs;
	fRTPgs = NULL;
	delete fRTCPgs;
	fRTCPgs = NULL;
}
