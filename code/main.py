import serial
import matplotlib.pyplot as plt
import numpy as np
import sys


def read():
    """
        This function read the datas from the port COM.
        The return of the function is:
            - False: that can tell to the main function to stop the loop.
            - Datas: This is the data that we read. But if we don't read a data this variable return false.
    """
    s = serial.Serial('COM'+sys.argv[1])  #Initialise the connection
    s.baudrate = 230400
    datas = []
    data = ''
    countExit = 0
    while True: 
        countExit = countExit + 1
        if countExit == 150000: #This is tell to the programm that we have any data because we still get out the function with the good port COM
            return False,False
            
        temp = s.read_all().decode("UTF-8")  # temps take the data in UTF-8 format
        if temp != '': # avoid the empty string 
            for i in temp:  
                if i == '-':  #tell us the end of one sample of data
                    count += 1
                    datas.append(round(int(data)/16384 * 1.8,2)) # Add the data to a table with all the datas. this formule convert 14 bit data(done by the ADC) to the real value of the voltage
                    data = ''
                else:
                    data += i
                if len(datas)==1000:  #stop the function when we have 1000 samples of data
                    return False,datas

def graph(datas): #Make a graph with the data
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