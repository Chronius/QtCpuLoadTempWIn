import multiprocessing as mp
import time, sys
import wmi
from decimal import *
from math import factorial
from multiprocessing import Pool

import collections


def setpriority(pid=None, priority=1):
    """ Set The Priority of a Windows Process.  Priority is a value between 0-5 where
        2 is normal priority.  Default sets the priority of the current
        python process but can take any valid process ID. """

    import win32api, win32process, win32con

    priorityclasses = [win32process.IDLE_PRIORITY_CLASS,
                       win32process.BELOW_NORMAL_PRIORITY_CLASS,
                       win32process.NORMAL_PRIORITY_CLASS,
                       win32process.ABOVE_NORMAL_PRIORITY_CLASS,
                       win32process.HIGH_PRIORITY_CLASS,
                       win32process.REALTIME_PRIORITY_CLASS]
    if pid == None:
        pid = win32api.GetCurrentProcessId()
    handle = win32api.OpenProcess(win32con.PROCESS_ALL_ACCESS, True, pid)
    win32process.SetPriorityClass(handle, priorityclasses[priority])


def get_temp_info():
    w = wmi.WMI(namespace="root\OpenHardwareMonitor")
    temp = {}
    temperature_info = w.Sensor()
    for sensor in temperature_info:
        if sensor.SensorType == 'Temperature' and 'CPU Core ' in sensor.Name:
            temp.update({sensor.Name: sensor.Value})

    # for key in temp.keys():
    #     print(key, "Temperature: ", temp[key], "Load: ", round(load[key], 2), "%")
    d = collections.OrderedDict(sorted(temp.items()))
    for key in d.keys():
        print(key[10:], d[key])
    sys.stdout.flush()


def main():

    get_temp_info()
    exit(0)

if __name__ == '__main__':
        main()
        print("\tEnd test\n".upper())
        sys.stdout.flush()
