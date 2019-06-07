import matplotlib.pyplot as plt 
import numpy as np
import time
import copy
import math

#Plots the corresponding edge that corresponds to node1 and node 2 into the plot
#If the edge has yet to be explored, it will be colored blue.
#An explored edge is colored red
def plotEdge(node1,node2,isExplored):
	color = 'b'
	if isExplored:
		color = 'r'
	plt.plot([node1[0], node2[0]], [node1[1], node2[1]], marker = 'o', c = color)

	# hl.set_xdata(numpy.append(hl.get_xdata(), new_data))
 #    hl.set_ydata(numpy.append(hl.get_ydata(), new_data))
 #    plt.draw()

#Plots a graph. Edges is a list of edges
#Will plot unexplored edges first (to reduce confusion as to whether a node has been explored)
#Each edge should be a list of two elements: [((node1), (node2)), isExplored]
#Where isExplored is a boolean indicating if that edge has been explored
#robotLocations is a list of robotLocations. A colored dot will be drawn at these nodes
def plotGraph(edges,robotLocations=[]):
	# if figure == None:
	# 	figure = plt.gcf()
	# if axis == None:
	# 	axis = plt.gca()

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
		# print("edges:")
		# print(edges)
		# print("edge:")
		# print(edge)
		if edge[2] == False:
			#The node has yet to be explored
			plotEdge(edge[0],edge[1],edge[2])
	#Now graph the explored edges in red
	for index in range(len(edges)):
		edge = edges[index]
		if edge[2] == True:
			#The node has been explored
			plotEdge(edge[0],edge[1],edge[2])

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

def manhattanDistance(startingNode,targetNode):
	return (abs(startingNode[0] - targetNode[0]) + abs(startingNode[1] - targetNode[1]))

def constructPath(cameFrom,current):
	total_path = [current]
	while current in cameFrom:
		current = cameFrom[current]
		total_path.append(current)
	total_path.reverse()
	return total_path

#Finds the shortest valid path between two edges using A*
#Returns: a set of edges from startingNode to targetNode
#			returns None if startingNode and/or targetNode 
#			are not contained within the set of edges or if
#			there is no valid path
def findValidPath(startingNode,targetNode,edges):
	closedSet = []
	openSet = [startingNode]
	cameFrom = {} #Create a dictionary to be used as a map
	gscore = {} #Default value is infinity
	gscore[startingNode] = 0
	fscore = {} #Default value is infinity
	fscore[startingNode] = manhattanDistance(startingNode,targetNode)

	while len(openSet) > 0:
		minScore = 1000
		current = openSet[0]
		for index in range(len(openSet)):
			c = openSet[index]
			if c not in fscore:
				fscore[c] = 999
			if fscore[c] < minScore:
					current = c
					minScore = fscore[c]

		if current == targetNode:
			return constructPath(cameFrom,current)

		# openSet.pop(current)
		openSet.remove(current)
		closedSet.append(current)

		neighbors = []
		for index in range(len(edges)):
			if edges[index][0] == current:
				neighbors.append(edges[index][1])
			elif edges[index][1] == current:
				neighbors.append(edges[index][0])

		for index in range(len(neighbors)):
			neighbor = neighbors[index]
			if neighbor in closedSet:
				continue
			if current not in gscore:
				gscore[current] = 999

			tentative_gscore = gscore[current] + manhattanDistance(current,neighbor)

			if neighbor not in openSet:
				openSet.append(neighbor)
			elif tentative_gscore >= gscore[neighbor]:
				continue

			cameFrom[neighbor] = current
			gscore[neighbor] = tentative_gscore
			fscore[neighbor] = gscore[neighbor] + manhattanDistance(neighbor,targetNode)

def euclideanDistance(node1,node2):
	return math.sqrt(math.pow(abs(node1[0]-node2[0]),2) + math.pow(abs(node1[1]-node2[1]),2))

#Returns the closest node to an arbitrary point (point need not be in the graph)
#The node will have an x and y that are a multiple of 10
#Returns: a tuple (x,y) of the closest node in the graph
def getClosestNode(point,edges):
	nodeList = []
	for index in range(len(edges)):
		if edges[index][0] not in nodeList:
				nodeList.append(edges[index][0])
		elif edges[index][1] not in nodeList:
				nodeList.append(edges[index][1])

	minDistance = 1000
	closestNode = None
	for index in range(len(nodeList)):
		dist = euclideanDistance(point,nodeList[index])
		if  dist < minDistance:
			minDistance = dist
			closestNode = nodeList[index] 
	return closestNode

