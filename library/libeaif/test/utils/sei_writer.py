import sys
import av
import numpy as np
import json
from typing import Tuple, Union, Iterator
from av.packet import Packet
from av.container import Container


NALU_PREFIX = b'\x00\x00\x00\x01'
NALU_END = b'\x80'
SEI_UUID = b'\x0f\x01\x02\x03\x04multiplayer'
SEI_H264_HEADER = b'\x06\x05'
SEI_HEVC_PREFIX_HEADER = b'\x4e\x01\x05'
SEI_HEVC_SUFFIX_HEADER = b'\x50\x01\x05'

SEI_FORMAT_STR = NALU_PREFIX + b'{}' + b'{}' + SEI_UUID + b'{}' + NALU_END

def sei_format_str(header: str, size_binary: str, payload: str) -> str:
	return NALU_PREFIX + header + size_binary + SEI_UUID + payload + NALU_END

class SEI_TYPE:
	PREFIX=0
	SUFFIX=1

class H264_TYPE:
	CS1=0x01
	CS2=0x02
	CS3=0x03
	CS4=0x04

	PPS=0x08
	SPS=0x07
	SEI=0x06
	IDR=0x05
	ACL=0x09

class HEVC_TYPE:
	VPS=32
	SPS=33
	PPS=34
	ACL=35
	SEI=39
	SEI_SUFFIX=40

def h264_type_convert(val: int) -> int:
	return val & 0x1f

def hevc_type_convert(val: int) -> int:
	return (val & 0x7e) >> 1

class SeiWriter(object):

	__slots__ = ('nalu_types', 'setup_sei_ptr')

	def setup_sei(self, sei_payload: str) -> str:
		size = []
		total_len = len(sei_payload) + len(SEI_UUID)
		while total_len >= 255:
			size += [255]
			total_len -= 255
		size += [total_len]
		size_binary = np.array(size,dtype=np.uint8).tostring()
		return sei_format_str(self.sei_header, size_binary, sei_payload)

	def nalu_type_convert(self, pkt : av.Packet) -> int:
		raise NotImplementedError

	def checkValidPkt(self, pkt : Packet) -> Tuple[int, int]:
		buf = pkt.to_bytes()
		if len(buf) == 0:
			return 2, f'Reach File ends Pkt size is 0'
		location = buf.find(NALU_PREFIX)
		if location == -1:
			print(buf[:len(NALU_PREFIX)+1])
			return 2, f'Cannot Find NALU_PREFIX {location} '
		ele = buf[location+len(NALU_PREFIX)]
		nalu_type = self.nalu_type_convert(ele)
		#=print(f"[67] nalu_type:{nalu_type}: {self.nalu_types} {location}, {ele}, {buf[:len(NALU_PREFIX)+1]}")
		if nalu_type in self.nalu_types:
			return 0, nalu_type
		return 1, nalu_type

	def write(self, container : Container, stream, pkt : Packet, sei_payload : str) -> Union[int, None]:
		self.idx += 1
		#print(f"[79] frame:{self.idx} len:{len(pkt.to_bytes())}")
		ret, nalu_type = self.checkValidPkt(pkt)
		if ret == 2:
			print(f"TBD Cannot insert SEI: {nalu_type} for {self.idx} packet, pkt len:{len(pkt.to_bytes())}")
			container.write(pkt)
			return 1
		if ret == 1:
			print(f"SKIP insert SEI: {nalu_type} for {self.idx} packet pkt len:{len(pkt.to_bytes())}")
			container.write(pkt)
			return 1
		sei_payload = self.setup_sei_ptr(sei_payload)
		sei = Packet(sei_payload)

		if self.sei_type == SEI_TYPE.PREFIX:
			container.write(sei)
			container.write(pkt)
		else:
			container.write(pkt)
			container.write(sei)
		return None

	def setup_sei_plain(self, sei_payload : bytes) -> bytes:
		return sei_payload

class H264SeiWriter(SeiWriter):

	h264_type = [H264_TYPE.SPS, H264_TYPE.PPS, H264_TYPE.CS1, H264_TYPE.CS2, H264_TYPE.CS3, H264_TYPE.CS4, H264_TYPE.SEI]

	def __init__(self, sei_type : int = SEI_TYPE.PREFIX):
		self.setup_sei_ptr = self.setup_sei
		self.sei_type = sei_type
		self.sei_header = SEI_H264_HEADER
		self.nalu_types = self.h264_type
		self.idx = -1

	def nalu_type_convert(self, pkt : Packet) -> int:
		return h264_type_convert(pkt)

