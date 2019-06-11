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
sys.path.insert(0, '../Mapping/')
import mapping
import matplotlib.pyplot as plt
import multiprocessing

#Checks to see if angle 1 is within range of angles 2.
#angles are in radians and the range is a double radian
#Returns: True if angle1 within range of angle 2. False otherwise
def withinRange(angle1,angle2,range):
    if abs(angle2 - angle1) < range or abs(angle2 - angle1) > 2 * math.pi - range : #Make sure the angle doesn't flip an unreasonable amount
        return True
    return False

#Return RFID info if it's a valid message
#Return None otherwise
#RFID info is an array [(xloc,yloc),[[xloc, yloc isLeaf] ... x4 ]]
def parseRFIDInfo(message):
    RFIDinfo = []
    base = message.find(b'$')
    if base >= 0:
        #This means a robot is sending a message back
        print("========================")
        print(message[base:])
        print("========================")
        code = message[base:]
        sampleCode = "$X XXX YYY [XXX YYY L] [XXX YYY L] [XXX YYY L] [XXX YYY L]"
        shortestSample = "$X XXX YYY"
        oneArray = "$X XXX YYY [XXX YYY L]"
        twoArray = "$X XXX YYY [XXX YYY L] [XXX YYY L]"
        threeArray = "$X XXX YYY [XXX YYY L] [XXX YYY L] [XXX YYY L]"
        RFIDadj = []
        if len(code) > len(sampleCode):
            code = code[:len(sampleCode)]
        if len(code) >= len(shortestSample):
            nodeNum = message[base+1]
            xVals = message[base+3:base+6]
            yVals = message[base+7:base+10]
            xint = int(xVals)
            yint = int(yVals)
            RFIDinfo.append((xint,yint))
            if len(code) >= len(oneArray) and chr(message[base+21]) == ']':
                x1 = message[base+12:base+15]
                y1 = message[base+16:base+19]
                b1 = message[base+20]
                if chr(b1) == '1':
                    b1 = True
                else:
                    b1 = False
                RFIDadj.append([int(x1),int(y1),b1])
            if len(code) >= len(twoArray) and chr(message[base+33]) == ']':
                x2 = message[base+24:base+27]
                y2 = message[base+28:base+31]
                b2 = message[base+32]
                if chr(b2) == '1':
                    b2 = True
                else:
                    b2 = False
                RFIDadj.append([int(x2),int(y2),b2])
            if len(code) >= len(threeArray) and chr(message[base+45]) == ']':
                x3 = message[base+36:base+39]
                y3 = message[base+40:base+43]
                b3 = message[base+44]
                if chr(b3) == '1':
                    b3 = True
                else:
                    b3 = False
                RFIDadj.append([int(x3),int(y3),b3])
            if len(code) >= len(sampleCode) and chr(message[base+57]) == ']':
                x4 = message[base+48:base+51]
                y4 = message[base+52:base+55]
                b4 = message[base+56]
                if chr(b4) == '1':
                    b4 = True
                else:
                    b4 = False
                RFIDadj.append([int(x4),int(y4),b4])

            RFIDinfo.append(RFIDadj)

    if len(RFIDinfo) > 0:
        return RFIDinfo
    return None

#Adds zeros to preappend to the number (to format for message sending)
#numStr is a a string and lengthRequired is the length of the desired string
#Returns: a string
def preappendZeros(numStr, lengthRequired):
    while len(numStr) < lengthRequired:
        numStr = "0" + numStr
    return numStr

