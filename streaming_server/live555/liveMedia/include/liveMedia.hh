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
// Inclusion of header files representing the interface
// for the entire library
//
// Programs that use the library can include this header file,
// instead of each of the individual media header files

#ifndef _LIVEMEDIA_HH
#define _LIVEMEDIA_HH

#include "AC3AudioFileServerMediaSubsession.hh"
#include "AC3AudioRTPSink.hh"
#include "AC3AudioRTPSource.hh"
#include "AC3AudioStreamFramer.hh"
#include "ADTSAudioFileServerMediaSubsession.hh"
#include "ADTSAudioFileSource.hh"
#include "AMRAudioFileServerMediaSubsession.hh"
#include "AMRAudioFileSink.hh"
#include "AMRAudioFileSource.hh"
#include "AMRAudioRTPSink.hh"
#include "AMRAudioRTPSource.hh"
#include "AVIFileSink.hh"
#include "AudioInputDevice.hh"
#include "BasicUDPSink.hh"
#include "BasicUDPSource.hh"
#include "ByteStreamMemoryBufferSource.hh"
#include "ByteStreamMultiFileSource.hh"
#include "DVVideoFileServerMediaSubsession.hh"
#include "DVVideoRTPSink.hh"
#include "DVVideoRTPSource.hh"
#include "DVVideoStreamFramer.hh"
#include "DarwinInjector.hh"
#include "DeviceSource.hh"
#include "GSMAudioRTPSink.hh"
#include "H261VideoRTPSource.hh"
#include "H263plusVideoFileServerMediaSubsession.hh"
#include "H263plusVideoRTPSink.hh"
#include "H263plusVideoRTPSource.hh"
#include "H263plusVideoStreamFramer.hh"
#include "H264VideoFileServerMediaSubsession.hh"
#include "H264VideoFileSink.hh"
#include "H264VideoRTPSink.hh"
#include "H264VideoRTPSource.hh"
#include "H264VideoStreamDiscreteFramer.hh"
#include "H264VideoStreamFramer.hh"
#include "H265VideoFileServerMediaSubsession.hh"
#include "H265VideoFileSink.hh"
#include "H265VideoRTPSink.hh"
#include "H265VideoRTPSource.hh"
#include "H265VideoStreamDiscreteFramer.hh"
#include "H265VideoStreamFramer.hh"
#include "JPEGVideoRTPSink.hh"
#include "JPEGVideoRTPSource.hh"
#include "JPEGVideoSource.hh"
#include "MP3ADU.hh"
#include "MP3ADURTPSink.hh"
#include "MP3ADURTPSource.hh"
#include "MP3ADUinterleaving.hh"
#include "MP3AudioFileServerMediaSubsession.hh"
#include "MP3FileSource.hh"
#include "MP3Transcoder.hh"
#include "MPEG1or2AudioRTPSink.hh"
#include "MPEG1or2AudioRTPSource.hh"
#include "MPEG1or2AudioStreamFramer.hh"
#include "MPEG1or2DemuxedElementaryStream.hh"
#include "MPEG1or2FileServerDemux.hh"
#include "MPEG1or2VideoFileServerMediaSubsession.hh"
#include "MPEG1or2VideoRTPSink.hh"
#include "MPEG1or2VideoRTPSource.hh"
#include "MPEG1or2VideoStreamDiscreteFramer.hh"
#include "MPEG2IndexFromTransportStream.hh"
#include "MPEG2TransportFileServerMediaSubsession.hh"
#include "MPEG2TransportStreamFramer.hh"
#include "MPEG2TransportStreamFromESSource.hh"
#include "MPEG2TransportStreamFromPESSource.hh"
#include "MPEG2TransportStreamTrickModeFilter.hh"
#include "MPEG2TransportUDPServerMediaSubsession.hh"
#include "MPEG4ESVideoRTPSink.hh"
#include "MPEG4ESVideoRTPSource.hh"
#include "MPEG4GenericRTPSink.hh"
#include "MPEG4GenericRTPSource.hh"
#include "MPEG4LATMAudioRTPSink.hh"
#include "MPEG4LATMAudioRTPSource.hh"
#include "MPEG4VideoFileServerMediaSubsession.hh"
#include "MPEG4VideoStreamDiscreteFramer.hh"
#include "MatroskaFileServerDemux.hh"
#include "OggFileServerDemux.hh"
#include "OggFileSink.hh"
#include "PassiveServerMediaSubsession.hh"
#include "ProxyServerMediaSession.hh"
#include "QCELPAudioRTPSource.hh"
#include "QuickTimeFileSink.hh"
#include "QuickTimeGenericRTPSource.hh"
#include "RTSPClient.hh"
#include "RTSPRegisterSender.hh"
#include "RTSPServerSupportingHTTPStreaming.hh"
#include "SIPClient.hh"
#include "SimpleRTPSink.hh"
#include "SimpleRTPSource.hh"
#include "StreamReplicator.hh"
#include "T140TextRTPSink.hh"
#include "TCPStreamSink.hh"
#include "TheoraVideoRTPSink.hh"
#include "TheoraVideoRTPSource.hh"
#include "VP8VideoRTPSink.hh"
#include "VP8VideoRTPSource.hh"
#include "VorbisAudioRTPSink.hh"
#include "VorbisAudioRTPSource.hh"
#include "WAVAudioFileServerMediaSubsession.hh"
#include "WAVAudioFileSource.hh"
#include "uLawAudioFilter.hh"

#include "AuxVidDeviceServerMediaSubsession.hh"
#include "AuxVidDeviceSource.hh"

#include "AuxAudioDeviceSource.hh"
#include "AuxAudioDeviceServerMediaSubsession.hh"

#endif
