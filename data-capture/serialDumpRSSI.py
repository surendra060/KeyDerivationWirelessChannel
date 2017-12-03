import serial,time,sys

ser = serial.Serial('/dev/ttyUSB0',57600)
filename = "RSSI(207-204)_RAW_500L_2000seq_" + sys.argv[1] +".txt"
f = open(filename,'w')
count = 0
while 1 :
	line = ser.readline()
	print(line)
	f.write(line)
	f.close()
	f = open(filename,'a')
		print("Counter"+count)
	count = count+1
	if count==4000:
		exit()
