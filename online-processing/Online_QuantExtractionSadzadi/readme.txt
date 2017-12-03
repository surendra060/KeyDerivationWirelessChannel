********************************************************************
@ Surendra Sharma 

Implementation of Measureent and Qunatization using JeeNode v5 Devices
********************************************************************
1. This program folder implements measurement, storage and recording of 
RSS values and implenets Quantization scheme.

2. The PROJECT_DIR path and arduino binary file installed in the system 
is required to be correctly updated/reflected in "makefile"

3. The transmitter Node ID = 10 and Responder Node ID =20 (Can be changed Tx <15 
and RX>15 in the main cpp program file). 

4.	Commands :-
	make - To compile and th ebinary is created in "bin fiolder" 
	if the paths are correctly configured.
	
	make upload - to burn the binary in Arduiono (JeeNode) device flash
	(ensure that the Node ID is correctly configured in main CPP file
	before flashing)
	
5.	The output of extracted bitstreams and parameters can be observed using Arduino
IDE serial monitor or can be written to file using the python script.

6. Python Command to store Data to file :-

	python serialWriteFile.py <OptionalFilname_Append>
