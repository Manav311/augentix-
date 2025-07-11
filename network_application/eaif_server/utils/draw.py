import argparse
import cv2
import numpy as np
from PIL import Image

def get_args():
	parser = argparse.ArgumentParser()
	parser.add_argument("image", type=str, help="input image")
	parser.add_argument("-d", type=int, nargs='+', help="input dim [h, w]")
	parser.add_argument("-b", type=int, nargs='+', help="input box w.r.t input w and h [s0x s0y e0x e0y s1x...]")
	parser.add_argument("-w", action="store_true", help="input box as [s0x s0y 0w h0 s1x...]")
	parser.add_argument("-i", type=str, default=None, help="widerface format file")
	return parser.parse_args()

def main(args):
	img = cv2.imread(args.image)[:,:,::-1].astype(np.uint8)
	ih, iw, _ = img.shape
	dw = iw
	dh = ih
	if args.b is None:
		dw = args.d[1]
		dh = args.d[0]
	print("img", args.image, " h,w ",img.shape)
	if args.i:
		lines = open(args.i).read().split("\n")
		for i in lines[2:-1]:
			line = i.split(" ")
			rect = [float(l) for l in line]
			b = [rect[0], rect[1], rect[0] + rect[2], rect[1] + rect[3]]
			b = [int(k) for k in b]
			print("img", img.shape, b)
			cv2.rectangle(img, (b[0],b[1]), (b[2],b[3]), (0,255,0), 1)
		Image.fromarray(img).show()
	else:
		box_dim = len(args.b)
		box_num = box_dim / 4
		assert box_dim % 4 == 0, "incorrect input number of box dimension, should be 4X"
		boxes = np.array(args.b).reshape(-1,4)
		boxes[:,::2] = boxes[:,::2] * iw * 1.0 / dw
		boxes[:,1::2] = boxes[:,1::2] * ih * 1.0 / dh

		for i in range(int(box_num)):
			b = boxes[i,:]
			print("img", img.shape, b)
			st = (b[0],b[1])
			if args.w:
				et = (b[0] + b[2] - 1, b[1] + b[3]-1)
			else:
				et = (b[2],b[3])
			cv2.rectangle(img, st, et, (0,255,0), 1)


		Image.fromarray(img).show()
		return

main(get_args())



