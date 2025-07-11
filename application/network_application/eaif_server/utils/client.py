import requests
import argparse
import time
import datetime
import numpy as np
from PIL import Image
import traceback as T
import cv2
# initialize the Keras REST API endpoint URL along with the input
# image path
KERAS_REST_API_URL = "http://localhost:5000"
IMAGE_DIR = 'dog.jpg'

def info(*args):
    print("[INFO] ", *args)

def err(*args):
    print("[ERROR] ", *args)

def getparse():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', type=str, default=IMAGE_DIR, help='input image dir')
    parser.add_argument('-u', type=str, default=KERAS_REST_API_URL, help='rest service http addr')
    parser.add_argument('-url', type=str, default='', help='rest service http addr with args')
    parser.add_argument('-o', type=str, default='classify', help='POST command: (D)classify/detection')
    parser.add_argument('-m', type=int, default=0, help='method, 0-jpg img, 1-bitstream array')
    parser.add_argument('-s', type=str, default=None, help='resize the image to wxh before send out')
    parser.add_argument('-d', action='store_true', help='flag to enable display result')

    args, _ = parser.parse_known_args()
    return args

def pretty_print_POST(req):
    """
    At this point it is completely built and ready
    to be fired; it is "prepared".

    However pay attention at the formatting used in 
    this function because it is programmed to be pretty 
    printed and may differ from the actual request.
    """
    print('{}\n{}\r\n{}\r\n\r\n{}'.format(
        '-----------START-----------',
        req.method + ' ' + req.url,
        '\r\n'.join('{}: {}'.format(k, v) for k, v in req.headers.items()),
        req.body,
    ))

# load the input image and construct the payload for the request
def POST(args):
    image_raw = open(args.i, "rb").read()
    image = np.array(Image.open(args.i).convert('RGB'))
    shape = image.shape
    meta ={"od": [{"obj": {"id":0, "rect":[0,0,shape[1],shape[0]], "vel":[0,10], "cat":"", "shaking":0}}]}
    if args.s is not None:
        dsize = tuple(map(int, args.s.split('x')))
        image = cv2.resize(image, dsize=dsize)
        for obj in meta['od']:
            obj['obj']['rect'] = [int(coord * dsize[1]/shape[0]) if i in [1, 3] else int(coord * dsize[0]/shape[1]) for i, coord in enumerate(obj['obj']['rect'])]
        shape = image.shape
        if args.m == 0:
            ret, jpg = cv2.imencode('.jpg', cv2.cvtColor(image, cv2.COLOR_RGB2BGR))
            image_raw = jpg.tobytes()

    payload = {"image": image_raw if not args.m else image,
               "format": b'jpg' if not args.m else b'raw',
               "shape":np.array(shape).astype('int32').tostring(),
               "time": str(datetime.datetime.now().microsecond),
               "meta": bytes(str(meta).replace("\'", "\""), encoding='utf8')}
    if args.url == '':
        url = "{}/{}".format(args.u, 'predict/' + args.o)
    else:
        url = args.url
    # submit the request
    ti = time.time()
    try:
        res = requests.post(url, files=payload)
        try:
            r = res.json()
            print(res.headers)
            print(res.text)
            #print(r)
        except:
            print('{} from {}'.format(res, url))
            import traceback
            print(traceback.format_exc())
            return 
        toc = time.time() - ti
        # ensure the request was successful
        if r["success"]:
            info("Success for ID ", r["time"])
            # loop over the predictions and display them
            if args.o == 'classify':
                for (i, result) in enumerate(r["predictions"]):
                    print(i, result)
            else:
                for (i, result) in enumerate(r["predictions"]):
                    print(i, result)
                    if args.d:
                        if 'box' in result:
                            draw_box(image, result["label"], result["box"])
                        else:
                            draw_box(image, result["label"][0], result['rect'], ratio = 0)
                if args.d:
                    imShow(image)
            info("TIME: %s Total time needed to response: %.4f sec" % (datetime.datetime.now(), toc))
        # otherwise, the request failed
        else:
            err("Request failed")

    except requests.exceptions.ConnectionError:
        err("Cannot Post to target IP Address, Please check the connection")

def draw_box(img, rname, box, ratio=1):
    size = img.shape
    #x, y, w, h = int(box[0]), int(box[1]), int(box[2]), int(box[3])
    #sx, sy, ex, ey = x - (w>>1), y - (h>>1), x + (w>>1), y + (h>>1)
    if ratio:
        sx, sy = int(box[1]*size[1]), int(box[0]*size[0])
        ex, ey = int(box[3]*size[1]), int(box[2]*size[0])
    else:
        sx, sy, ex, ey = box
    cv2.rectangle(img, (sx, sy), (ex - 2 - 1, ey - 2 - 1), (0, 255, 0), 2)
    cv2.putText(img, rname, (sx, ey), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255,0), 2, cv2.LINE_AA)

def imShow(img):
    Image.fromarray(img).show()
    """cv2.namedWindow("Demo", cv2.WINDOW_NORMAL)
    img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
    cv2.imshow("Demo", img)
    k = cv2.waitKey(0)"""

def main():
    POST(getparse())

if __name__ == '__main__':
    main()
