#!python3

import serial
import time

import tkinter as tk
from functools import partial

ser = serial.Serial('COM12', 115200, timeout=0.1)
print(ser.name)

state = "Clear"

window = tk.Tk()
window.title("Hand Held Device Controller")
# window.geometry('1000x700')
lbl = tk.Label(window, text="Hand Held Device Controller", font=("Calibri", 20))
lbl.pack(fill=tk.X)

# readWriteFrame = tk.LabelFrame(window, text=" Insert Text to Write Below ", font=("Calibri", 12))
# readWriteFrame.pack(fill=tk.BOTH, pady=10)
# write_text = tk.Text(readWriteFrame)
# write_text.pack(side="left", fill=tk.BOTH, pady=10)

# read_text = tk.StringVar()
# read_text.set("Data On RFID Tag will Appear Here")
# read_area = tk.Label(readWriteFrame, textvariable=read_text, font=("Calibri", 12))
# read_area.pack(side="left", fill=tk.BOTH, pady=10)
# read_area.grid(row=0,column=1, pady=10)
readAndEditFrames = tk.Frame(window)
readAndEditFrames.pack(side="top", fill=tk.X, pady=(10, 0), expand=True)
editFrame = tk.LabelFrame(readAndEditFrames, text=" Edit ", font=("Calibri", 12))
editFrame.pack(side="left", fill=tk.X, pady=10, expand=True)
readFrame = tk.LabelFrame(readAndEditFrames, text=" Read ", font=("Calibri", 12))
readFrame.pack(side="right", fill=tk.X, pady=10, expand=True)

# set up the edit frame
tk.Label(editFrame, text="X").grid(row=0)
tk.Label(editFrame, text="Y").grid(row=1)
xe_var = tk.StringVar()
ye_var = tk.StringVar()
xe = tk.Entry(editFrame, textvariable=xe_var)
ye = tk.Entry(editFrame, textvariable=ye_var)
xe.grid(row=0, column=1)
ye.grid(row=1, column=1)
editNodes = []
for j in range(4):
    frame = tk.LabelFrame(editFrame, text=" Node " + str(j+1) + " ", font=("Calibri", 12))
    frame.grid(row=2+j, columnspan=2)
    tk.Label(frame, text="X").grid(row=0)
    tk.Label(frame, text="Y").grid(row=1)
    tk.Label(frame, text="isLeaf").grid(row=2)
    x_var = tk.StringVar()
    y_var = tk.StringVar()
    isLeaf_var = tk.StringVar()
    x = tk.Entry(frame, textvariable=x_var)
    y = tk.Entry(frame, textvariable=y_var)
    isLeaf = tk.Entry(frame, textvariable=isLeaf_var)
    x.grid(row=0, column=1)
    y.grid(row=1, column=1)
    isLeaf.grid(row=2, column=1)
    nodeList = [x_var, y_var, isLeaf_var]
    editNodes.append(nodeList)

# Set up the read frame
tk.Label(readFrame, text="X").grid(row=0)
tk.Label(readFrame, text="Y").grid(row=1)
xr_var = tk.StringVar()
yr_var = tk.StringVar()
xr = tk.Entry(readFrame, state="disabled", textvariable=xr_var)
yr = tk.Entry(readFrame, state="disabled", textvariable=yr_var)
xr.grid(row=0, column=1)
yr.grid(row=1, column=1)
readNodes = []
for j in range(4):
    frame = tk.LabelFrame(readFrame, text=" Node " + str(j+1) + " ", font=("Calibri", 12))
    frame.grid(row=2+j, columnspan=2)
    tk.Label(frame, text="X").grid(row=0)
    tk.Label(frame, text="Y").grid(row=1)
    tk.Label(frame, text="isLeaf").grid(row=2)
    x_var = tk.StringVar()
    y_var = tk.StringVar()
    isLeaf_var = tk.StringVar()
    x = tk.Entry(frame, textvariable=x_var, state="disabled")
    y = tk.Entry(frame, textvariable=y_var, state="disabled")
    isLeaf = tk.Entry(frame, textvariable=isLeaf_var, state="disabled")
    x.grid(row=0, column=1)
    y.grid(row=1, column=1)
    isLeaf.grid(row=2, column=1)
    nodeList = [x_var, y_var, isLeaf_var]
    readNodes.append(nodeList)


