import cv2
import numpy as np
import sys
import os
import json
import traceback

fourcc = cv2.VideoWriter_fourcc(*'mp4v')
life_th = 0
draw_life = 1
OBJ_NO = 128
__COLOR_RAND = [np.random.randint(150, 255, size=(3)).tolist() for i in range(OBJ_NO)]

class COLORE:
	BLUE = (255,70,70)
	BLACK = (0,0,0)
	RED = (70,70,255)
	WHITE = (255,255,255)
	GREY = (120, 120, 120)
	GREEN = (70,255,70)

def info(*args):
	print("[INFO]", *args)

class Vid(object):
	def __init__(self, video_fn):
		cap = cv2.VideoCapture(video_fn)
		if not cap.isOpened():
			help("Cannot open %s" % video_fn)
			sys.exit(0)
		self.fps = float(cap.get(cv2.CAP_PROP_FPS))
		self.w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
		self.h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
		self.cap = cap
		info("Video info %dx%d : %.4f : %s" % (self.w, self.h, self.fps, video_fn))

def LoadLog(log_fn):
	fp = open(log_fn, "r")
	if not fp:
		help("Cannot open %s" % log_fn)
		sys.exit(0)
	obj_lists = []
	lines = fp.readlines()
	fp.close()
	for l in lines:
		try:
			ol = json.loads(l)
		except:
			print(traceback.format_exc())
			print(l)
			sys.exit(0)
		obj_lists.append(ol)
	info("total %d Lines from log file : %s" % (len(obj_lists), log_fn))
	return obj_lists

def draw_frame(fr, ol, fr_id):
	COLOR = COLORE
	COLOR_RAND = __COLOR_RAND

	cv2.putText(fr, "%-4d" % fr_id, (0,40), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 0), 2, cv2.LINE_AA)

	for obj in ol["od"]:
		box = obj['obj']
		if (box['life'] < life_th):
			continue
		o_id = box['id']
		box_id = o_id % OBJ_NO # temp solution
		cat = box['cat']
		color = COLOR_RAND[box_id]
		back = (0,0,0)
		sx, sy, ex, ey = box["rect"]
		draw_cat = 0
		prob = box['conf']
		if cat is not None or cat != "":
			draw_cat = 1
		if draw_cat:
			if "person" in cat or "human" in cat:
				cat = "human"
				color = COLOR.BLUE
			else:
				color = COLOR.GREY
			back = COLOR.WHITE
		st = (sx, sy)
		et = (ex, ey)

		image_in = cv2.rectangle(fr, st, et, color, thickness = 3)
		cv2.putText(image_in, prob, (sx, sy), cv2.FONT_HERSHEY_SIMPLEX, 1.2, color, 3, 16)
		if draw_cat:
			cv2.putText(image_in, cat, (sx+4, ey-15), cv2.FONT_HERSHEY_SIMPLEX, 1.2, back, 3, 16)
			cv2.putText(image_in, cat, (sx+8, ey-15), cv2.FONT_HERSHEY_SIMPLEX, 1.2, back, 3, 16)
			cv2.putText(image_in, cat, (sx+6, ey-18), cv2.FONT_HERSHEY_SIMPLEX, 1.2, back, 3, 16)
			cv2.putText(image_in, cat, (sx+6, ey-12), cv2.FONT_HERSHEY_SIMPLEX, 1.2, back, 3, 16)
			cv2.putText(image_in, cat, (sx+6, ey-15), cv2.FONT_HERSHEY_SIMPLEX, 1.2, color, 3, 16)
	return fr

def draw_fr_frame(fr, ol, fr_id):
	COLOR = COLORE
	COLOR_RAND = __COLOR_RAND

	cv2.putText(fr, "%-4d" % fr_id, (0,40), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 0), 2, cv2.LINE_AA)

	for obj in ol["od"]:
		box = obj['obj']
		if (box['life'] < life_th):
			continue
		o_id = box['id']
		box_id = o_id % OBJ_NO # temp solution
		cat = box['cat']
		color = COLORE.GREEN
		back = (0,0,0)
		sx, sy, ex, ey = box["rect"]
		draw_cat = 0
		if cat is not None and cat != "":
			draw_cat = 1
		if draw_cat:
			back = COLOR.WHITE
			if "unknown" in cat:
				color = COLOR.GREY
			else:
				color = COLOR.BLUE
		st = (sx, sy)
		et = (ex, ey)
		fr = cv2.rectangle(fr, st, et, color, thickness = 3)
		if draw_life:
			cv2.putText(fr, "%d:%d" % (box['id'],box['life']),
                            (sx+4, sy+20), cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0,0,0), 2, 16)
		if draw_cat:
			#top = int( (sx + ex - 20) / 2) center
			top = sx
			cv2.putText(fr, cat, (top+4, sy-15), cv2.FONT_HERSHEY_SIMPLEX, 1.2, back, 3, 16)
			cv2.putText(fr, cat, (top+8, sy-15), cv2.FONT_HERSHEY_SIMPLEX, 1.2, back, 3, 16)
			cv2.putText(fr, cat, (top+6, sy-18), cv2.FONT_HERSHEY_SIMPLEX, 1.2, back, 3, 16)
			cv2.putText(fr, cat, (top+6, sy-12), cv2.FONT_HERSHEY_SIMPLEX, 1.2, back, 3, 16)
			cv2.putText(fr, cat, (top+6, sy-15), cv2.FONT_HERSHEY_SIMPLEX, 1.2, color, 3, 16)
	return fr

OTHER=0
FACRECO=1

def get_api(param_fn: str) -> int:
	text = open(param_fn, "r").read()
	print(text)
	if "api=facereco" in text:
		return FACRECO
	return OTHER

def run(param_fn, video_fn, log_fn, out_fn):
	api = get_api(param_fn)
	vid = Vid(video_fn)
	ols = LoadLog(log_fn)
	output = cv2.VideoWriter(out_fn, fourcc, vid.fps, (vid.w, vid.h))

	draw = draw_frame
	if api == FACRECO:
		draw = draw_fr_frame

	if not output.isOpened():
		info("Cannot open %s" % out_fn)
		return
	i = 0
	while True:
		ret, fr = vid.cap.read()
		if not ret:
			break
		fr = draw(fr, ols[i], i)
		output.write(fr)
		if i % 20 == 0: info("Drawing Process: %d/%d" % (i+1, len(ols)))
		i += 1
	info("Drawing Process: %d/%d" % (i, len(ols)))
	vid.cap.release()

def help(msg=""):
	if msg != "":
		print("\t", msg)
	print("\n\t Usage :")
	print("\t\tpython draw_eaif.py [eaif.param] [input_video] [generated_eaif_log] [output_mp4_name]\n")

def main():
	args = sys.argv
	if len(args) != 5:
		help()
		return
	param = args[1]
	video = args[2]
	log = args[3]
	out = args[4]
	assert os.path.exists(video), "Cannot Find %s " % video 
	assert os.path.exists(log), "Cannot Find %s " % log
	assert os.path.exists(param), "Cannot Find %s " % param
	run(param, video, log, out)

if __name__ == '__main__':
	TOP = 1
	BOT = 0
	main()
