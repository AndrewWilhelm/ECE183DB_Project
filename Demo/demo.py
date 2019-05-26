import numpy as np
import cv2
import cv2.aruco as aruco
import time
import serial
import sys
import math
sys.path.insert(0, '../Aruco Tags/')
import objectLocalization
sys.path.insert(0, '../Control/')
import robotControl

#Checks to see if angle 1 is within range of angles 2.
#angles are in radians and the range is a double radian
#Returns: True if angle1 within range of angle 2. False otherwise
def withinRange(angle1,angle2,range):
    if abs(angle2 - angle1) < range or abs(angle2 - angle1) > 2 * math.pi - range : #Make sure the angle doesn't flip an unreasonable amount
        return True
    return False

def main():

    ser = serial.Serial('COM4', 115200, timeout=0)
    print(ser.name)

    targetState = (20,80,0)
    robotNode = 1

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
    initialized = False
    initialxya = []
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
                    if rvec[i][0][0] > 0:
                        image = aruco.drawAxis(image,mtx,dist,rvec[i],tvec[i],0.1)
                        a = rvec[i][0][1]
                        # print(rvec[i])
                        a = objectLocalization.convertAtoRadians(a)
                # if ids[i] < 5:
                #     print("Drawing ID: " + str(ids[i]) )
                #     print("before")
                #     print(rvec[i])
                #     image = aruco.drawAxis(image,mtx,dist,rvec[i],tvec[i],0.1)
                #     print("after")


        if ids is not None:
            for i in range(len(ids)):
                if ids[i] == 0:
                    fourCorners[0] = objectLocalization.getRectMid(corners[i][0])
                elif ids[i] == 1:
                    fourCorners[1] = objectLocalization.getRectMid(corners[i][0])
                elif ids[i] == 2:
                    fourCorners[2] = objectLocalization.getRectMid(corners[i][0])
                elif ids[i] == 3:
                    fourCorners[3] = objectLocalization.getRectMid(corners[i][0])
                elif ids[i] == 5:
                    if (len(fourCorners[0]) > 0 and len(fourCorners[1]) > 0 and len(fourCorners[2]) > 0 and len(fourCorners[3]) > 0):
                        [x,y] = objectLocalization.scalePoint(fourCorners, objectLocalization.getRectMid(corners[i][0]))
                        [x,y] = objectLocalization.convertToRobotLocation(x,y)

        # print(fourCorners)
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

        
        # iter = 0
        # while (iter < 5):
        temp = ser.read(1000)
        # temp = ser.readline()
        if temp != b'':
            if temp == b'q' or temp == 'Q':
                ser.close()
                break
            # print(temp)
            # if temp[0] == '$':
            base = temp.find(b'$')
            if base > 0:
                #This means a robot is sending a message back
                code = temp[base:]
                sampleCode = "$X XXX XXX X.XXX"
                if len(code) > len(sampleCode):
                    code = code[:len(sampleCode)]
                if len(code) == len(sampleCode): #length
                    nodeNum = temp[base+1]
                    xVals = temp[base+3:base+6]
                    yVals = temp[base+7:base+10]
                # if not xVals and not yVals:
                    tempState = (int(xVals),int(yVals),0)
                    if tempState != (0,0,0):
                        targetState = tempState 
                        print("------------- NEW TARGET STATE ----------")
                        print(targetState)
                # while(True):
                #     print(temp)
                #     print("------------- NEW TARGET STATE ----------")
                #     print(targetState)

        # time.sleep(0.1)
        # print(x == '')
        # print(y == '')
        # print(a)
        # time.sleep(0.05)
        if (x != '' and y != '' and a != ''):
            # print("x: " + str(x))
            # print("y: " + str(y))
            if (len(initialxya) < 10): #looking to find theaverage of the first several measurements to get an accurate starting point
                initialxya.append((x,y,a))

            else:
                if not initialized:
                    avex = 0
                    avey = 0
                    avea = 0
                    #NOTE: Possible bug could occur if the robot is aligned along 0 degrees since this could fluctuate
                    #between 0 and 2 pi for values (resulting in an average of pi, the exact oppposite direction)
                    for i in range(10):
                        avex = avex + initialxya[i][0]
                        avey = avey + initialxya[i][1]
                        avea = avea + initialxya[i][2]
                    avex = avex/len(initialxya)
                    avey = avey/len(initialxya)
                    avea = avea/len(initialxya)
                    prevxya = (avex,avey,avea)
                    prevxya2 = (avex,avey,avea)
                    initialized = True
                    print(initialxya)
                    previouslyMissed = False
                    falsePrev = prevxya
                else:
                    # if withinRange(a,prevxya2[2], math.pi * 1/ 2) or (withinRange(a,falsePrev[2],math.pi * 1 /2) and previouslyMissed): #Make sure the angle doesn't flip an unreasonable amount
                    #     if withinRange(a,prevxya2[2], math.pi * 1/ 2) or (withinRange(a,falsePrev[2],math.pi * 1 /2) and previouslyMissed): #Make sure the angle doesn't flip an unreasonable amount
                    
                    # print("Target State: " + str(targetState))
                    print("A: " + str(a))
                    if(True):
                    # if (not withinRange(a,prevxya[2] - math.pi,math.pi*4/8)):
                    # if (withinRange(a,prevxya2[2], math.pi * 3/ 4) and withinRange(a,prevxya[2], math.pi * 2/4)) or (previouslyMissed and withinRange(a,falsePrev[2],math.pi * 1 /8)): #3/4 1/2Make sure the angle doesn't flip an unreasonable amount
                        message = robotControl.prepareControlMessage((x,y,a),targetState)
                        message = str(robotNode) + " " + str(message) + '\r\n' #preappend the robot node number
                        ser.write(str.encode(message))
                        # print(message)
                        ser.flush()
                        # if (withinRange(a,falsePrev[2],math.pi * 1 /2) and previouslyMissed):
                        #     prevxya2 = falsePrev
                        # else:
                            # prevxya2 = prevxya
                        prevxya2 = prevxya
                        prevxya = (x,y,a)
                        # print(initialxya)
                        # print((avex,avey,avea))
                        # ser.close()
                        # time.sleep(0.1)
                        # print("Just flushed")
                    else:
                        # print("MISSED IT")
                        # print(prevxya)
                        # print(prevxya2)
                        # print(x)
                        # print(y)
                        # print("A: " + str(a))
                        falsePrev = (x,y,a)
                        previouslyMissed = True



    cv2.waitKey(0)
    cv2.destroyAllWindows()
    message = str(robotNode) + " f0\r\n" #preappend the robot node number
    ser.write(str.encode(message))
    ser.close()


if __name__ == '__main__':
    main()