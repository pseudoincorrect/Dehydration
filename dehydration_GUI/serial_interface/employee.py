import time

###############################################################################
## Employee class
##
class Employee(object):
    def __init__(self, dehydration = 0, ambiant_humidity = 0,
                ambiant_temperature = 0, skin_temperature = 0, heart_rate = 0,
                no_used_yet = 0, name = "no name", notif = 0, parent=None):
        self.name                = name
        self.dehydration = dehydration
        self.ambiant_humidity    = ambiant_humidity
        self.ambiant_temperature = ambiant_temperature
        self.skin_temperature    = skin_temperature
        self.heart_rate          = heart_rate
        self.no_used_yet         = no_used_yet
        self.notif               = notif
        self.last_activity = time.time()

    def update_name(self, name):
        self.name = name

###############################################################################
## end class Employee #########################################################
###############################################################################

###############################################################################
## Employee List class
##
class EmployeeList(object):
    def __init__(self, parent=None):
        self.employee_list = list()

