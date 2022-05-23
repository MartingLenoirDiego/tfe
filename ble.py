from asyncore import read
import serial
import keyboard
import matplotlib.pyplot as plt
import numpy as np
import time

x = np.linspace(0, 10, 100)
y = np.cos(x)

plt.ion()

figure, ax = plt.subplots(figsize=(8,6))
line1, = ax.plot(x, y)

plt.title("Dynamic Plot of sinx",fontsize=25)

plt.xlabel("X",fontsize=18)
plt.ylabel("sinX",fontsize=18)

datas = []
s = serial.Serial('COM6')
s.baudrate = 115200
count = 0
while True:
    if keyboard.is_pressed('q'):  # if key 'q' is pressed 
            break
    else:
        temp = s.read_all().decode("UTF-8")
        data = ''
        for i in temp:
            print(i)
            data = data + i
            if i == '*':
                data = data.replace('*','')
                datas.append(float(data))
                
                updated_y = float(data)
    
                line1.set_xdata(x)
                line1.set_ydata(updated_y)
                
                figure.canvas.draw()
                
                figure.canvas.flush_events()
                data = ''