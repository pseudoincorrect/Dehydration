 #        8888888888  .d88888b.  8888888b.   .d88888b.
 #           888     d88P" "Y88b 888  "Y88b d88P" "Y88b
 #           888     888     888 888    888 888     888
 #           888     888     888 888    888 888     888
 #           888     888     888 888    888 888     888
 #           888     888     888 888    888 888     888
 #           888     Y88b. .d88P 888  .d88P Y88b. .d88P
 #           888      "Y88888P"  8888888P"   "Y888888"

	# push button to remove the inactive nodes
	# Use Qt threads
	# single node refresh
	# store data
	# display data on graphs

import re
import sys
import qdarkstyle
from time import sleep, time
from threading import Lock, Thread
from PyQt5 import QtGui, QtWidgets, QtCore
from serial_interface import serial_constants as S_C
# pylint: disable=no-name-in-module 
from PyQt5.QtWidgets import QApplication, QMainWindow 
from GUI.Employee_overview_ui import Ui_Employees_Overview
from serial_interface.overview_processing import OverviewProcessing
import serial_interface.data_logger as D_L

###############################################################################
## Classe used heriting from Push Button to animate the blinkin
##
class AnimatedButton(QtWidgets.QPushButton):
	def __init__(self, darkmode):
		QtWidgets.QPushButton.__init__(self)

		self.darkmode = darkmode

		if (self.darkmode):
			color1 = QtGui.QColor(0x31, 0x36, 0x3b)
			color2 = QtGui.QColor("black")
		else:
			color1 = QtGui.QColor(0xe1, 0xe1, 0xe1)
			color2 = QtGui.QColor("red")

		self.co_get = 0
		self.co_set = 0

		byar = QtCore.QByteArray()
		byar.append('zcolor')
		self.color_anim = QtCore.QPropertyAnimation(self, byar)
		self.color_anim.setStartValue(color1)
		self.color_anim.setKeyValueAt(0.5, color2)
		self.color_anim.setEndValue(color1)
		self.color_anim.setDuration(900)
		self.color_anim.setLoopCount(7)

		self.custom_anim = QtCore.QPropertyAnimation(self, byar)

	# parse the current palette
	def parseStyleSheet(self):
		ss = self.styleSheet()
		sts = [s.strip() for s in ss.split(';') if len(s.strip())]
		return sts

	# get the background color of the current palette
	def getBackColor(self):
		self.co_get += 1
		return self.palette().color(self.pal_ele)

	# Set the new backgroud
	def setBackColor(self, color):
		self.co_set += 1
		sss = self.parseStyleSheet()
		bg_new = 'background-color: rgba(%d,%d,%d,%d);' % (color.red(),
								color.green(), color.blue(), color.alpha())

		for k, sty in enumerate(sss):
			if re.search(r'\Abackground-color:', sty):
				sss[k] = bg_new
				break
		else:
			sss.append(bg_new)

		self.setStyleSheet('; '.join(sss))

	def gray_out(self):
		self.setEnabled(False)

	def un_gray_out(self):
		self.setDisabled(False)

	pal_ele = QtGui.QPalette.Window
	zcolor = QtCore.pyqtProperty(QtGui.QColor, getBackColor, setBackColor)


###############################################################################
## Main GUI window
##
class MyMainWindow(QMainWindow, Ui_Employees_Overview):
	def __init__(self, darkmode=0, parent=None):
		super(MyMainWindow, self).__init__(parent=None)
		self.setupUi(self)

		self.buttonGroup = QtWidgets.QButtonGroup(self)
		self.buttonGroup.buttonClicked.connect(self.handleButtonClicked)

		# Windows Icon setting-up
		icon = QtGui.QIcon()
		icon.addPixmap(QtGui.QPixmap("GUI/gui_image/drop_icon.jpg"),
		               					    QtGui.QIcon.Normal, QtGui.QIcon.On)
		self.setWindowIcon(icon)
		# Windows image setting-up
		QPixmap_path = "GUI/gui_image/water_bottle.jpg"
		self.LB_image.setPixmap(QtGui.QPixmap(QPixmap_path))
		# Push buttogn signal function setting-up
		self.PB_report           .clicked.connect(self.PB_report_f)
		self.PB_notif_all        .clicked.connect(self.PB_notif_all_f)
		self.PB_drink            .clicked.connect(self.PB_drink_f)
		self.PB_connect_nrf      .clicked.connect(self.PB_connect_nrf_f)
		self.PB_save             .clicked.connect(self.PB_save_f)
		self.PB_import           .clicked.connect(self.PB_import_f)
		self.PB_set_name         .clicked.connect(self.PB_set_name_f)

		self.darkmode = darkmode

		# instanciate main processing object
		self.overview_processing = OverviewProcessing()
		self.empl_button_dict = dict()
		self.empl_dict_temp = dict()
		# try a first connection to the nrf

		self.timer = QtCore.QTimer()
		# self.timer.timeout.connect(self.update)
		self.timer.start(1000) #trigger every sec.
		self.timer.timeout.connect(self.timer_refresh_UI)

		self.selected_employee_addr = ""
		self.blink_tr = None
		connect_nrf(self)

		D_L.init_logger()

		print("\nOVERVIEW Starting #################\n\n\r")

	###########################################################################
	# Class functions
	##
	def handleButtonClicked(self, button):
		for item in self.buttonGroup.buttons():
			if button is item:
				PB_employee_f(self, item.objectName())

	def PB_report_f(self):
		send_notif_cmd(self)

	def PB_notif_all_f(self):
		send_notif_all_cmd(self)

	def PB_drink_f(self):
		send_drink_cmd(self)

	def PB_connect_nrf_f(self):
		connect_nrf(self)

	# def PB_set_path_f(self):
	# 	set_config_file(self)

	def PB_set_name_f(self):
		showDial(self)

	def PB_save_f(self):
		save_config(self)

	def PB_import_f(self):
		import_config(self)

	def timer_refresh_UI(self):
		refresh_UI(self)