def isEdgeinGraph(edge,graph):
	v1 = (edge[0],edge[1])
	v2 = (edge[1],edge[0])

	#Some graphs also have an additional boolean variable in them for isExplored
	v3 = (edge[0],edge[1],True)
	v4 = (edge[0],edge[1],False)
	v5 = (edge[1],edge[0],True)
	v6 = (edge[1],edge[0],False)
	#Some graphs use arrays instead of tuples
	v7 = [edge[0],edge[1],True]
	v8 = [edge[0],edge[1],False]
	v9 = [edge[1],edge[0],True]
	v10 = [edge[1],edge[0],False]
	if v1 in graph or v2 in graph or v3 in graph or v4 in graph or v5 in graph or v6 in graph:
		return True
	if v7 in graph or v8 in graph or v9 in graph or v10 in graph:
		return True
	return False


#Returns the next node that the robot at robotLocation should visit
#given the graph provided by the robot's local map edges (which need to include isExplored boolean values)
#Uses the robot's local map to make a decision for where to move next
#Returns: a tuple (x,y) of the next node. None if there is no logical move available
#Robots are specified a target location (x,y) and an edge that they are travelling on
#Two robots cannot be on the same edge or travelling towards the same target at any time
def getNextMove(localMap,robotLocation, allRobotTargetLocations=[],robotEdges=[]):
	minDist = 1000
	minPath = []
	for index in range(len(localMap)):
		edge = localMap[index]
		# edgeV1 = (edge[0],edge[1])
		# edgeV2 = (edge[1],edge[0])
		# # print("Comparison:")
		# # print(edgeV1)
		# # print(robotEdges)
		# if edgeV1 in robotEdges or edgeV2 in robotEdges:
		# 	continue
		if isEdgeinGraph(edge,robotEdges):#another robot is on that edge or about to move to that edge -> it's in the process of being explored already
			continue

		if edge[0] == robotLocation and edge[2] == False:
			print("gets here")
			if edge[1] in allRobotTargetLocations:#Another robot just beat it to that node
				continue
			# edge[2] = True
			localMap[index] = (edge[0],edge[1],True)
			return edge[1]
			# print("here")
			# print(robotLocation)
			# print(allRobotTargetLocations)
		elif edge[1] == robotLocation and edge[2] == False:
			if edge[0] in allRobotTargetLocations:#Another robot just beat it to that node
				continue
			# edge[2] = True
			localMap[index] = (edge[0],edge[1],True)
			return edge[0]

		if edge[2] == False:
			pathA = findValidPath(robotLocation,edge[0],localMap)
			pathB = findValidPath(robotLocation,edge[1],localMap)
			if pathA != None:
				if len(pathA) < minDist:
					# print(pathA)
					if pathA[0] == robotLocation:
						edgeToEval = (robotLocation,pathA[1])
						targetPoint = pathA[1]
					else:
						edgeToEval = (robotLocation,pathA[-2])
						targetPoint = pathA[-2]
					# edgeToEvalV2 = (edgeToEval[1],edgeToEval[0])
					#Verify no collsions between robots and the target points/edges
					if not isEdgeinGraph(edgeToEval,robotEdges) and targetPoint not in allRobotTargetLocations:
						minPath = pathA
						minDist = len(pathA)
			if pathB != None:
				if len(pathB) < minDist:
					if pathB[0] == robotLocation:
						edgeToEval = (robotLocation,pathB[1])
						targetPoint = pathB[1]
					else:
						edgeToEval = (robotLocation,pathB[-2])
						targetPoint = pathB[-2]
					# edgeToEvalV2 = (edgeToEval[1],edgeToEval[0])
					#Verify no collsions between robots and the target points/edges
					if not isEdgeinGraph(edgeToEval,robotEdges) and targetPoint not in allRobotTargetLocations:
						minPath = pathB
						minDist = len(pathB)
	if len(minPath) > 1:
		if minPath[0] == robotLocation:
			return minPath[1]
		else:
			return minPath[-2]

	if robotLocation in allRobotTargetLocations:
		#This robot needs to move since another robot with higher priority wants to move here
		for index in range(len(localMap)):
			edge = localMap[index]
			# edgeV1 = (edge[0],edge[1])
			# edgeV2 = (edge[1],edge[0])

			if not isEdgeinGraph(edge,robotEdges):
				if edge[0] == robotLocation and edge[1] not in allRobotTargetLocations:
					return edge[1]
				elif edge[1] == robotLocation and edge[0] not in allRobotTargetLocations:
					return edge[0]
	# print("Default selected")
	return robotLocation

