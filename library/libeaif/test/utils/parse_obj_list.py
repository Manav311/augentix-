import numpy as np
import json
import sys
import os

H264 = 0
HEVC = 1

mpi_rect = np.dtype([
	("sx", np.int16),
	("sy", np.int16),
	("ex", np.int16),
	("ey", np.int16)
	])
mpi_mv = np.dtype([
	("x", np.int16),
	("y", np.int16),
	])
mpi_iva_obj_attr = np.dtype([
	("id", np.int32),
	("life", np.int16),
	("rect", mpi_rect),
	("mv", mpi_mv),
	("pad", np.int16)
	])
mpi_iva_obj_list = np.dtype([
	("timestamp", np.uint32),
	("obj_num", np.int32),
	("obj", mpi_iva_obj_attr, 10)
	])


def parse264Sei(binary):
	uuid_len = 16
	data = binary.split(b"\x00\x00\x00\x01\x06\x05")
	data.pop(0)
	size = len(data)
	print("L38 Total %d frames!" % size)

	obj_lists = np.zeros(size, dtype=mpi_iva_obj_list)
	k = 0
	i = 0
	for i in range(size):
		if i % 20 == 0:
			print("Processing %d:%d/%d" % (i,k,size))
		meta = data[i]
		obj_list = obj_lists[k]
		payload_len = 0
		pos = 0
		if b"agtx" not in meta:
			print("Skip", i, meta)
			continue
		while meta[pos] == 0xff:
			payload_len += 0xff
			pos += 1
		payload_len += meta[pos]
		k += 1
		if (meta[pos + payload_len + 1] != 0x80):
			print("not valid sei! msg: ", meta)
			sys.exit()
		payload = meta[pos+uuid_len+1:pos+payload_len+1]
		agtx = json.loads(payload)['agtx']
		#print("L59",agtx, obj_list)
		try:
			obj_list[0] = agtx["timestamp"]
		except KeyError:
			obj_list[0] = agtx["time"]
	
		iva = agtx.get('iva')
		od = iva.get("od")
		obj_list[1] = len(od)
		if obj_list[1]:
			for j in range(len(od)):
				obj = od[j]["obj"]
				lobj = obj_list[2][j]
				lobj[0] = obj["id"]
				lobj[1] = obj["life"]
				lobj[2][0] = obj["rect"][0]
				lobj[2][1] = obj["rect"][1]
				lobj[2][2] = obj["rect"][2]
				lobj[2][3] = obj["rect"][3]
				try:
					lobj[3][0] = obj["vel"][0]
					lobj[3][1] = obj["vel"][1]
				except KeyError:
					lobj[3][0] = 0
					lobj[3][1] = 0
					print("Warning sei missing vel at %d" % i)
	print("Processing %d:%d/%d" % (i,k,size))
	return obj_lists

def parse265Sei(binary):
	uuid_len = 16
	data = binary.split(b"\x00\x00\x00\x01\x4e\x01\x05")
	data.pop(0)
	size = len(data)
	print("L38 Total %d frames!" % size)

	obj_lists = np.zeros(size, dtype=mpi_iva_obj_list)
	k = 0
	i = 0
	for i in range(size):
		if i % 20 == 0:
			print("Processing %d:%d/%d" % (i,k,size))
		meta = data[i]
		obj_list = obj_lists[k]
		payload_len = 0
		pos = 0
		if b"agtx" not in meta:
			print("Skip", i, meta)
			continue
		while meta[pos] == 0xff:
			payload_len += 0xff
			pos += 1
		payload_len += meta[pos]
		k += 1
		if (meta[pos + payload_len + 1] != 0x80):
			print("not valid sei! msg: ", meta)
			sys.exit()
		payload = meta[pos+uuid_len+1:pos+payload_len+1]
		agtx = json.loads(payload)['agtx']
		#print("L59",agtx, obj_list)
		try:
			obj_list[0] = agtx["timestamp"]
		except KeyError:
			obj_list[0] = agtx["time"]
	
		iva = agtx.get('iva')
		od = iva.get("od")
		obj_list[1] = len(od)
		if obj_list[1]:
			for j in range(len(od)):
				obj = od[j]["obj"]
				lobj = obj_list[2][j]
				lobj[0] = obj["id"]
				lobj[1] = obj["life"]
				lobj[2][0] = obj["rect"][0]
				lobj[2][1] = obj["rect"][1]
				lobj[2][2] = obj["rect"][2]
				lobj[2][3] = obj["rect"][3]
				try:
					lobj[3][0] = obj["vel"][0]
					lobj[3][1] = obj["vel"][1]
				except KeyError:
					lobj[3][0] = 0
					lobj[3][1] = 0
					print("Warning sei missing vel at %d" % i)
	print("Processing %d:%d/%d" % (i,k,size))
	return obj_lists

def parse_and_save(scheme : int , obj_list_file : str):
	fr = open(obj_list_file, 'rb')
	data = fr.read()
	fr.close()
	obj_lists = parse264Sei(data) if scheme == H264 else parse265Sei(data)
	binary_file = obj_list_file[:-3] + "bin"
	binary = obj_lists.tostring()
	print("Saving to %s" % binary_file)
	with open(binary_file, "wb") as f:
		f.write(binary)

def main():
	assert len(sys.argv) == 3, "Usage <prog> [\"264\"/\"265\" [\"obj-list.sei\"]]"
	obj_list_file = sys.argv[2]
	scheme = H264 if sys.argv[1] == "264" else HEVC
	if not os.path.exists(obj_list_file):
		print("Cannot find", obj_list_file)
		return
	parse_and_save(scheme, obj_list_file)

if __name__ == "__main__":
	main()