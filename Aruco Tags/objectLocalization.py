import numpy as np
import cv2
import cv2.aruco as aruco
import time
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
			image = aruco.drawAxis(image,mtx,dist,rvec[i],tvec[i],0.1)
			rotations.append(rvec[i][0][1])
			print(rvec[i])

	# if corners: #check to see if it's not empty
	# 	rect1 = corners[0][0]
	# 	print(getRectMid(rect1))
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
			elif ids[i] == 5:
				if (len(fourCorners[0]) > 0 and len(fourCorners[1]) > 0 and len(fourCorners[2]) > 0 and len(fourCorners[3]) > 0):
					print("Scaled: " + str(scalePoint(fourCorners, getRectMid(corners[i][0]))))

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

cv2.waitKey(0)
cv2.destroyAllWindows()