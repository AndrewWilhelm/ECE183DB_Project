import matplotlib.pyplot as plt 
import numpy as np
import time
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
	plt.clf()
	plt.xlim(0,100)
	plt.ylim(0,100)
	plt.xticks(np.arange(0, 100+1, 10.0))
	plt.yticks(np.arange(0, 100+1, 10.0))
	plt.grid()
	plt.gca().invert_yaxis()
	plt.gca().xaxis.tick_top()

	for index in range(len(edges)):
		edge = edges[index]
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
	plt.pause(0.1)

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

def euclidieanDistance(node1,node2):
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
		dist = euclidieanDistance(point,nodeList[index])
		if  dist < minDistance:
			minDistance = dist
			closestNode = nodeList[index] 

	return closestNode

#Returns the next node that the robot at robotLocation should visit
#given the graph provided by edges (which need to include isExplored boolean values)
#Returns: a tuple (x,y) of the next node. None if there is no logical move available
def getNextMove(edges,robotLocation):
	for index in range(len(edges)):
		edge = edges[index]
	return

def main():
	edge1 = [(50,50), (50,60), True]
	edge2 = [(50,50), (60,50), True]
	edge3 = [(60,50), (60,60), False]
	edge7 = [(60,60), (60,70), False]
	edge8 = [(60,50), (70,50), False]
	edge9 = [(70,50), (80,50), False]
	edge10 = [(80,50), (80,60), False]
	edge11 = [(80,60), (80,70), False]
	edge12 = [(70,60), (70,70), False]
	edge13 = [(80,70), (70,70), False]
	edge14 = [(70,60), (70,50), False]
	edge15 = [(20,30), (20,20), False]

	# graph = [edge1[0:2], edge2[0:2], edge3[0:2], edge7[0:2]]
	graph = [edge1, edge2, edge3, edge7, edge8, edge9, edge10, edge11, edge12, edge13, edge14, edge15]
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


	#REMOVE THESE 3 LINES BELOW TO GET BACK TO CONTINUOUS UPDATES
	plotGraph(graph,[(50,50),(60,70)])
	plt.waitforbuttonpress()
	return

	while True:
		plotGraph(graph)
		# plt.draw()
		time.sleep(0.5)


		plotGraph(graph2)
		# plt.draw()
		time.sleep(0.5)

	plt.waitforbuttonpress()

if __name__ == '__main__':
	main()