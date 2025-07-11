import numpy as np
import sys
import cv2

sys.path.append("..")

exec(f"from {sys.argv[1]} import *")
exec(f"import {sys.argv[1]}")

img_name = sys.argv[2]

img = cv2.imread(img_name)
dimg = img.copy()
for det in detFrame["detections"]:
	coord = det["coord"]
	st = (int(coord[0]), int(coord[1]))
	et = (int(coord[2]), int(coord[3]))
	cv2.rectangle(dimg, st, et, (255, 0, 0), 2)

cv2.imwrite("detframe.jpg", dimg)

dimg = img.copy()
for det in detObjList["detections"]:
	coord = det["coord"]
	st = (int(coord[0]), int(coord[1]))
	et = (int(coord[2]), int(coord[3]))
	cv2.rectangle(dimg, st, et, (255, 0, 0), 2)

cv2.imwrite("detObjList.jpg", dimg)