###############################################################################
## end class MyMainWindow #####################################################
###############################################################################

# function to display another form

###############################################################################
## function to display a form for the name changing
##
def showDial(M_W):
	if (M_W.selected_employee_addr == ""):
			print("no employee selected")
	else:
		text, okPressed = QtWidgets.QInputDialog.getText(M_W, "",
							"Employee name:", QtWidgets.QLineEdit.Normal, "")
		if okPressed and text != '':
			if (M_W.selected_employee_addr != ""):
				M_W.overview_processing.config_employee.employee_dict[ \
									M_W.selected_employee_addr].name = text

###############################################################################
## Refresh the UI
##
def refresh_UI(M_W):
	refresh_all(M_W)
	# TO DO implement a single not based on a Queue refresh process
	# refresh_all_cnt = 0
	# refresh_node(M_W)
	# refresh_all_cnt = (refresh_all_cnt + 1) % 5
	# if (not refresh_all_cnt):
	#	refresh_all(M_W)

###############################################################################
## to be implemented
## refresh a single node
##
def refresh_node(M_W):
	pass

###############################################################################
## Refresh all node in a row, rather heavy function so it needs to be
## caller not often
##
def refresh_all(M_W):
	# temporary coppy of the employee copy
	M_W.empl_dict_temp = dict(
						M_W.overview_processing.config_employee.employee_dict)
	# run through the temporary dict
	for empl_key in list(M_W.empl_dict_temp):
		empl_value = M_W.empl_dict_temp[empl_key]
		# add the employe to the button dict if does not exist
		if empl_key not in M_W.empl_button_dict:
			M_W.empl_button_dict[empl_key] = empl_value
			add_push_button(M_W, str(empl_key), empl_value)
		# else simply refresh the employee
		else:
			M_W.empl_button_dict[empl_key] = empl_value
			refresh_push_button(M_W, str(empl_key), empl_value)
			if (empl_value.notif):
				set_notif(M_W, empl_key)
		# refresh the employe detailed info
		if (empl_key == M_W.selected_employee_addr):
			PB_employee_f(M_W, empl_key)

		D_L.log_to_file(empl_value)
		

###############################################################################
## display a notification emanating from an employee by blinking his
## dedicated push button
##
def set_notif (M_W, empl_key):
	blink(M_W, empl_key)
	M_W.overview_processing.config_employee.un_notify(empl_key)
	employee       = M_W.empl_button_dict[empl_key]
	employee.notif = 0
	M_W.empl_button_dict[empl_key] = employee

###############################################################################
## Add a push button dynamically to the employee list the push buttons
## are contained in a pushbutton group
##
def add_push_button(M_W, button_name, employee):
	BT_name = button_name
	# empl_button = QtWidgets.QPushButton(BT_name, M_W.SAWidget_employee_list)
	empl_button = AnimatedButton(M_W.darkmode)
	empl_button.setObjectName(BT_name)
	empl_button_txt = get_push_button_txt(employee)

	empl_button.setText(empl_button_txt)
	M_W.verticalLayout.addWidget(empl_button)
	# link each employee buttons to its function
	for button in M_W.VGB_employees.findChildren(AnimatedButton):
	# for button in M_W.VGB_employees.findChildren(QtWidgets.QPushButton):
		if (M_W.buttonGroup.id(button) < 0):
			M_W.buttonGroup.addButton(button)

