

import serial,time,sys

ser = serial.Serial('/dev/ttyUSB0',57600)
filename = "QuantExtraction_MathurAono_" + sys.argv[1] +".txt"
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
