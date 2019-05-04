#!python3

import serial
import time

ser = serial.Serial('COM4', 115200, timeout=0)
print(ser.name)
# for x in range(100):
#     temp = ser.read(100)
#     if temp != b'':
#         print(temp)
#     time.sleep(0.1)
# print('Writing')
# ser.write(b'T')
# ser.flush()
# print('Just flushed')
iter = 0
node = 2
x = 1
y = 2
a = 3
while True:
    temp = ser.read(1000)
    if temp != b'':
        if temp == b'q' or temp == 'Q':
            ser.close()
            break
        print(temp)
    time.sleep(0.1)
    if iter % 10 == 0:
        message = str(node) + str(' x:') + str(x) + ' y:' + str(y) + ' a:' + str(a)
        ser.write(str.encode(message))
        ser.flush()
        # print("Just told the transmitter to send")
        # print(b'2-x:1y:1a:1')
    iter = iter + 1
ser.close()
