import serial
import keyboard
import numpy as np
import time
import matplotlib.pyplot as plt

ser = serial.Serial('COM5')
ser.baudrate = 115200
temp = []


def capture():
    try:
        s = ser.read_all()
        if s != b'':
            b = s.decode('utf-8')
            if b != '0':
                for i in b:
                    temp.append(i)
    except:
        print('Error')


if __name__ == '__main__':
    while True:
        if keyboard.is_pressed("q"):
            print(temp)
            break
        else:
            capture()





