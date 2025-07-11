import cv2
import argparse
import sys

def detect(img_path, rate, th=0):
    hog = cv2.HOGDescriptor() 
    hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector()) 
       
    # Reading the Image 
    image = cv2.imread(img_path) 
    
    # Resizing the Image
    print(image.shape) 
    # Detecting all the regions in the  
    # Image that has a pedestrians inside it 
    # Drawing the regions in the Image 
    # Showing the output Image 
    cv2.namedWindow("Image", cv2.WINDOW_NORMAL)
    stride = 2
    while True:
        img = image.copy()
        h = int(image.shape[0] * rate)
        w = int(image.shape[1] * rate)
        wimg = cv2.resize(img, (w,h), interpolation=cv2.INTER_LINEAR)
        wimg = cv2.cvtColor(wimg, cv2.COLOR_BGR2GRAY)
        (regions, a) = hog.detectMultiScale(wimg, th,
                                        winStride=(stride, 16), 
                                        padding=(8, 8),
                                        scale=1.05)#,
                                        #finalThreshold=4) 
        print(wimg.shape, th, rate, stride)
        i = 0
        for region in regions:
            x, y, w, h = [int(i/rate) for i in region]
            print(i, x, y, x+w, y+h, a[i]); i+= 1
            cv2.rectangle(img, (x, y),  
                          (x + w, y + h),  
                      (0, 0, 255), 2) 

        cv2.imshow("Image", img) 
        k = cv2.waitKey(0) & 0xff
        if k == ord('q'):
            break
        elif k == ord('t'):
            th = (cv2.waitKey(0) - ord('0'))
            th += (cv2.waitKey(0) - ord('0')) * 0.1
            th += (cv2.waitKey(0) - ord('0')) * 0.01
        elif k == ord('r'):
            rate = 1.0
            rate *= (cv2.waitKey(0) - ord('0'))
            rate += (cv2.waitKey(0)-ord('0'))*0.1
        elif k == ord('w'):
            stride = (cv2.waitKey(0) - ord('0')) * 10
            stride += (cv2.waitKey(0) - ord('0'))
    cv2.destroyAllWindows()

def detectVideo(data_path, rate, th=0):
    hog = cv2.HOGDescriptor() 
    hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector()) 
       
    # Reading the Image
    cap = cv2.VideoCapture(data_path) 
    
    # Resizing the Image
    ret, image = cap.read()
    print(image.shape)
    cv2.namedWindow("Image", cv2.WINDOW_NORMAL)
    while True:
        ret, image = cap.read()
    
        h = int(image.shape[0] * rate)
        w = int(image.shape[1] * rate)
        image = cv2.resize(image, (w,h), interpolation=cv2.INTER_LINEAR)
           
        # Detecting all the regions in the  
        # Image that has a pedestrians inside it 
        (regions, _) = hog.detectMultiScale(image, th,
                                            winStride=(2, 8), 
                                            padding=(32, 32),
                                            scale=1.05)#,
                                            #finalThreshold=th) 
        # Drawing the regions in the Image 
        for (x, y, w, h) in regions: 
            cv2.rectangle(image, (x, y),  
                          (x + w, y + h),  
                          (0, 0, 255), 2) 
          
        # Showing the output Image 
        cv2.imshow("Image", image) 
        k = cv2.waitKey(30)
        if k == ord('q'):
            break
        elif k == ord('t'):
            th = 10
            th *= (cv2.waitKey(0) - ord('0'))
            th += cv2.waitKey(0)
           
    cap.release()
    cv2.destroyAllWindows()

def main():
    args = sys.argv
    detect(args[1], float(args[2]), float(args[3]))
    #detectVideo(args[1], float(args[2]), float(args[3]))

main()
