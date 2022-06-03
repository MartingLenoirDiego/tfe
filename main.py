import serial
import matplotlib.pyplot as plt
import numpy as np
import sys


def read():
    datas = []
    s = serial.Serial('COM'+sys.argv[1])
    s.baudrate = 230400
    data = ''
    count = 0
    exit = 0
    while True:
        exit = exit + 1
        if exit == 15000:
            print("Port COM incorrect")
            quit()
        temp = s.read_all().decode("UTF-8")
        if temp != '':
            for i in temp:
                if i == '-':
                    count += 1
                    datas.append(round(int(data)/16384 * 1.8,2))
                    data = ''
                else:
                    data += i
                if count == 100:
                    fin = False
                    print(len(datas))
                    return fin,datas
def graph(datas):
    x=np.linspace(0,10,100)
    plt.plot(x,datas)  
    plt.ylabel('Tension')
    plt.xlabel("")
    plt.show()


if __name__ == "__main__":
    stop = True
    count = 0
    while stop:
        try:
            stop,datas = read()
        except:
            count = count+1
            if count == 20:
                quit()
    graph(datas)
    