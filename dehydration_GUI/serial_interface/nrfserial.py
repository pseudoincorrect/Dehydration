import serial
from time import sleep
from queue            import Queue
from threading        import Lock, Thread
from serial_interface import serial_constants   as S_C
from serial_interface import serial_interface   as S_I


###############################################################################
## Serial class
##
class NrfSerial(object):
    def __init__(self, parent=None):
        self.serial_mutex  = Lock()
        self.serial_read_t = None
        self.nrf_ser       = None
        self.serial_queue  = Queue()


    ###############################################################################
    ## Test and Connect the serial UART connection
    ##
    def connect_nrf(self):
        if (self.is_serial_connected()):
            print("nrf already connected")
            self.start_thread_serial_read()
            return 1
        else:
            self.get_nrf52()
            if (self.is_serial_connected()):
                self.start_thread_serial_read()
                return 1
            else:
                return 0

    ###############################################################################
    ## Checke whether the serial is still connected
    ##
    def is_serial_connected(self):
        print("checking connexion")
        if (self.nrf_ser != None):
            self.serial_mutex.acquire()
            # print ("mutex acquired)
            r_msg =  S_I.test_connection(self.nrf_ser)
            self.serial_mutex.release()
            # print ("mutex released")
            if (r_msg):
                return True
            else:
                self.nrf_ser = None
                return False
        else:
            return False

    ###############################################################################
    ## Get the port and create a UART serial instance
    ##
    def get_nrf52(self):
        nrf_ser_port = S_I.get_port()
        if (nrf_ser_port == None):
            self.nrf_ser = None
            return None
        else:
            self.nrf_ser = serial.Serial(nrf_ser_port, 115200, timeout=0.2)
        return 1

    ###############################################################################
    ## start the serial listening thread
    ##
    def start_thread_serial_read(self):
        if ((self.serial_read_t is None) or (not self.serial_read_t.isAlive())):
            self.serial_read_t = Thread(target=self.thread_serial_read)
            self.serial_read_t.daemon = True
            self.serial_read_t.start()

    ###############################################################################
    ## Serial listening infinite Thread function
    ##
    def thread_serial_read(self):
        while (True):
            self.serial_mutex.acquire()
            # print ("mutex acquired)
            ret_val = self.read_serial()
            self.serial_mutex.release()
            # print ("mutex released")
            if (ret_val == -1):
                break
            sleep(0.001)

    ###############################################################################
    ## Serial listening function
    ##
    def read_serial(self):
        ret_val, ret_msg = S_I.read(self.nrf_ser)
        if (ret_msg):
            # if (len(ret_msg) > 12):
                # if (ret_msg[12] == 'u'):
                #     print("", ret_msg[12])
                # else:
                #     print("  ", ret_msg[12])
            self.serial_queue.put(ret_msg)
        return ret_val

    # def start_thread_send_msg(self, msg):
    #     msg_to_send = str(msg)
    #     return_msg  = S_I.send_read_poll(self.nrf_ser, msg_to_send)
    #     if (self.is_serial_connected()):
    #         print("here")
            # self.serial_mutex.acquire()
    # print ("mutex acquired)
    #         msg_to_send = str(msg)
    #         return_msg  = S_I.send_read_poll(self.nrf_ser, msg_to_send)
            # self.serial_mutex.release()
    # print ("mutex released")

    ###############################################################################
    ## Send a test serial message
    ##
    def start_thread_send_msg(self, msg):
        serial_send_t = Thread(target=self.thread_send_msg, args=(msg,))
        serial_send_t.daemon = True
        serial_send_t.start()

    ###############################################################################
    ## Send a test serial message
    ##
    def thread_send_msg(self, msg):
        if (self.is_serial_connected()):
            print("here")
            self.serial_mutex.acquire()
            # print ("mutex acquired)
            msg_to_send = str(msg)
            S_I.send_read_poll(self.nrf_ser, msg_to_send)
            self.serial_mutex.release()
            # print ("mutex released")

    ###############################################################################
    ## SET LED
    ## msg example : cmd, eemployee/server=3, led=1
    ## SET_LED,0003,0001   (e_0003_0001)
    ##
    def send_to_node(self, node_name, cmd):
        msg_type    = S_C.TO_SINGLE
        node_addr   = node_name
        cmd_str     = cmd
        msg_to_send = str(msg_type) + str(node_addr) + str(cmd_str) + '\r'
        print(msg_to_send)
        self.start_thread_send_msg(msg_to_send)

    ###############################################################################
    ## SET LED
    ## msg example : cmd, eemployee/server=3, led=1
    ## SET_LED,0003,0001   (e_0003_0001)
    ##
    def send_to_all(self, cmd):
        msg_type    = S_C.TO_ALL
        cmd_str     = cmd
        msg_to_send = str(msg_type) + str(cmd_str) + '\r'
        print(msg_to_send)
        self.start_thread_send_msg(msg_to_send)

###############################################################################
## end class nrfSerial ########################################################
###############################################################################
