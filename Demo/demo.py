import numpy as np
import cv2
import cv2.aruco as aruco
import time
import serial
sys.path.insert(0, '../Aruco Tags/')
import objectLocalization
sys.path.insert(0, '../Control/')
import robotControl


def main():

    ser = serial.Serial('COM4', 115200, timeout=0)
    print(ser.name)

    targetState = (50,50,0)

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
                    a = objectLocalization.convertAtoRadians(a)


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

        temp = ser.read(1000)
        if temp != b'':
            if temp == b'q' or temp == 'Q':
                ser.close()
                break
            print(temp)
        # time.sleep(0.1)
        message = robotControl.prepareControlMessage((x,y,a),targetState)
        ser.write(str.encode(message))
        ser.flush()
        # if iter % 10 == 0:
            # sx = str(x)
            # sy = str(y)
            # sa = str(a)
            # message = str(node) + str(' x:') + sx[0:7] + ' y:' + sy[0:7] + ' a:' + sa[0:7]
            # ser.write(str.encode(message))
            # ser.flush()
            # print("Just told the transmitter to send")
            # print(b'2-x:1y:1a:1')
        # iter = iter + 1

    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()