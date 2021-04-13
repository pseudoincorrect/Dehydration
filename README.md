<h1 align="center">Dehydration Measurement System</h1>


[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

<p align="center"><img width=50% src="support/readme_assets/pcb_gui.jpg"></p>

# Quick Description

PCB, Embedded system firmware and complementary GUI to measure the vital signs of a group of individuals.
<br>
This project's code "Dehydration" is due to the final objective to analyze data and display a dehydration level of the user. 
<br>
(Machine Learning involved, from the data science department) 
<br>

# Details

This system enable us to measure different various physical values of a group of subjects and record/display them on desktop application. The values measured are skin temperature, surrounding humidity and surrounding temperature.
<br>

## Embedded System and PCB
The embedded system communicates with bluetooth LE to a base station connected to a computer. 
<br>
It uses the Nordic NRF SDK and acts as a beacon to transmit data at regular intervals.
<br>
The PCB has been made with Eagle and assembled on premises. 
<br><br>
<p align="center"><img width=30% src="support/readme_assets/pcb_hand.jpg"></p>
<br>

## GUI
The GUI is designed with Qt, PyQt more especially. 
<br>
It will search through the USB port for a base station and start receiving data when connected.
<br>
Data is then stored and displayed. The Devices are appearing and disapearing dynamically along their connections status.
<br>
Not the most elaborate UI, but it did a fine job back then and validated our proof of concept.
<br><br>

<p align="center"><img width=30% src="support/readme_assets/gui.jpg"></p>

# Note
This project is the first version/prototype as proof of concept which lead us to continue on a following project with a higher complexity and capabilities.