class HevcSeiWriter(SeiWriter):

	hevc_type = [HEVC_TYPE.VPS, HEVC_TYPE.PPS, HEVC_TYPE.SEI]

	def __init__(self, sei_type : int = SEI_TYPE.PREFIX):
		self.setup_sei_ptr = self.setup_sei
		self.sei_type = sei_type
		self.nalu_types = self.hevc_type
		self.idx = -1
		if sei_type == SEI_TYPE.PREFIX:
			self.sei_header = SEI_HEVC_PREFIX_HEADER
		else:
			self.sei_header = SEI_HEVC_SUFFIX_HEADER

	def nalu_type_convert(self, pkt : av.Packet) -> int:
		return hevc_type_convert(pkt)

class OD(object):

	__slots__=('id','life','rect','mv')

	def __init__(self, *args):
		self.id = int(args[0])
		self.life = int(args[1])
		self.rect = (int(args[2]),int(args[3]),int(args[4]),int(args[5]))
		self.mv = (int(args[6]),int(args[7]))

def convert_jsonStr(obj_list : OD, timestamp : int = 0, blank : int = 0) -> bytes:
	if blank == -1:
		return b'{"agtx":{"timestamp":%d,"chn":0,"iva":{"od":[{"obj":{"rect":[100,100,200,200],"id":0,"cat":"","life":160}}]}}}' % timestamp
	jdict = {"agtx":{"timestamp":timestamp,"iva":{"od":[]}}}
	OD = jdict['agtx']['iva']['od']
	for od in obj_list:
		obj = {"obj":{"id":od.id,"life":od.life,"rect":od.rect,"vel":[-od.mv[0], -od.mv[1]],"cat":""}}
		OD.append(obj)
	js = json.dumps(jdict,separators=(',',':')).encode()
	return js

def verify_sei(data : bytes) -> Tuple[int, int]:
	idx = 0
	data_size = 0
	while data[idx] == 255:
		data_size += 255
		idx+=1
	data_size += data[idx]
	if data[idx+data_size+1] == 0x80:
		return 1, data_size
	return 0, data_size

class metadata_reader(object):

	def __init__(self, filename : str, fps : float, jiffy_hz : int = 100):
		self.b_sei_file = 0
		if 'sei' in filename:
			self.__init__plainSei(filename)
			self.b_sei_file = 1
			self.next_ptr = self.__next__plainSei
		else:
			self.__init__metadata(filename, fps, jiffy_hz)
			self.next_ptr = self.__next__MetaData

	def __init__plainSei(self, filename : str) -> None:
		assert 'sei' in filename, "File does not named sei"
		f = open(filename,'rb').read()
		self.header = NALU_PREFIX + ( \
			SEI_H264_HEADER if '264' in filename else \
			SEI_HEVC_PREFIX_HEADER )
		self.header_len = len(self.header)
		sei_data = self.parse_plainSei(f)
		self.idx = 0
		self.list = sei_data
		self.__next__ = self.__next__plainSei

	def parse_plainSei(self, f_content : bytes) -> Iterator[bytes]:
		sei_list = f_content.split(self.header)
		sei_list.pop(0)
		j = 0
		for i in sei_list:
			b, v = verify_sei(i)
			j += 1
			if not b:
				print("Invalid SEI content! Res:" + str(v))
				print(j, b, v, i[v+1:])
				print("HEADER", self.header)
		sei_list = [self.header + i for i in sei_list]
		return sei_list

	def __next__(self) -> bytes:
		return self.next_ptr()

	def __next__plainSei(self) -> bytes:
		if self.idx < len(self.list):
			data = self.list[self.idx]
			self.idx += 1
		else:
			data = self.list[self.idx-1]
		return data

	def __init__metadata(self, filename : str, fps : float, jiffy_hz : int = 100) -> None:
		f = open(filename,'r').read()
		obj_list = self.parse_metaData(f)
		self.time_inc = int(1./fps * jiffy_hz)
		self.timestamp = 0
		self.idx = 0
		self.list = obj_list
		self.__next__ = self.__next__MetaData

	def parse_metaData(self, f_content : str) -> OD:
		ols = []
		for li in f_content.split("\n"):
			if li == '':
				continue
			li = li.split(',')
			fr = int(li[0])
			nb = int(li[1])
			idx = 2
			ol = []
			if not nb:
				ols.append((fr, []))
				continue
			for i in range(nb):
				ol.append(OD(*li[idx:idx+8]))
				idx+=8
			ols.append((fr, ol))
		return ols

	def __next__MetaData(self) -> bytes:
		if self.idx < len(self.list):
			fr, ol = self.list[self.idx]
			ret = convert_jsonStr(ol, self.timestamp)
		else:
			ret = convert_jsonStr(None, self.timestamp, -1)
		self.idx += 1
		self.timestamp += self.time_inc
		return ret


