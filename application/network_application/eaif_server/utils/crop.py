import cv2
import sys

def imcrop(fi, fo, box):
	img = cv2.imread(fi)
	h, w, c = img.shape
	sx, sy, ex, ey = box
	ex = min(ex, w)
	ey = min(ey, h)
	oimg = img[sy:ey,sx:ex,:]
	cv2.imwrite(fo, oimg)

fi = sys.argv[1]
fo = sys.argv[2]
box = sys.argv[3:7]
box = [int(b) for b in box]
imcrop(fi, fo, box)
