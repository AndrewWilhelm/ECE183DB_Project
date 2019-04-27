from webcam import Webcam
import cv2
from datetime import datetime

import os
cwd = os.getcwd()
print(cwd)
  
webcam = Webcam()
webcam.start()
 
while True: 
     
    # get image from webcam
    image = webcam.get_current_frame()
 
    # display image
    cv2.imshow('grid', image)
    key = cv2.waitKey(1500)
 
    # save image to file, if pattern found
    ret, corners = cv2.findChessboardCorners(cv2.cvtColor(image,cv2.COLOR_BGR2GRAY), (9,6), None)
 
    if ret == True:
        filename = datetime.now().strftime('%Y%m%d_%Hh%Mm%Ss%f') + '.jpg'
        cv2.imwrite("sample_images/" + filename, image)
        print("Found corners")

    else:
        print("FALSE")


    if key == 27:
        # exit on ESC
        break

cv2.waitKey(0)
cv2.destroyAllWindows()