#adds edges that are connected to robotLocation if they aren't alrady in the mapToUpdate
def updateMapUsingSensing(mapToUpdate,globalMap,robotLocation):
	for edge in globalMap:
		if edge[0] == robotLocation or edge[1] == robotLocation:
			if not isEdgeinGraph(edge,mapToUpdate):	
				mapToUpdate.append((edge[0],edge[1],False))


#upades the mapToUpdate using the RFIDInfo
#Verifies that the information use doesn't violate the global map
#(i.e. no mistakes were made in what was written to the RFID tag)
#RFID info is an array [(xloc,yloc),[[xloc, yloc isLeaf] ... x4 ]]
def updateMapUsingRFID(mapToUpdate,globalMap,RFIDInfo):
	tagLoc = RFIDInfo[0]
	adjacentTags = RFIDInfo[1]
	print("Adjacent Tags:")
	print(adjacentTags)
	for adj in adjacentTags:
		isLeaf = adj[2]
		edge = (tagLoc,(adj[0],adj[1]))
		print("ADJ:")
		print(adj)
		print("EDGE:")
		print(edge)
		print("GLOBAL MAP:")
		print(globalMap)
		if isEdgeinGraph(edge,globalMap):
			print("//////////////////////////////////////////")
			print(mapToUpdate)
			print("done with map")
			if (edge[0],edge[1],False) in mapToUpdate:
				#the edge is already in the map but not explored
				if isLeaf:
					i = mapToUpdate.index((edge[0],edge[1],False))
					mapToUpdate[i] = (edge[0],edge[1],True)
			if (edge[1],edge[0],False) in mapToUpdate:
				#the edge is already in the map but not explored
				if isLeaf:
					i = mapToUpdate.index((edge[1],edge[0],False))
					mapToUpdate[i] = (edge[1],edge[0],True)
			#if it gets here, then the edge has either already been explored
			#or it is not in the map at all
			if not isEdgeinGraph(edge,mapToUpdate):
				if isLeaf:
					mapToUpdate.append((edge[0],edge[1],True))
				else:
					mapToUpdate.append((edge[0],edge[1],False))
			#if it has been explored on the local map, then we don't do anything

#graph uses (x,y,isExplored) edges
def determineIsLeaf(node,graph):
	numEdges = 0
	for edge in graph:
		if edge[2] == True:
			#Can only know if it's a leaf if that edge (and thus that node) has been explored
			if edge[0] == node or edge[1] == node:
				#This is an edge coming out of the selected node
				numEdges = numEdges + 1
	if numEdges == 1:
		return True
	return False


#Returns an array in the format of RFID info to write to the RFID tag
#RFID info is an array [(xloc,yloc),[[xloc, yloc isLeaf] ... x4 ]]
def genRFIDTagUpdate(localMap,robotLocation):
	adjacents = []
	for edge in localMap:
		if edge[0] == robotLocation:
			adjacents.append([edge[1][0],edge[1][1],determineIsLeaf(edge[1],localMap)])
		if edge[1] == robotLocation:
			adjacents.append([edge[0][0],edge[0][1],determineIsLeaf(edge[1],localMap)])
	info = [robotLocation,adjacents]
	return info


# #Creates the RFID dictionary RFIDInfo
# 	#Key: (x,y) tuple location of the RFID tag
# 	#Value: a list of booleans indicating which adjacent edges have been explored
# 	#		e.g. [T,F,F,T] indicates the top has been explored, the right edge hasn't
# 	#			the bottom edge hasn't, and the right edge has been explored
# #Return: RFIDInfo
# def initializeGraph(edges):
# 	RFIDInfo = {}
# 	for edge in edges:
# 		node1 = edge[0]
# 		node2 = edge[1]
# 		node1 = (node1[0],node1[1])
# 		node2 = (node2[0],node2[1])
# 		if node1 not in RFIDInfo:
# 			RFIDInfo[node1] = [False,False,False,False]
# 		if node2 not in RFIDInfo:
# 			RFIDInfo[node2] = [False,False,False,False]
# 	return RFIDInfo