def prepRFIDInfo(tagLocation,RFIDInfo,graph):
    tagLocX = preappendZeros(str(tagLocation[0]),3)
    tagLocY = preappendZeros(str(tagLocation[1]),3)
    message = "$1w " + tagLocX + " " + tagLocY
    # print("RFIDInfo: ")
    # print(str(RFIDInfo))
    RFIDadj = RFIDInfo[1]
    for adj in RFIDadj:
        message = message + " ["
        adjLocX = preappendZeros(str(adj[0]),3)
        adjLocY = preappendZeros(str(adj[1]),3)
        adjBool = '0'
        # if adj[2] == True:
        # print("#############")
        # print(adj[0])
        # print(adj[1])
        if mapping.determineIsLeaf((adj[0],adj[1]),graph) == True:
            adjBool = '1'
        message = message + adjLocX + " " + adjLocY + " " + adjBool + "]"
    print("MESSAGE")
    print(message)
    return message

def plotMap(localMap, robotPosition):
    # fig = plt.gcf()
    # ax = plt.gca()
    # print("PLOTTING MAP")
    # mapping.plotGraph(localMap,[robotPosition], fig, ax)
    # plt.pause(0.001)
    # if figure == None:
    #     figure = plt.gcf()
    # if axis == None:
    #     axis = plt.gca()
    edges = localMap
    robotLocations = [robotPosition]
    plt.clf()
    plt.xlim(0,100)
    plt.ylim(0,100)
    plt.xticks(np.arange(0, 100+1, 10.0))
    plt.yticks(np.arange(0, 100+1, 10.0))
    plt.grid()
    plt.gca().invert_yaxis()
    plt.gca().xaxis.tick_top()
    # axis.invert_yaxis()
    # axis.xaxis.tick_top()

    for index in range(len(edges)):
        edge = edges[index]
        if edge[2] == False:
            #The node has yet to be explored
            mapping.plotEdge(edge[0],edge[1],edge[2])
    #Now graph the explored edges in red
    for index in range(len(edges)):
        edge = edges[index]
        if edge[2] == True:
            #The node has been explored
            mapping.plotEdge(edge[0],edge[1],edge[2])

    colors = ['k','DarkGray','c']
    for index in range(len(robotLocations)):
        node = robotLocations[index]
        if index > len(colors) - 1:
            color = colors[len(colors) - 1]
        else:
            color = colors[index]

        plt.plot([node[0], node[0]], [node[1], node[1]], marker = 'o', c = color)


    plt.gcf().canvas.draw_idle()
    # figure.canvas.draw_idle()
    plt.pause(0.001)


#Returns the initialized graph
def initializeGraph():
    edge1 = [(20,20), (20,40), False] 
    edge2 = [(20,40), (20,60), False]  
    edge3 = [(20,40), (50,40), False]  
    edge4 = [(20,60), (50,60), False] 
    edge5 = [(20,80), (50,80), False] 
    edge6 = [(50,20), (50,40), False] 
    edge7 = [(50,40), (50,60), False] 
    edge8 = [(50,60), (50,80), False] 
    edge9 = [(50,20), (80,20), False] 
    edge10 = [(50,40), (80,40), False] 
    edge11 = [(50,60), (80,60), False] 
    edge12 = [(50,80), (80,80), False]
    graph = [edge1, edge2, edge3, edge4, edge5, edge6, edge7, edge8, edge9, edge10, edge11, edge12]  

    plt.ion()
    # fig, ax = plt.subplots()
    x, y = [],[]
    line = plt.plot(x,y)[0]
    return graph

