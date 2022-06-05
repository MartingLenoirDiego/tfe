import serial
import matplotlib.pyplot as plt
import numpy as np
import sys


def read():
    s = serial.Serial('COM'+sys.argv[1])
    s.baudrate = 230400
    datas = []
    data = ''
    countExit = 0
    while True:
        countExit = countExit + 1
        if countExit == 150000:
            return False,False
            
        temp = s.read_all().decode("UTF-8")
        if temp != '':
            for i in temp:
                if i == '-':
                    count += 1
                    datas.append(round(int(data)/16384 * 1.8,2))
                    data = ''
                else:
                    data += i
                if len(datas)==1000:
                    return False,datas
def graph(datas):
    x=np.linspace(0,10,1000)
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
                print("Port COM incorrect")
                sys.exit()
    if datas == False:
        print("Port COM incorrect")
        sys.exit()
    graph(datas)
    