#performs the necessary sequence of map updates and get the next step.
#returns the new target location and the info to write to the RFID tag
#RFID info is the formatted information read from the RFID tag
def updateMapandDetermineNextStep(localMap,globalMap,robotLocation,RFIDInfo,allRobotTargetLocations=[],robotEdges=[]):
	#update robot map using "sensing" (i.e. the adjacent nodes in the global map)
	updateMapUsingSensing(localMap,globalMap,robotLocation)

	#update robot map using RFID tag
	updateMapUsingRFID(localMap,globalMap,RFIDInfo)
	preprocessedMap = copy.deepcopy(localMap)

	#get the next move using the robot map. This will also update the robot map to mark that edge has isExplored
	target = getNextMove(localMap,robotLocation,allRobotTargetLocations,robotEdges)

	#write to the RFID Tag (this can be moved to after updateMapUsingRFID if necessary)
	info = genRFIDTagUpdate(localMap,robotLocation)
	return (target,info,preprocessedMap)


def main():
	# edge1 = [(50,50), (50,60), False]
	# edge2 = [(50,50), (60,50), False]
	# edge3 = [(60,50), (60,60), True]
	# edge7 = [(60,60), (60,70), False]
	# edge8 = [(50,50), (50,40), False]
	# edge9 = [(50,50), (40,50), False]
	# edge10 = [(50,40), (60,40), False]
	# edge11 = [(60,40), (60,50), False]
	# edge12 = [(50,60), (50,70), False]
	# edge13 = [(50,70), (60,70), False]
	# edge14 = [(50,70), (40,70), False]
	# edge15 = [(60,50), (70,50), False]
	# edge16 = [(60,40), (70,40), False]
	# graph = [edge1, edge2, edge3, edge7, edge8, edge9, edge10, edge11, edge12, edge13, edge14, edge15, edge16]

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

	# graph = [edge1, edge2, edge3, edge7, edge8, edge9, edge10, edge11, edge12, edge13, edge14, edge15]
	# print(graph)
	# edgeGraph = [convertFromPlotEdgetoEdge(edge1),convertFromPlotEdgetoEdge(edge2),convertFromPlotEdgetoEdge(edge3),convertFromPlotEdgetoEdge(edge7)]

	edge4 = [(50,50), (50,60), False]
	edge5 = [(50,50), (60,50), False]
	edge6 = [(60,50), (60,60), True]
	graph2 = [edge4, edge5, edge6]

	# print(findValidPath((50,50),(60,70),graph))
	# print(euclidieanDistance((0,2),(0,0)))
	# print(getClosestNode((56,55),graph))

	# return

	plt.ion()
	# fig, ax = plt.subplots()
	x, y = [],[]
	line = plt.plot(x,y)[0]
	robotLocation1 = (80,20)
	robotLocation2 = (60,70)
	rmap1 = []

	while True:
		
		updateMapUsingSensing(rmap1,graph,robotLocation1)
		plotGraph(rmap1,[robotLocation1])
		# plotGraph(graph,[robotLocation1])
		# print(rmap1)
		plt.waitforbuttonpress()
		#get the next move using the robot map. This will also update the robot map to mark that edge has isExplored
		target = getNextMove(rmap1,robotLocation1)

		#write to the RFID Tag (this can be moved to after updateMapUsingRFID if necessary)
		info = genRFIDTagUpdate(rmap1,robotLocation1)
		# nextLoc2 = getNextMove(graph,robotLocation2,[nextLoc1],[(prevLoc1,nextLoc1)])
		print(target)
		# print(nextLoc2)
		robotLocation1 = target
		# robotLocation2 = nextLoc2
		

		# plotGraph(graph,[robotLocation1,robotLocation2])
		# # plt.draw()
		# # plotGraph(graph,[robotLocation1,robotLocation2])
		# prevLoc1 = robotLocation1
		# prevLoc2 = robotLocation2
		# nextLoc1 = getNextMove(graph,robotLocation1)
		# nextLoc2 = getNextMove(graph,robotLocation2,[nextLoc1],[(prevLoc1,nextLoc1)])
		# print(nextLoc1)
		# print(nextLoc2)
		# robotLocation1 = nextLoc1
		# robotLocation2 = nextLoc2
		# plt.waitforbuttonpress()


if __name__ == '__main__':
	main()