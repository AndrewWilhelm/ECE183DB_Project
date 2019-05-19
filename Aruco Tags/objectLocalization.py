import numpy as np
import cv2
import cv2.aruco as aruco
import time
import serial
import math


# ser = serial.Serial('COM4', 115200, timeout=0)
# print(ser.name)
#NOTE: Our camera is 480 height by 640 width
# So the possible x values are 0 to 640 and the max y values are 0 to 480
# The bottom right corner is (479,639)
#NOTE: This means that y values increase when moving down the image

#Function to get the midpoint of a rectangle. Returns an array [x,y]
# rectArray is an array of the rectangle coordinates
def getRectMid(rectArray):
    xSum = 0
    ySum = 0
    for point in rectArray:
        xSum = xSum + point[0]
        ySum = ySum + point[1] 
    x = xSum / 4;
    y = ySum / 4;
    return [x,y]

#given the array of four corners and a point to scale, return the scaled value of that point
#Scales to a value between 0 and 100, where (0,0) is the top corner and (99,99) is the bottom right corner
#Returns an array [x,y]
#NOTE: This assumes that the four corners are appropriately perpendicular to each other
# i.e. camera needs to be straight on to rectangle
def scalePoint(fourCorners, point):
    xScale = 100 / (fourCorners[1][0] - fourCorners[0][0])
    yScale = 100 / (fourCorners[3][1] - fourCorners[0][1])
    x = (point[0] - fourCorners[0][0]) * xScale
    y = (point[1]  - fourCorners[0][1]) * yScale
    return [x,y]

#Function used to convert the angle given by the Aruco library to a value between 0 and 2 pi radians
def convertAtoRadians(a):
    if (a < 0):
        a = (2 * math.pi) + a
    return a



#Holds the values of the four corners of the robotic environment. Goes clockwise starting with the top left corner
#The ids for the aruco tags for each of the four corners goes from 0 to 3 clockwise, starting with the top left corner
fourCorners = [[],[],[],[]]

vc = cv2.VideoCapture(0)

# np.load("sample_images.npz")
with np.load("sample_images.npz") as data:
    mtx = data['mtx']
    dist = data['dist']
# ret,frame = vc.read()
# image = frame
# time.sleep(1)
# ret,frame = vc.read()
# image = frame

aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_50)
parameters = aruco.DetectorParameters_create()
iter = 0
node = 2
x = ''
y = ''
a = ''
while (vc.isOpened()):
    ret,frame = vc.read()
    image = frame
    
    # image = cv2.imread("52814747.png")
    # image = cv2.imread("arucoTestImage.png")

    # aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_250)
    corners, ids, rejectedImgPoints = aruco.detectMarkers(
        image, aruco_dict, parameters=parameters)
    rvec, tvec, _ = aruco.estimatePoseSingleMarkers(corners, 0.1, mtx, dist)
    rotations = []
    if ids is not None:
        for i in range(len(ids)):
            if ids[i] == 5:
                image = aruco.drawAxis(image,mtx,dist,rvec[i],tvec[i],0.1)
                a = rvec[i][0][1]
                a = convertAtoRadians(a)
                # rotations.append(rvec[i][0][1])
                # print(rvec[i])

    # if corners: #check to see if it's not empty
    #   rect1 = corners[0][0]
    #   print(getRectMid(rect1))
    # print("FourCorners: " + str(fourCorners))
    # print("Corners: " + str(corners))
    if ids is not None:
        for i in range(len(ids)):
            if ids[i] == 0:
                fourCorners[0] = getRectMid(corners[i][0])
            elif ids[i] == 1:
                fourCorners[1] = getRectMid(corners[i][0])
            elif ids[i] == 2:
                fourCorners[2] = getRectMid(corners[i][0])
            elif ids[i] == 3:
                fourCorners[3] = getRectMid(corners[i][0])
            elif ids[i] == 4: #Identified an obstacle
                # print(corners)
                # print(corners[i])
                rect = []
                rect.append(corners[i])
                aruco.drawDetectedMarkers(image, rect, ids[i], (0,0,255))
                # print(corners[i][0])
                # cv2.rectangle(image, corners[i][0],(0,0,255),15)
            elif ids[i] == 5: #Identified a robot
                if (len(fourCorners[0]) > 0 and len(fourCorners[1]) > 0 and len(fourCorners[2]) > 0 and len(fourCorners[3]) > 0):
                    #print("Scaled: " + str(scalePoint(fourCorners, getRectMid(corners[i][0]))))
                    [x,y] = scalePoint(fourCorners, getRectMid(corners[i][0]))

    # print(corners)
    # print(ids)

    # print(corners, ids, rejectedImgPoints)

    #NOTE: Two lines below used to draw markers originally
    # aruco.drawDetectedMarkers(image, corners, ids)
    # aruco.drawDetectedMarkers(image, rejectedImgPoints, borderColor=(100, 0, 240))

    cv2.imshow('Video', image)
    
    key = cv2.waitKey(20)

    if key == 27:
        # exit on ESC
        break

    # temp = ser.read(1000)
    # if temp != b'':
    #     if temp == b'q' or temp == 'Q':
    #         ser.close()
    #         break
    #     print(temp)
    # time.sleep(0.1)
    # if iter % 10 == 0:
    #     sx = str(x)
    #     sy = str(y)
    #     sa = str(a)
    sx = str(x)
    sy = str(y)
    sa = str(a)
    #   message = str(node) + str(' x:') + sx[0:4] + ' y:' + sy[0:4] + ' a:' + sa[0:4]
    message = str(' x:') + sx[0:4] + ' y:' + sy[0:4] + ' a:' + sa[0:4]
    print(message)
    # print(fourCorners)
    #     ser.write(str.encode(message))
    #     ser.flush()
    #     # print("Just told the transmitter to send")
    #     # print(b'2-x:1y:1a:1')
    # iter = iter + 1

cv2.waitKey(0)
cv2.destroyAllWindows()

#NOTE: Scaling in X-direction is pretty inaccurate. This needs to be scaled appropriately with the y value
# Since the values of x of the bottom corners are wider than the x values at the top of the board.

