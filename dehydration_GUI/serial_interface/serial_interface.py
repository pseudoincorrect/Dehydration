import serial
from . import serial_constants
from time import sleep

###############################################################################
## Test the COM port by sending a string destinated to be
## recognized by the nrf52 and check whether it answer correctly
## or not
## arg port_list: list of the COM port
##
def test_ports(port_list):
	for port_i in port_list:
		s               = serial.Serial(port_i, 115200, timeout=0.5)
		test_message    = serial_constants.TEST_MSG
		expected_answer = serial_constants.ANSWER_MSG
		response        = send_read(s, test_message)
		if (response == expected_answer):
			s.close()
			return port_i
		s.close()

###############################################################################
## Send a message with serial isntance provided and read the
## answer until a new line char appears
## arg s: serial instance
## arg serial_message: message to be sent
## arg sleep_time: time during which the program will "sleep"
##				   before reading the UART RX
##
def send_read(s, serial_message, sleep_time=0.1):
	s.write(serial_message.encode())
	sleep(sleep_time)
	bytesToRead = s.inWaiting()
	if (bytesToRead):
		response = s.readline().decode("utf-8")
		return response
	else:
		return

###############################################################################
## Send a message to the serial isntance provided and read the
## answer until a new line char appears, by polling the RX
## arg s: serial instance
## arg serial_message: message to be sent
##
def send_read_poll(s, serial_message):
	# print("send msg = ", serial_message)
	if (s != None):
		time_up  = 0
		response = None
		try:
			s.write(serial_message.encode())
			while(s.inWaiting() < len(serial_message)):
				# sleep(0.001)
				time_up = time_up + 1
				if (time_up > 20000):
					return
			response = s.readline().decode("utf-8")
		except serial.serialutil.SerialException:
			print ("Nrf52 not connected")
		return response
	else:
		print("Nrf52 not connected")
		return None

###############################################################################
## Try to read the serial and return the read message if any
## arg s: serial instance
##
def read(s):
	if (s != None):
		response = None
		try:
			# response = s.readline().decode("utf-8")
			bytesToRead = s.inWaiting()
			if (bytesToRead):
				# print ("bytesToRead ", bytesToRead)
				response = s.readline().decode("utf-8")
			else:
				return 0, None
		except serial.serialutil.SerialException:
			print ("Nrf52 not connected")
			return -1, None
		return 0, response
	else:
		print("Nrf52 not connected")
		return -1, None

###############################################################################
## Test Whether the nrf is still connected by sending a test
## message and checking the answer if present
## arg s: serial instance
##
def test_connection(s):
	response = None
	try:
		test_message = serial_constants.TEST_MSG
		response     = send_read_poll(s, test_message)
	except serial.serialutil.SerialException:
		pass
	return response


###############################################################################
## Get a list of the COM ports
##
##
def list_ports():
	ports  = ['COM%s' % (i + 1) for i in range(256)]
	result = []
	for port in ports:
		try:
			s = serial.Serial(port)
			s.close()
			result.append(port)
		except (OSError, serial.SerialException):
			pass

	return result

###############################################################################
## The port connected to the nrf52
##
def get_port():
	port_list = list_ports()
	nrf52_port = test_ports(port_list)
	if (nrf52_port == None):
		print ("Nrf52 NOT detected in the port list")
	else:
		print ("Nrf52 detected on port", nrf52_port)
	return nrf52_port

###############################################################################
## test the serial_interface module by checkink whether
## a nrf52 is connected or not
##
def module_test():
	#ser = serial.Serial('COM5', 115200, timeout=1)
	port_list = list_ports()
	print (port_list)
	nrf52_port = test_ports(port_list)
	if (nrf52_port == None):
		print ("Nrf52 NOT detected")
	else:
		print ("Nrf52 detected on port", nrf52_port)


if __name__ == '__main__':
    module_test()