def main():

    ser = serial.Serial('COM3', 115200, timeout=0)
    print(ser.name)

    # targetState = (20,80,0)
    robotNode = 2
    robotTargetLoc1 = (80,20)
    robotMap = []
    # robotLocation2 = (60,70)
    globalMap = initializeGraph()
    mapping.plotGraph(robotMap,[robotTargetLoc1])
    performRobotWriting = False

    #Holds the values of the four corners of the robotic environment. Goes clockwise starting with the top left corner
    #The ids for the aruco tags for each of the four corners goes from 0 to 3 clockwise, starting with the top left corner
    fourCorners = [[],[],[],[]]

    vc = cv2.VideoCapture(1)

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
    node = 1
    x = ''
    y = ''
    a = ''
    initialized = False
    initialxya = []
    iter = 0
    while (vc.isOpened()):
        # if iter % 90 == 0:
        #     plt.pause(0.001)
        iter = (iter + 1) % 25

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
                        # rvec[i][0][0] = -1 * rvec[i][0][0]
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

        print(fourCorners)
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
            if len(temp) > 0:
                RFIDinfo = parseRFIDInfo(temp)
                print("RFID Info: ")
                print(RFIDinfo)
                # print("RFIDInfo: " + str(RFIDinfo))    
                if RFIDinfo != None:
                    if mapping.euclideanDistance((x,y),(RFIDinfo[0][0],RFIDinfo[0][1])) < 10 and mapping.euclideanDistance((x,y),robotTargetLoc1) < 10: #Make sure we're in the range of the tag we think we are
                        (newTargetLoc, info,preprocessedMap) = mapping.updateMapandDetermineNextStep(robotMap,globalMap,robotTargetLoc1,RFIDinfo)
                        print("------------------" + str(newTargetLoc) + "---------------------")
                        if performRobotWriting:
                            message = prepRFIDInfo(robotTargetLoc1,info,preprocessedMap)
                            message = message +"\r\n"
                            ser.write(str.encode(message))
                            # print(message)
                            ser.flush()

                        mapping.plotGraph(preprocessedMap,[robotTargetLoc1])
                        robotTargetLoc1 = newTargetLoc
                        # mapping.plotGraph(robotMap,[robotTargetLoc1])


                # if len(code) >= len(shortestSample):
                # # if not xVals and not yVals:
                #     tempState = (int(xVals),int(yVals),0)
                #     if tempState != (0,0,0):
                #         targetState = tempState 
                #         print("------------- NEW TARGET STATE ----------")
                #         print(targetState)
        first = True
        job = multiprocessing.Process(target=plotMap,args=(robotMap,(x,y)))
        

        # time.sleep(0.1)
        # print(x == '')
        # print(y == '')
        # print(a)
        # time.sleep(0.05)
        if (x != '' and y != '' and a != ''):
            # print("x: " + str(x))
            # print("y: " + str(y))
            #     
            #     plt.pause(0.001)
            # job.start()
            # job.join()
            # if iter % 45 == 0:
                # job.start()
                # mapping.plotGraph(robotMap,[(x,y)])
                # plotMap(robotMap,(x,y))
            # if first:
            #     job.start()

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
                    # print(initialxya)
                    previouslyMissed = False
                    falsePrev = prevxya
                else:
                    # if withinRange(a,prevxya2[2], math.pi * 1/ 2) or (withinRange(a,falsePrev[2],math.pi * 1 /2) and previouslyMissed): #Make sure the angle doesn't flip an unreasonable amount
                    #     if withinRange(a,prevxya2[2], math.pi * 1/ 2) or (withinRange(a,falsePrev[2],math.pi * 1 /2) and previouslyMissed): #Make sure the angle doesn't flip an unreasonable amount
                    
                    # print("Target State: " + str(targetState))
                    # print("A: " + str(a))
                    if(True):
                    # if (not withinRange(a,prevxya[2] - math.pi,math.pi*4/8)):
                    # if (withinRange(a,prevxya2[2], math.pi * 3/ 4) and withinRange(a,prevxya[2], math.pi * 2/4)) or (previouslyMissed and withinRange(a,falsePrev[2],math.pi * 1 /8)): #3/4 1/2Make sure the angle doesn't flip an unreasonable amount
                        message = robotControl.prepareControlMessage((x,y,a),robotTargetLoc1)
                        message = "$" + str(robotNode) + str(message) + '\r\n' #preappend the robot node number
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
    message = "$" + str(robotNode) + "f 0\r\n" #preappend the robot node number
    ser.write(str.encode(message))
    ser.close()


if __name__ == '__main__':
    main()