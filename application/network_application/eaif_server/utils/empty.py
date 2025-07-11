###############################
## requirement 
## : python3
##
## dependecy installation
## $ pip3 install flask --user
##
## usage:
## $ python3 empty.py
############################### 

from flask import Flask, jsonify, request
import json
import sys

app = Flask(__name__)

def read_from_buffer():
	time_stamp = None
	meta = None
	if "time" in request.files.keys():
		time_stamp = int(request.files["time"].read())
	if "meta" in request.files.keys():
		meta = request.files["meta"].read()
	return (time_stamp, meta)

def decode_buffer(meta):
	if meta is None or meta == [] or meta == b'None':
		return []
	_json = json.loads(meta.decode('utf-8'))
	od = _json.get('od')
	if not od:
		return []
	box_list = []
	for obj in od:
		_id = int(obj.get('obj').get('id'))
		sx, sy, ex, ey = obj.get('obj').get('rect')
		bbox = [_id, sx, sy, ex, ey]
		if sx < ex and sy < ey:
			box_list.append(bbox)
		else:
			print("[WARN] rect:%d [%d %d %d %d] is not valid", *bbox)
	return box_list

conf = 0

def getDataRet():
	time_stamp, meta = read_from_buffer()
	try:
		box_list = decode_buffer(meta)
	except:
		import traceback
		print(traceback.format_exc(), "value", meta)
		return jsonify({"fail":1})
	data = {"success":1, "pred_num":len(box_list),
	"predictions":[], "time":time_stamp}
	global conf
	conf = (conf + 1) % 100
	for obj in box_list:
		data["predictions"].append(
			{"idx": obj[0], "label_num":1,
			 "label": ['person'],
			 "prob": [conf/100.]}
			 )
	return jsonify(data)

A = 0 
R = 5

def getDataRetBox():
	global A, R
	time_stamp, meta = read_from_buffer()
	try:
		box_list = decode_buffer(meta)
	except:
		import traceback
		print(traceback.format_exc(), "value", meta)
		return jsonify({"fail":1, "meta":meta})
	data = {"success":1, "pred_num":5,
	"predictions":[], "time":time_stamp}
	global conf
	conf = (conf + 1) % 100
	box_list = [0,0,0,0,0]
	for i in range(5):
		A += 150
		A %= 1000
		data["predictions"].append(
			{"idx": 0, "label_num":1,
			 "label": ['person'],
			 "prob": [conf/100.],
			 "rect": [A, i*100, A+200, i*100+200]}
			 )
	return jsonify(data)

def getData():
	data = {"success":1, "pred_num":0,
	"predictions":[], "time":99999}
	return jsonify(data)

def get():
	return getData()

def predict(path):
	print(f" * Get POST for ... {path} from {request.remote_addr}")
	if 'yolo' in path:
		return getDataRetBox()
	return getDataRet()

app.add_url_rule('/', 'get', get, methods=["GET"])
app.add_url_rule('/predict/<path:path>', 'predict', predict, methods=["POST","GET"])

if __name__ == '__main__':
	args = sys.argv
	host = "0.0.0.0"
	port = 8048
	if len(args) == 3:
		host = args[1]
		port = int(args[2])
	print(f"test server host at {host}:{port}")
	app.run(host, port)

