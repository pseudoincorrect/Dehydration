import logging
from serial_interface.employee import Employee

dataLogger = logging.getLogger('sensor_logger')

def init_logger():
    # create logger with 'spam_application'
    logger = logging.getLogger('sensor_logger')
    logger.setLevel(logging.DEBUG)
    # create file handler which logs even debug messages
    fh = logging.FileHandler('sensors.dat')
    fh.setLevel(logging.DEBUG)
    # create formatter and add it to the handlers
    formatter = logging.Formatter('%(asctime)s - %(message)s')
    fh.setFormatter(formatter)
    # add the handlers to the logger
    logger.addHandler(fh)
    dataLogger.info("******************************************")
    dataLogger.info("*********** NEW MEASUREMENTS *************")
    dataLogger.info("******************************************")

def log_to_file(employee):
    logMsg = "%s, skin %d C, air %d C, humidity %d %%" % (
        employee.name, 
        employee.skin_temperature, 
        employee.ambiant_temperature, 
        employee.ambiant_humidity)

    dataLogger.info(logMsg)