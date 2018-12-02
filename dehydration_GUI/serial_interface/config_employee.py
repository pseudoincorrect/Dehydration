import os
import pickle

###############################################################################
## Employee class
##
class ConfigEmployee(object):
    def __init__(self, parent=None):
        self.config_dict   = dict()
        self.employee_dict = dict()
        self.employee_conf_path = ""
        self.employee_list_path = ""
        self.start_set_config_dict()
        # self.employee_list_path = self.config_dict['emp_f_p']

    # read or create the config file and set the config dict
    def start_set_config_dict(self):
        dir_class = os.path.dirname(__file__)
        dir_conf = os.path.join(dir_class, '..\overview_conf')
        if (not os.path.isdir(dir_conf)):
            os.mkdir(dir_conf)
        file_conf = os.path.join(dir_class,
                                        '..\overview_conf\overview_conf.dat')
        self.employee_conf_path = file_conf
        if (not os.path.isfile(file_conf)):
            print ("creating new config file")
            self.create_config_file(file_conf, dir_conf)
        self.set_config_dict(file_conf)
        if (os.path.isfile(self.config_dict['emp_f_p'])):
            self.import_config(self.config_dict['emp_f_p'])

    # create the default config file
    def create_config_file(self, file_conf, dir_conf):
        employee_file_path = os.path.join(dir_conf, '.\employees_list.dat')
        initial_dict = \
        {
            # set the employee file path
            'emp_f_p' : employee_file_path
        }
        with open(file_conf, 'wb') as f:
            pickle.dump(initial_dict, f, pickle.HIGHEST_PROTOCOL)
        f.close()
        return employee_file_path

    # read the config file and set the config dict
    def set_config_dict(self, file_conf):
        with open(file_conf, 'rb') as f:
            self.config_dict = pickle.load(f)
        f.close()

    def is_in_emp_dict(self, addr_key):
        if (addr_key in self.employee_dict):
            return 1
        else:
            return 0

    def save_config(self, save_path):
        with open(save_path, 'wb') as f:
            pickle.dump(self.employee_dict, f, pickle.HIGHEST_PROTOCOL)
        f.close()
        self.update_list_path(save_path)

    def import_config(self, import_path):
        with open(import_path, 'rb') as f:
            self.employee_dict = dict(pickle.load(f))
        f.close()
        self.update_list_path(import_path)

    def update_list_path(self, list_path):
        with open(self.employee_conf_path, 'rb') as f:
            config_dict = pickle.load(f)
        f.close()
        config_dict['emp_f_p'] = list_path
        with open(self.employee_conf_path, 'wb') as f:
            pickle.dump(config_dict, f, pickle.HIGHEST_PROTOCOL)
        f.close()

    def list_employee(self):
        for i in self.employee_dict:
            print (i)
            print (self.employee_dict[i].name)

    def update_employee(self, addr_key, employee):
        if (addr_key in self.employee_dict):
            employee.name = self.employee_dict[addr_key].name
            employee.notif = self.employee_dict[addr_key].notif
        self.employee_dict[addr_key] = employee
        # print("employee updated ", addr_key[11])

    def notif_by_employee(self, addr_key):
        if (addr_key in self.employee_dict):
            employee = self.employee_dict[addr_key]
            employee.notif = 1
            self.employee_dict[addr_key] = employee
        else:
            print("employee does not exist")

    def un_notify(self, addr_key):
        if (addr_key in self.employee_dict):
            employee = self.employee_dict[addr_key]
            employee.notif = 0
            self.employee_dict[addr_key] = employee
        else:
            print("employee does not exist")

###############################################################################
## end class Employee #########################################################
###############################################################################