class VideoFormat(object):
	__slots__ = (
		'codec', 'w', 'h', 'fps', 'bitrate', 'pix_fmt', 'b_sei_file'
		)

class WriteSei2Video(object):

	def setup(self, ifilename, ofilename, metadata_file, sei_type=SEI_TYPE.PREFIX):
		self.ifmt, self.icontainer = self.setup_inputContainer(ifilename)
		self.ocontainer = self.setup_outputContainer(ofilename)

		self.sei_type = sei_type
		self.sei_payload = b''
		self.metadata_reader = metadata_reader(metadata_file, self.ifmt.fps)
		self.b_sei_file = self.metadata_reader.b_sei_file
		self.writer = self.setup_writer()

	def setup_inputContainer(self, filename : str) -> Tuple[VideoFormat, Container]:
		# load video conf
		fmt = VideoFormat()
		container = av.open(filename)
		stream = None
		for i in container.streams:
			if i.type == 'video':
				stream = i
		fmt.fps = stream.base_rate.numerator * 1.0 / stream.base_rate.denominator
		if stream.name.find('264') > -1: 
			fmt.codec = 'libx264'
		elif stream.name.find('hevc') > -1 or stream.name.find('265') > -1:
			fmt.codec = 'hevc'
		fmt.pix_fmt = stream.format.name
		fmt.w = stream.format.width
		fmt.h = stream.format.height
		return fmt, container

	def setup_outputContainer(self, filename : str):
		# open and setup output params
		fmt = self.ifmt
		#output = av.open(filename, 'w')
		#stream = output.add_stream(fmt.codec, fmt.fps)
		#stream.width = fmt.w
		#stream.height = fmt.h
		#stream.pix_fmt = fmt.pix_fmt
		#return output
		f = open(filename, 'wb')
		return f

	def setup_writer(self) -> SeiWriter:
		writer = None
		if self.ifmt.codec.find('264') > -1:
			writer = H264SeiWriter(self.sei_type)
		elif (self.ifmt.codec.find('hevc') > -1 or
		    self.ifmt.codec.find('265') > -1):
			writer = HevcSeiWriter(self.sei_type)
		if self.b_sei_file and writer is not None:
			writer.setup_sei_ptr = writer.setup_sei_plain
			return writer
		if writer is None:
			raise RuntimeError(f"Codec {self.ifmt.codec} does not support SEI")

	def setup_sei(self, sei_payload : bytes):
		self.sei_payload = self.writer.setup_sei(sei_payload)

	def help(self, assert_msg : str) -> str :
		msg = "%s\n\n\t Usage:\n\t\tMultiplayer utils app.py <ifile> <ofile> <metadir>" % assert_msg
		msg += "\n\t\t<metadir> - plain sei -> 264/265.sei\n"
		msg += "\t\t<metadir> - json text -> <name>.txt\n"

		msg += "\n\n For filtering sei from video see\n" + \
		       "ffmpeg -i <input(264)> -c:v copy -bsf:v 'filter_units=pass_types=1-5' <output>\n" + \
		       "ffmpeg -i <input(HEVC> -c:v copy -bsf:v 'filter_units=pass_types=39-40' <output>\n"
		return msg

	def run(self, *args : str) -> None:
		try:
			assert len(args[1:]) == 3, self.help("input args should be equal to 3")

			print("[WARNING] Current OD.txt insertion have to reverse od mv sign!")

			ifile, ofile, metafile = args[1:]
			self.setup(ifile, ofile, metafile)
			writer = self.writer
			inContainer = self.icontainer
			output = self.ocontainer

			video_stream = next(s for s in inContainer.streams if s.type == 'video')
			frame_count = 0
			sei_payload = b''
			skip = 0
			for pkt in inContainer.demux(video_stream):
				if not skip:
					sei_payload = next(self.metadata_reader)
				skip = writer.write(output, video_stream, pkt, sei_payload)
				frame_count += 1
				if frame_count % 20 == 0:
					print(f"Finish frame count: {frame_count}")
			output.close()
			print(f"Finish frame count: {frame_count}")
		except:
			import traceback
			print(traceback.format_exc())
			self.help("")

app = WriteSei2Video()

if __name__ == '__main__':
	import sys
	app.run(*sys.argv)