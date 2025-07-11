from PIL import Image
import argparse
import numpy as np
import os
import glob
import cv2

WIN = "T"
def set_cursor_event(w, h):
	def cursor(event, x, y, flags, param):
		_x, _y, _w, _h = cv2.getWindowImageRect(WIN)
		__x, __y = x*1.0 / _w * w, y*1.0 / _h * h
		print("cursor position", __x, __y)
	return cursor


def readyuv(img, h, w):
	if not os.path.exists(img):
		print("[INFO] cannot find ", img)
		return
	fp = open(img, "rb")
	data = fp.read()
	fp.close()
	data = data[:h*w]
	image = np.frombuffer(data,dtype=np.uint8).reshape(h, w)
	cv2.imshow("T", np.dstack([image,image,image]))

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument("-i", nargs='+', help="input image path")
	parser.add_argument("-d", type=int, nargs='+', help="input image dimension hxw")
	args = parser.parse_args()
	img = args.i
	h, w = args.d
	idx = 0
	Len = len(img)
	cv2.namedWindow(WIN, cv2.WINDOW_NORMAL)
	#cv2.setMouseCallback(WIN, set_cursor_event(w, h))
	while True:
		i = img[idx]
		print(idx, i)
		readyuv(i, h, w)
		k = cv2.waitKey(0)
		if k == ord('k'):
			idx = (idx + 1) % Len
		elif k == ord('l'):
			idx = (idx - 1) % Len
		elif k == ord('q'):
			break