# clears the entries for the read panel
def clearRead():
    xr_var.set("")
    yr_var.set("")
    for nodeNum in range(4):
        for index in range(3):
            readNodes[nodeNum][index].set("")


# clears the entries for the edit panel
def clearEdit():
    xe_var.set("")
    ye_var.set("")
    for nodeNum in range(4):
        for index in range(3):
            editNodes[nodeNum][index].set("")


# the message is in the formate below
# $message x y [x1 y1 iL1]...        where x,y are 3 numbers long and iL is either 1 or 0
# if the node does not exist, it will not transmit it
def updateRead(message):
    clearRead()
    clearEdit()
    xr_var.set(str(int(message[3:6])))
    yr_var.set(str(int(message[7:10])))
    xe_var.set(str(int(message[3:6])))
    ye_var.set(str(int(message[7:10])))
    index = 11
    nodeNum = 0
    # this loop assumes the index is at the open bracket
    while(nodeNum < 4 and index+11 < len(message)):
        index = index + 1  # skip past first open bracket
        readNodes[nodeNum][0].set(str(int(message[index:index+3])))
        editNodes[nodeNum][0].set(str(int(message[index:index+3])))
        index = index + 4
        readNodes[nodeNum][1].set(str(int(message[index:index+3])))
        editNodes[nodeNum][1].set(str(int(message[index:index+3])))
        index = index + 4
        readNodes[nodeNum][2].set(message[index])
        editNodes[nodeNum][2].set(message[index])
        index = index + 3  # to get to the next open bracket
        nodeNum = nodeNum + 1


    # read_text.set(message)

    # write_text.configure(state="normal")
    # write_text.delete(1.0,'end-1c')
    # write_text.insert(1.0, message)
    # write_text.configure(state="disabled")

# writes a message to the serial line with the following format
# $message x y [x1 y1 iL1] [x2 y2 iL2]...        where x,y are 3 numbers long and iL is either 1 or 0
def writeRFID():
    if(not xe_var.get() or not ye_var.get()):
        return
    toPrint = "$w "
    try:
        x = int(xe_var.get())
        y = int(ye_var.get())
        if(x < 0 or x > 100 or y < 0 or y > 100):
            "X and Y weren't valid"
            return
        toPrint = toPrint + "{:03d}".format(x) + " "
        toPrint = toPrint + "{:03d}".format(y)
        for nodeNum in range(4):
            # check if any of the entries are empty
            if not editNodes[nodeNum][0].get() or not editNodes[nodeNum][1].get() or not editNodes[nodeNum][2].get():
                print("some entries were empty")
                continue
            x = int(editNodes[nodeNum][0].get())
            y = int(editNodes[nodeNum][1].get())
            isLeaf = int(editNodes[nodeNum][2].get())
            if(x < 0 or x > 100 or y < 0 or y > 100 or not (isLeaf == 1 or isLeaf == 0)):
                print("Some of the values weren't valid")
                print(x)
                print(y)
                print(isLeaf)
                return
            toPrint = toPrint + " [" + "{:03d}".format(x) + " "
            toPrint = toPrint + "{:03d}".format(y) + " "
            toPrint = toPrint + str(isLeaf) + "]"
    except:
        print("Had an exception")
        return

    print(toPrint)
    ser.write(str.encode(toPrint))
    ser.flush()
    message = "Current Mode: "
    state = "Write"
    btn_text.set(message + state)


def eraseNodes():
    for nodeNum in range(4):
        for index in range(3):
            editNodes[nodeNum][index].set("")


def transferData():
    xe_var.set(xr_var.get())
    ye_var.set(yr_var.get())
    for nodeNum in range(4):
        for index in range(3):
            editNodes[nodeNum][index].set(readNodes[nodeNum][index].get())


editButtonsFrame = tk.Frame(window)
editButtonsFrame.pack(fill=tk.BOTH, pady=1)
eraseNodesBtn = tk.Button(editButtonsFrame, text="Erase Node Fields", command=eraseNodes)
eraseNodesBtn.pack(side="left", fill=tk.BOTH, expand=True)
transferBtn = tk.Button(editButtonsFrame, text="Transfer Data", command=transferData)
transferBtn.pack(side="right", fill=tk.BOTH, expand=True)

