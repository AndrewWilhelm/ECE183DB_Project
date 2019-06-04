import math
import time
import sys

# sys.path.append('../Aruco Tags/')
sys.path.insert(0, '../Aruco Tags/')
import objectLocalization

#Returns "right" or "left" if necessary to turn right or left.
#Otherwise returns an empty string
def angleCheck(currentAngle, targetAngle):
	threshold = math.pi * 5 / 32
	if abs(currentAngle - targetAngle) > threshold and abs(currentAngle - targetAngle) < 2 * math.pi - threshold:
		#Need to adjust current angle - not on the edge of 0 and 2 pi
		diff = currentAngle - targetAngle
		turnOpposite = False
		if diff > math.pi:
			diff = diff - 2 * math.pi
		elif diff < -1 * math.pi:
			diff = diff + 2 * math.pi
		if diff < 0:
			return("r")
		elif diff > 0:
			return("l")
	return("")

#Verify that the targetPoint is not within the xy margin from the currentpoint
def withinXYMargin(currentPoint,targetPoint):
	margin = 1
	# print(currentPoint)
	# print(targetPoint)
	distance = math.sqrt(math.pow(currentPoint[1] - targetPoint[1],2) + math.pow(currentPoint[0] - targetPoint[0],2))
	if  distance < margin:
		return True
	return False


#Verifies to see if the current point is within a reasonable margin of the target location
#Input: currentPoint and targetPoint are tuples with (x,y) positions
#Return: Foward instruction and speed proportional to distance from point if current distance is greater than a given margin
#		Otherwise return an empty string
def xyCheck(currentPoint,targetPoint):
	motorMaxValue = 1023 #max pwm value to motor
	motorMinValue = 700 #min pwm value to motor to still have vehicle movement
	maxDist = 100

	if withinXYMargin(currentPoint,targetPoint) == True:
		return ""
	else:
		distance = math.sqrt(math.pow(currentPoint[1] - targetPoint[1],2) + math.pow(currentPoint[0] - targetPoint[0],2))
		if (distance > maxDist):
			distance = maxDist 
		throttle = ((distance)/maxDist) * (motorMaxValue - motorMinValue) + motorMinValue
		return("f "+str(int(throttle)))

#Prepares the control message for transmission given the current state and target state of the robot
#Input: currentState and targetState, both of which are tuples of the form (x,y,angle)
#Output: The control message (as a string). Will return an empty string if the target and current state are the same
def prepareControlMessage(currentState,targetState):
	debug = False

	if withinXYMargin(currentState,targetState) == True:
		#This is the unlikely case where the robot is already at the x y point that it needs to be at. Therefore, it doesn't move
		return "f 0"

	if targetState[0] == currentState[0]:
		if targetState[1] > currentState[1]: #want to move up; angle is zero
			turningAngle = math.pi
		elif targetState[1] < currentState[1]: #want to move down; angle is pi
			turningAngle = 0
	else:
		angleTan = math.atan((targetState[1] - currentState[1]) / (targetState[0] - currentState[0]))
		if ((targetState[0] - currentState[0]) > 0):
			turningAngle = ((math.pi / 2) + angleTan)
		else:
			turningAngle = ((3 * math.pi / 2) + angleTan) 

	# print(turningAngle)
	message = angleCheck(currentState[2],turningAngle)
	if message == "":
		message = xyCheck((currentState[0],currentState[1]),(targetState[0],targetState[1]))

	if debug:
		print("Current State: " + str(currentState))
		print( "Target State: " + str(targetState))
		print ("Message: " + str(message))

	return message

def main():
	# ser = serial.Serial('COM4', 115200, timeout=0)
	# print(ser.name)
	# #Transmit r,l, or f#, where the # after f specifies the throttle value
	# # I should create a file called "System Integration" that uses both objectLocalization and robotControl
	# #to get a robots current location and transmit to the robot the necessary actions

 #    iter = 0
	# node = 2

	# prepareControlMessage test cases
	# print(prepareControlMessage((0,0,0),(10,20,0)))
	# print(prepareControlMessage((0,0,0),(0,0,0)))
	# print(prepareControlMessage((0,0,1*math.pi/2),(1,1,0)))
	# print(prepareControlMessage((20,0,0),(0,0,0)))
	# print(prepareControlMessage((50,50,0),(0,0,0)))
	# print(prepareControlMessage((50,50,0),(100,0,0)))
	# print(prepareControlMessage((50,50,0),(100,100,0)))
	# print(prepareControlMessage((50,50,0),(0,100,0)))
	print(prepareControlMessage((50,50,0),(50,0,0)))
	print(prepareControlMessage((50,50,0),(100,50,0)))
	print(prepareControlMessage((50,50,0),(50,100,0)))
	print(prepareControlMessage((50,50,0),(0,50,0)))



	# # angleCheck test cases
	# print(angleCheck(math.pi,math.pi+0.5) == "r")
	# print(angleCheck(math.pi,math.pi-0.5) == "l")
	# print(angleCheck(2*math.pi - 0.1,0.5) == "r")
	# print(angleCheck(0.5,2*math.pi - 0.1) == "l")
	# print(angleCheck(-0.01,0.01) == "")
	# print(angleCheck(0.01,-0.01) == "")
	# print(angleCheck(math.pi,math.pi+0.01) == "")
	# print(angleCheck(math.pi,math.pi-0.01) == "")

	# #xyCheck test cases
	# print(xyCheck((1,1),(0,0)) == "")
	# print(xyCheck((1,0),(6.5,0)))
	# print(xyCheck((6,1),(6,10)))
	# print(xyCheck((90,80),(90,80)) == "")
	# print(xyCheck((90,80),(0,0)))

if __name__ == '__main__':
	main()