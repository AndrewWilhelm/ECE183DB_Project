#!python3

import serial
import time

import tkinter as tk
from functools import partial

ser = serial.Serial('COM4', 115200, timeout=0.1)
print(ser.name)

state = "Write"
rfidMessage = "I've been read from RFID"

window = tk.Tk()
window.title("Hand Held Device Controller")
window.geometry('1000x1000')
lbl = tk.Label(window, text="Hand Held Device Controller", font=("Calibri", 20))
lbl.pack(fill=tk.X)

readWriteFrame = tk.LabelFrame(window, text=" Insert Text to Write Below ", font=("Calibri", 12))
readWriteFrame.pack(fill=tk.BOTH, pady=10)
write_text = tk.Text(readWriteFrame)
# write_entry.grid(row=0,column=0, pady=10)
write_text.pack(side="left", fill=tk.BOTH, pady=10)

read_text = tk.StringVar()
read_text.set("Data On RFID Tag will Appear Here")
read_area = tk.Label(readWriteFrame, textvariable=read_text, font=("Calibri", 12))
read_area.pack(side="left", fill=tk.BOTH, pady=10)
# read_area.grid(row=0,column=1, pady=10)

def updateRead(message):

    read_text.set(message)

    write_text.configure(state="normal")
    write_text.delete(1.0,'end-1c')
    write_text.insert(1.0, message)
    write_text.configure(state="disabled")

def writeRFID():
    content = write_text.get(1.0,'end-1c')

    # print(content)
    message = 'w ' + content
    ser.write(str.encode(message))
    ser.flush()

def performState():
    if rwButton_text.get() == "Read":
        print("This button has no functionality - use the hardware button for reading.")
    else:
        writeRFID()

actionFrame = tk.LabelFrame(window, text="Read/Write", font=("Calibri", 12))
actionFrame.pack(fill=tk.BOTH, pady=10)
rwButton_text = tk.StringVar()
rwButton_text.set(state)
btn = tk.Button(actionFrame, textvariable=rwButton_text, command=performState)
btn.pack(side="top", fill=tk.BOTH, pady=10)


modeFrame = tk.LabelFrame(window, text=" Change Mode Below ", font=("Calibri", 12))
modeFrame.pack(fill=tk.BOTH, pady=10)


message = "Current Mode: "



def clicked():
    # res = "Welcome to " + txt.get()
    # lbl.configure(text=res)
    message = "Current Mode: "
    #print(btn_text.get())
    if btn_text.get() == message + "Read":
        state = "Write"
        write_text.configure(state="normal")
    else:
        state = "Read"
        write_text.configure(state="disabled")
        code = 'r'
        ser.write(str.encode(code))
        ser.flush()
    # btn_text.set("Current Mode: " + state)    
    # print("running")
    # print(state)
    btn_text.set(message + state)
    rwButton_text.set(state)
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
        if temp[0] == ord('%') and s == 'Read':
            
            #valid message
            temp = temp[2:-1]
            # print(temp)
            updateRead(temp)

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