actionFrame = tk.LabelFrame(window, text=" Send Write Command ", font=("Calibri", 12))
actionFrame.pack(fill=tk.BOTH, pady=10)
btn = tk.Button(actionFrame, text="Send", command=writeRFID)
btn.pack(side="top", fill=tk.BOTH, pady=10)


modeFrame = tk.LabelFrame(window, text=" Change Mode Below ", font=("Calibri", 12))
modeFrame.pack(fill=tk.BOTH, pady=10)

message = "Current Mode: "


def clicked():
    # res = "Welcome to " + txt.get()
    # lbl.configure(text=res)
    message = "Current Mode: "
    #print(btn_text.get())
    if btn_text.get() == message + "Write":
        state = "Clear"
        # write_text.configure(state="disabled")
        code = '$c'
        ser.write(str.encode(code))
        ser.flush()
    elif btn_text.get() == message + "Read":
        state = "Write"
        # write_text.configure(state="normal")
    else:
        state = "Read"
        # write_text.configure(state="disabled")
        code = '$r'
        ser.write(str.encode(code))
        ser.flush()
    # btn_text.set("Current Mode: " + state)    
    # print("running")
    # print(state)
    btn_text.set(message + state)
    # print(btn_text.get())
    # btn_text.set("HEY" + state)

btn_text = tk.StringVar()
btn_text.set(message + state)
btn = tk.Button(modeFrame, textvariable=btn_text, command=clicked)
btn.pack(side="top", fill=tk.BOTH, pady=10)
# action_with_arg = partial(clicked,state)
# btn = tk.Button(window, text="Change Mode", command=clicked)
# btn = tk.Button(window, text="Change Mode", command=action_with_arg)
btn.pack(fill=tk.X)

clicked()
while True:
    window.update_idletasks()
    window.update()

    # temp = ser.read(1000)
    temp = ser.readline()
    # print(temp)

    # For some reason, state doesn't update in this loop... s is used to hold the updated current state
    s = btn_text.get()
    # print(s)
    s = s[-4:]
    # print(s)
    if temp != b'':
        # print(temp)
        # print((temp[0]))
        # print(state)
        # print(s)
        if temp[0] == ord('$') and s == 'Read':
            #valid message
            # temp = temp[2:-1]
            # print(temp)
            updateRead(temp.decode("utf-8"))

# window.mainloop()

# read_area = tk.Label(window, textvariable=btn_text, font=("Calibri", 12))
# read_area.pack(side="top", fill=tk.BOTH, pady=10)

# def update_btn_text():
#     btn_text.set("b")



# labelframe = tk.LabelFrame(window, text=" 2.) Enter Coordinates for Tag Below (If Requi   red)")
# labelframe.pack(fill=tk.X, pady=10)
 
# x_label = tk.Label(labelframe, text="X:")
# x_label.pack(side="left", padx=5, pady=10)
# x_entry = tk.Entry(labelframe, width=10)
# x_entry.pack(side="left", padx=5, pady=10)
# y_label = tk.Label(labelframe, text="Y:")
# y_label.pack(side="left", padx=5, pady=10)
# y_entry = tk.Entry(labelframe, width=10)
# y_entry.pack(side="left", padx=5, pady=10)



# for x in range(100):
#     temp = ser.read(100)
#     if temp != b'':
#         print(temp)
#     time.sleep(0.1)
# print('Writing')
# ser.write(b'T')
# ser.flush()
# print('Just flushed')
# iter = 0
# node = 2
# x = 1
# y = 2
# a = 3
# while True:
#     temp = ser.read(1000)
#     if temp != b'':
#         if temp == b'q' or temp == 'Q':
#             ser.close()
#             break
#         print(temp)
#     time.sleep(0.1)
#     if iter % 10 == 0:
#         message = str(node) + str(' x:') + str(x) + ' y:' + str(y) + ' a:' + str(a)
#         ser.write(str.encode(message))
#         ser.flush()
#         # print("Just told the transmitter to send")
#         # print(b'2-x:1y:1a:1')
#     iter = iter + 1
# ser.close()
