#!python3

import serial
import time

import tkinter as tk


window = tk.Tk()
window.title("Hand Held Device Controller")
window.geometry('500x500')
lbl = tk.Label(window, text="Hand Held Device Controller", font=("Calibri", 20))
lbl.pack(fill=tk.X)

readWriteFrame = tk.LabelFrame(window, text=" Change Mode Below ", font=("Calibri", 12))
readWriteFrame.pack(fill=tk.BOTH, pady=10)
write_text = tk.Text(readWriteFrame)
# write_entry.grid(row=0,column=0, pady=10)
write_text.pack(side="left", fill=tk.BOTH, pady=10)
read_area = tk.Label(readWriteFrame, text="Data On RFID Tag will Appear Here", font=("Calibri", 12))
# read_area.grid(row=0,column=1, pady=10)
read_area.pack(side="right", fill=tk.BOTH, pady=10)

def clicked():
    res = "Welcome to " + txt.get()
    lbl.configure(text=res)
btn = tk.Button(window, text="Click Me", command=clicked)
btn.pack(fill=tk.X)

# labelframe = tk.LabelFrame(window, text=" 2.) Enter Coordinates for Tag Below (If Required)")
# labelframe.pack(fill=tk.X, pady=10)
 
# x_label = tk.Label(labelframe, text="X:")
# x_label.pack(side="left", padx=5, pady=10)
# x_entry = tk.Entry(labelframe, width=10)
# x_entry.pack(side="left", padx=5, pady=10)
# y_label = tk.Label(labelframe, text="Y:")
# y_label.pack(side="left", padx=5, pady=10)
# y_entry = tk.Entry(labelframe, width=10)
# y_entry.pack(side="left", padx=5, pady=10)
window.mainloop()

# ser = serial.Serial('COM12', 115200, timeout=0)
# print(ser.name)
# # for x in range(100):
# #     temp = ser.read(100)
# #     if temp != b'':
# #         print(temp)
# #     time.sleep(0.1)
# # print('Writing')
# # ser.write(b'T')
# # ser.flush()
# # print('Just flushed')
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
