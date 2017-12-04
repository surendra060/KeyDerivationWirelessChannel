********************************************************************
@ Surendra Sharma 

Implementation of Post-Procesing on a PC/WorkStation - Pre-conditioning/Pre-processing

********************************************************************
1. One RAW RSS capture file is pre-conditioned to produce two files (one each for nodeA and nodeB) with 
standardized row length of 1000 RSS values.

2. It implements preconditioning of data captured using three diffrent scenarios in three seperated folders
(Proc_Static, Proc_SnifferData, Proc_DynMotion).


3. A bash script file of name procRawFile.sh is used in Static and Dynamic Motion scenario with :-

	INPUT - RAW_RSS_XXX.txt file as input comprising of RSS measurment data of both nodes in alternating rows.
	Also the size of each line / row in input file is limited to 500 RSS value due to RAM size limitation in 
	JeeNode v5 SRAM.  
	
	OUTPUT - nodeA.txt and nodeB.txt are final output file along with some intermediate temp_XXXX.txt files. 
	Each of the final output file has RSS measurement data of only one node (A or B) and has 1000 values in row. 

4.	The bash script file for Sniffer scenario is procRawFile_sniffer.txt and is used for data captured by 
Sniffer Node. of name procRawFile.sh is used in Static and Dynamic Motion scenario with :-

	INPUT - RSSI_RAW_SnifferData_All.txt file as input comprising of RSS measurment data of all four nodes in
	a single file. The RSS measurments by node A and node B for each other are represented with 10_20 and 20_10 
	in the begining of respective row. The measurement by sniffer node (ID 31) is represented as with 31_20 and 31_10  
	in the begining of respective row. R 
	
	OUTPUT - RSS measurements of each type measurements are seperated in four files (nodeA_B.txt, nodeB_A.txt, 
	nodeE_A.txt, nodeE_B.txt) where nodeX_Y.txt represents RSS node  measurement made by node X from Packets 
	transmitted by node Y. 
	
	
