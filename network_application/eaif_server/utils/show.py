import numpy as np
from PIL import Image
import sys
import argparse

def get_args():
	parser = argparse.ArgumentParser()
	parser.add_argument("-i", help="input file")
	parser.add_argument("-d", type=int, default=0, help="dtype[0-uint8, X-float32]")
	parser.add_argument("-z", type=float, nargs="+", default=[0.,0.,0.], help="zeros")
	parser.add_argument("-s", type=float, nargs="+", default=[1.,1.,1.], help="scales")
	return parser.parse_args()

def run(args):
	fimg = args.i
	dty = np.uint8 if args.d == 0 else np.float32
	zeros = args.z
	scales = args.s

	img = open(fimg, "rb").read()
	meta = np.frombuffer(img[:16], dtype=np.int32)
	dtype, w, h, c = meta[0], meta[1], meta[2], meta[3]

	print("Dtype:%d %dx%dx%d" % (dtype, h, w, c))

	if c == 1:
		data = np.frombuffer(img[16:], dtype=dty).reshape(h,w)
	else:
		print("LEN:", len(img[16:]))
		data = np.frombuffer(img[16:], dtype=dty).reshape(h,w,c)
	
	data = (data * scales + zeros).astype(np.uint8)
	ii = Image.fromarray(data)
	target_img = "save-" + fimg[:-3] + "jpg"
	ii.save(target_img)
	ii.show()

def main():
	args = get_args()
	print(f"args")
	run(args)

if __name__ == "__main__":
	main()