###############################################################################
## refresh a single node info and gray it out if inactive
##
def refresh_push_button(M_W, button_name, employee):
	for button in M_W.VGB_employees.findChildren(QtWidgets.QPushButton):
		if (button.objectName() == button_name):
			empl_key = button_name

			# empl_button_txt = str(employee.name) + " " + \
			# 			str(int(employee.dehydration))

			empl_button_txt = get_push_button_txt(employee)

			button.setText(empl_button_txt)
			if (not is_up_to_date(M_W, empl_key)):
				button.gray_out()
			else:
				button.un_gray_out()

###############################################################################
## Format employee info for the push button
##
def get_push_button_txt(employee):
		return str(employee.name) + '   ' + \
			"\n\rSkin Temperature " + \
			str(int(employee.skin_temperature)) +  '°C '\
			"\n\rAmbiant Temperature " + \
			str(int(employee.ambiant_temperature)) + '°C   ' + \
			"\n\rAmbiant Humidity " + \
			str(int(employee.ambiant_humidity)) +  '%'

###############################################################################
## Check if a single node has been inactive for too long
##
def is_up_to_date(M_W, empl_key):
	empl_date = M_W.empl_button_dict[empl_key].last_activity
	time_now  = time()
	delta_time = time_now - empl_date
	if (delta_time >= 10):
		return 0
	else:
		return 1

###############################################################################
## fuction called by the employee push button list, that will refresh
## the detailled info about the selected employee, call either by a
## push button click or a refresh all for the selected employee
##
def PB_employee_f(M_W, PB_name):
	employee = M_W.empl_button_dict[PB_name]
	update_selected_employee_info(	M_W,
									employee.name,
									PB_name,
									employee.ambiant_temperature,
									employee.ambiant_humidity,
									employee.skin_temperature)

###############################################################################
# Test and Connect the serial UART connection
##
def connect_nrf(M_W):
	ret = M_W.overview_processing.nrf_serial.connect_nrf()
	if (ret):
		M_W.LB_status_val.setText("Connected")
	else:
		M_W.LB_status_val.setText("Not Connected")

###############################################################################
# Update the labels containing the info of the current
# selected employee
# arg number: employee number
# arg name: employee name
# arg dehydration: employee dehydration number
##
def update_selected_employee_info(M_W, name, address, 
			ambiant_temperature, ambiant_humidity, skin_temperature):
	M_W.selected_employee_addr  = address
	M_W.LB_name_val        .setText(name)
	M_W.LB_select_val      .setText(str(address))
	M_W.LB_dehydration_val .setText(str(int(skin_temperature))+'°C')
	M_W.LB_temperature_val .setText(str(int(ambiant_temperature))+'°C')
	M_W.LB_humidity_val    .setText(str(int(ambiant_humidity))+'%')

###############################################################################
## Save the configuration info of the employee (name and other data)
##
def save_config(M_W):
	save_path = QtWidgets.QFileDialog.getSaveFileName()
	if (save_path[0] != ""):
		M_W.overview_processing.config_employee.save_config(save_path[0])

###############################################################################
## Import a configuration, usefull if we don't want to retype the employee
## name each time
##
def import_config(M_W):
	import_path = QtWidgets.QFileDialog.getOpenFileName()
	if (import_path[0] != ""):
		M_W.overview_processing.config_employee.import_config(import_path[0])

###############################################################################
## Blink a custom push button in the employee list
##
def blink(M_W, empl_key):
	for button in M_W.VGB_employees.findChildren(AnimatedButton):
		if (button.objectName() == empl_key):
			button.color_anim.stop()
			button.color_anim.start()

###############################################################################
## send drink command notification to the currently selected node
##
def send_drink_cmd(M_W):
	if (M_W.selected_employee_addr != ""):
		M_W.overview_processing.nrf_serial.send_to_node(
						M_W.selected_employee_addr, S_C.DRINK_NOTIF)
	else:
		M_W.LB_name_val.setText("No employee selected")

###############################################################################
## send general command notification to the currently selected node
##
def send_notif_cmd(M_W):
	if (M_W.selected_employee_addr != ""):
		M_W.overview_processing.nrf_serial.send_to_node(
							M_W.selected_employee_addr, S_C.NOTIF)
	else:
		M_W.LB_name_val.setText("No employee selected")

###############################################################################
## send notif to ALL nodes
##
def send_notif_all_cmd(M_W):
	M_W.overview_processing.nrf_serial.send_to_all(S_C.NOTIF_ALL)

###############################################################################
## MAIN
##
def main():
	app = QApplication(sys.argv)

	darkmode = 0

	arguments = sys.argv[1:]
	count = len(arguments)
	if (count  == 0):
		pass
	elif ((count == 1) and (sys.argv[1] == "-dark")):
		# Set dark theme
		darkmode = 1
		app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())
	else:
		print("\n\r    USAGE: python main.py [-dark] \n\r")
		return 1

	w = MyMainWindow(darkmode)
	# w = MyMainWindow()
	w.show()
	sys.exit(app.exec_())


if __name__ == '__main__':
    main()
