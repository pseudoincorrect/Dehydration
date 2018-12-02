from time import sleep
from queue import Queue
from threading import Lock, Thread
from serial_interface.employee import Employee
from serial_interface.nrfserial import NrfSerial
from serial_interface import serial_constants as S_C
from serial_interface.config_employee import ConfigEmployee


###############################################################################
## Employee class
##
class OverviewProcessing(object):
    def __init__(self, parent=None):
        self.nrf_serial      = NrfSerial()
        # self.employee_list   = EmployeeList()
        self.config_employee = ConfigEmployee()
        self.config_overview()
        self.start_thread_processing()

    ## start the UART receive and process thread
    def start_thread_processing(self):
        msg_process_th = Thread(target=self.thread_msg_process)
        msg_process_th.daemon = True
        msg_process_th.start()

    ## check the message queue and process its messages
    def thread_msg_process(self):
        while(True):
            if (not self.nrf_serial.serial_queue.empty()):
                msg = self.nrf_serial.serial_queue.get()
                self.msg_process(msg)
            sleep(0.001)


    ## retreive employee info from config file if any and update the dict
    def config_overview(self):
        pass
        # self.config_employee.config_init()

    ## process a message
    def msg_process(self, msg):
        if (msg[S_C.I_CMD] == S_C.MSG_UPDATE):
            self.msg_data_update(msg)
        elif (msg[S_C.I_CMD] == S_C.MSG_NOTIF):
            self.msg_notif(msg)

    ## refresh sensors data
    def msg_data_update(self, msg):
        addr       = str(msg[S_C.I_ADD : S_C.I_CMD])
        ambi_dehy  = str(msg[S_C.I_DEHID : S_C.I_DEHID + S_C.DATA_SIZE])
        ambi_humi  = str(msg[S_C.I_A_HUM : S_C.I_A_HUM + S_C.DATA_SIZE]) 
        ambi_temp  = str(msg[S_C.I_A_TEM : S_C.I_A_TEM + S_C.DATA_SIZE]) 
        skin_temp  = str(msg[S_C.I_S_TEM : S_C.I_S_TEM + S_C.DATA_SIZE]) 
        heart_rate = str(msg[S_C.I_HEART : S_C.I_HEART + S_C.DATA_SIZE])
        try :
            ambi_dehy  = int(ambi_dehy, 16)
            ambi_humi  = int(ambi_humi, 16)
            ambi_temp  = int(ambi_temp, 16)
            skin_temp  = int(skin_temp, 16)
            heart_rate = int(heart_rate, 16)
            employee   = Employee(ambi_dehy, ambi_humi, ambi_temp, 
                                                    skin_temp, heart_rate)
            self.config_employee.update_employee(addr, employee)
        except ValueError:
            print ("it was not an integer")
            pass

    def msg_notif(self, msg):
        addr = str(msg[S_C.I_ADD : S_C.I_CMD])
        # print("notif")
        self.config_employee.notif_by_employee(addr)


    ## print the employes and their details
    def list_employee(self):
        self.config_employee.list_employee()


###############################################################################
## end class Employee #########################################################
###############################################################################
