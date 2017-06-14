import multiprocessing as mp
import time, sys, subprocess
import wmi
from decimal import *
from math import factorial
from multiprocessing import Pool

import collections


def get_temp_info():
    w = wmi.WMI(namespace="root\OpenHardwareMonitor")
    temp = {}
    temperature_info = w.Sensor()
    for sensor in temperature_info:
        if sensor.SensorType == 'Temperature' and 'CPU Core' in sensor.Name:
            temp.update({sensor.Name: sensor.Value})

    # for key in temp.keys():
    #     print(key, "Temperature: ", temp[key], "Load: ", round(load[key], 2), "%")
    d = collections.OrderedDict(sorted(temp.items()))
    for key in d.keys():
        print(key, d[key])
    sys.stdout.flush()

def temp_info():
    temp_type = {1: "CPU", 2: "BOX", 3: "ENV", 4: "BOARD", 5: "BACKPLANE", 6: "CHIPSET", 7:"VIDEO"}
    sens_count = 0
    sens_name = ""
    sens_type = None
    sens_value = 0
    with subprocess.Popen("ktool32.exe temp GetTempSensorCount ",
                      stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
        for line in process.stdout.readlines(): # b'\n'-separated lines
            if b"count" in line:                    
                sens_count = int(line[27:28].decode('CP866'))

        for i in range(sens_count):
            with subprocess.Popen("ktool32.exe temp GetTempSensorInfo " + str(i),
                      stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
                for line in process.stdout.readlines(): 
                    if b"name" in line:                    
                        sens_name = line.decode('CP866').strip()
                    if b"type" in line:
                        sens_type = line[6:9]
                        sens_type = int(sens_type)
                        for j in temp_type:
                            if j == sens_type:
                                print('\n'+sens_name +" on "+ temp_type[sens_type])

            with subprocess.Popen("ktool32.exe temp GetTempSensorValue " + str(i),
                      stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
                for line in process.stdout.readlines():
                    if b"value" in line:
                        sens_value = float(line[14:21])
                        sens_value /= 1000
                        print("Temperature ", sens_value)


def main():

    # get_temp_info()
    temp_info()
    exit(0)

if __name__ == '__main__':
        main()
        print("\tEnd test\n".upper())
        sys.stdout.flush()
