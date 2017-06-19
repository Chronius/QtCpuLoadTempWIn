import wmi, subprocess, sys

Drive_Type = {"Unknown" : 0, "No Root Directory" : 1, "Local Disk" : 2,
             "Network Drive" : 3, "Compact Disc" : 4, "RAM Disk" : 5}

dir_tool = ""


class hw_info():
    def __init__(self):
        self.w = wmi.WMI()
        self.list_info = [arg for arg in dir(self) if (callable(getattr(self, arg)) \
                                                and not arg.startswith('_'))]

        self.dict_info = dict((k, False) for k in self.list_info)

    #return list of tuples [(Type0, Name0), (Type1, Name1), ...]
    def hard_info(self):
        disk_list = []
        for disk in self.w.Win32_DiskDrive():
            if "SanDisk Ultra" in disk.Model or "Virtual Disk" in disk.Model:
               continue
            if "Removable" in disk.MediaType:
               continue
            #print("Name =", disk.Name)
            print(disk.Model)
            #print("MediaType =", disk.MediaType)
            size = (int(disk.Size)/pow(1024,3))
            print(round(size, 2))
            #print("Sector size =", disk.BytesPerSector)
            #print("Serial number =", disk.SerialNumber)
            #print("InterfaceType =", disk.InterfaceType, "\n")
            #disk_list.append((disk.Name, disk.Model))
        #return disk_list

    def flash_info(self):
        disk_list = []
        for disk in self.w.Win32_DiskDrive():
            if "Fixed hard" in disk.MediaType:
                continue
            if "4C530001211008107113" in disk.SerialNumber or "Virtual Disk" in disk.Model:
                continue
            print("Name =", disk.Name)
            print("Model =", disk.Model)
            print("MediaType =", disk.MediaType)
            size = (int(disk.Size) / pow(1024, 3))
            print("Size =", round(size, 2), "Gbyte")
            print("Sector size =", disk.BytesPerSector)
            print("Serial number =", disk.SerialNumber)
            print("InterfaceType =", disk.InterfaceType, "\n")
            disk_list.append((disk.Name, disk.Model))
        return disk_list

    def com_info(self):
        for port in self.w.Win32_SerialPort():
            print("Name =", port.Name)
            print("ProviderType =", port.ProviderType)
            print("MaxBaudRate =", port.MaxBaudRate)
            print("Status =", port.Status, "\n")

    def mem_info(self):
        for dev in self.w.Win32_PhysicalMemory():
            #print(dev)
            #print("Name =", dev.Name)
            print(dev.Manufacturer)
            size = int(dev.Capacity)/pow(1024, 3)
            print(size)
            #print("DeviceLocator =", dev.DeviceLocator)
            #print("SerialNumber =", dev.SerialNumber)            
            #print("---------------------------------------")
        #for dev in self.w.Win32_PerfFormattedData_PerfOS_Memory():
            #print("Memory free =", dev.AvailableMbytes, "Mb")
            #print("")

    def voltage_info(self):
        volt_type = {1: "VCORE", 2: "1V8", 3: "2V5", 4: "3V3", 5: "VBAT", 6: "5V", 7: "5VSB", 8: "12V", 9: "AC"}
        sens_count = 0
        with subprocess.Popen(dir_tool+"ktool32.exe volt GetVoltageSensorCount ",
                          stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
            for line in process.stdout.readlines(): # b'\n'-separated lines
                if b"count" in line:                    
                    sens_count = int(line[22:26].decode('CP866'))
            for i in range(sens_count):
                with subprocess.Popen(dir_tool+"ktool32.exe volt GetVoltageSensorInfo " + str(i),
                          stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
                    for line in process.stdout.readlines():
                        if b"type" in line:
                            sens_type = line[6:9]
                            sens_type = int(sens_type)
                            for j in volt_type:
                                if j == sens_type:
                                    print(volt_type[sens_type])

                sens_value = 0
                with subprocess.Popen(dir_tool+"ktool32.exe volt GetVoltageSensorValue " + str(i),
                          stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
                    for line in process.stdout.readlines():
                        if b"value" in line:
                            sens_value = float(line[14:21])
                            sens_value /= 1000
                            print(sens_value , "V")
                if not(sens_value):
                    print(sens_value)

    def temp_info(self):
        temp_type = {1: "CPU", 2: "BOX", 3: "ENV", 4: "BOARD", 5: "BACKPLANE", 6: "CHIPSET", 7:"VIDEO"}
        sens_count = 0
        with subprocess.Popen(dir_tool+"ktool32.exe temp GetTempSensorCount ",
                          stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
            for line in process.stdout.readlines(): # b'\n'-separated lines
                if b"count" in line:                    
                    sens_count = int(line[27:28].decode('CP866'))

            for i in range(sens_count):
                with subprocess.Popen(dir_tool+"ktool32.exe temp GetTempSensorInfo " + str(i),
                          stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
                    for line in process.stdout.readlines(): 
                        if b"type" in line:
                            sens_type = line[6:9]
                            sens_type = int(sens_type)
                            for j in temp_type:
                                if j == sens_type:
                                    print(temp_type[sens_type])

                sens_value = 0
                with subprocess.Popen(dir_tool+"ktool32.exe temp GetTempSensorValue " + str(i),
                          stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, bufsize=1) as process:
                    for line in process.stdout.readlines():
                        if b"value" in line:
                            sens_value = float(line[14:21])
                            sens_value /= 1000
                            print(sens_value)
                if not(sens_value):
                    print(sens_value)

                    

    def cpu_info(self):
        for cpu in self.w.Win32_Processor():
            print(cpu.Name)
            # print("CPU Family =", cpu.Caption)
            print(cpu.NumberOfLogicalProcessors)
            # print("NumberOfCores =", cpu.NumberOfCores)
            # print("CurrentClockSpeed =", cpu.CurrentClockSpeed)

    def eth_info(self):
        #enable all adapters to see macaddress if there is
        num = 3
        ind_list = []
        #print("=======================================")
        for nic in self.w.Win32_PerfFormattedData_Tcpip_NetworkInterface():
            print(nic.Name)
            print(nic.BytesTotalPerSec)
            print(nic.CurrentBandwidth)

        return
        for nic in self.w.Win32_NetworkAdapterConfiguration():
            #if  "I210" in str(nic.Description) and nic.MACAddress:
            if  nic.MACAddress:
                #print(str(nic.Index) + " success enable")
                #print("MAC =", nic.MACAddress)
                print(nic.Description)
                if (str(nic.IPEnabled)=="True"):
                    print("On")
                else:
                    print("Off")
                #print("---------------------------------------")
                #ind_list.append((nic.Index, nic.IPEnabled, nic.Description))
        #print(ind_list)
        #return ind_list

    def _get_data(self):
        return self.dict_info

    def _update(self, details):
        self.dict_info = details.copy()


if __name__ == '__main__':
    
    hw = hw_info()
    argc = ''
    if len(sys.argv) > 1:
        argc = sys.argv[1]
    else:
        exit()

    for arg in dir(hw_info):
        if not arg.startswith('_'):
            if argc in arg:
                # print(arg)
                eval("hw."+arg+"()")
    sys.stdout.flush()
    exit(0)