# @ Surendra Sharma
#Python file to Write Serial Port Data to file. File name can be customized with Date, Identifier string etc.
# by specifying the Identifier string as an argument to this Python Script.
# This Script is generated using examples from Online sources such as Stackoverflow.com and documentation
# on pySerial Library of python at - http://pyserial.readthedocs.io/en/latest/shortintro.html


import serial,time,sys

ser = serial.Serial('/dev/ttyUSB0',57600)
filename = "QuantExtraction_Aono_" + sys.argv[1] +".txt"
f = open(filename,'w')
count = 0
while 1 :
	line = ser.readline()
	print(line)
	f.write(line)
	f.close()
	f = open(filename,'a')
	count = count+1
	if count==1000:
		exit()
