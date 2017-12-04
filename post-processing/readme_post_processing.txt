********************************************************************
@ Surendra Sharma 

Implementation of Post-Procesing on a PC/WorkStation - Pre-conditioning,
of RAW RSS capture files, Freq Distribution Plots/visualization Analysis
and Qunatization for Bit stream Extraction. Only the folder RSS_RAWdataFile_Conditioning
uses bash shell script for pre-conditioning/RAW capture file processing. All other 
folders use R - programming environment.

********************************************************************
1. The RAW RSS captured file using algorithms in data-capture folder are required to 
be processed in a particular sequence for visualization, evaluation and analysis.
This following folders are required for post processing in a pipelined sequence:

	(a) RSS_RAWdataFile_Conditioning Folder - Using bash shell script, it implements 
	validation RAW RSS file, Seperation and extraction of RSS measurements for 
	Tranmitter node A, and Responder node B in seperate files viz nodeA.txt and nodeB.txt.
	Also standardizes the output files with each row having rows of equal length of 1000 RSS
	values each.
	
	(b)	freqPlotDynamicDisplay Folder - Uses R program script to plot and display Frequency
	distribution graph for successive Rows in the files nodeA.txt and nodeB.txt. The two graphs
	(one each for nodes A and B) are concurrently shown, adjacent to each other. The start 
	row number and end row number can be changed inside the R Script. It depicts correspondance 
	of RSS measurement value between the transponder and Responder nodes and also overlays Mean, 
	Median and Normal Plot on the actual plot to indicate nature of Freq distribution curve.
	
	(c)	  Plot_FreqGraph_DiffScenarios Folder - While the previous graph display folder is dynamic and 
	R Studio to show all the graphs dynamically oen after the other, this folder implements saving of 
	frequency plots in .png format in various folders. It also has various folders for three diffrent 
	scenarios (i.e. STATIC, SNIFFER, MOBILE). These graphs also depict the various threshholds set by 
	different qunatization schemes, and has necessary input files (nodeA.txt and nodeB.txt (or nodeX_Y.txt
	format files for SNIFFER folders)).
	
	(d)	BitExtractionAlgorithms Folder - Implements the four algorithms for bit extraction and performance
	parameters calculations using ALGO_RSSQuantization.R file. This file outputs the Bitstring files (for 
	node A and B), Bitstring files for each node alongwith staistical data obtained for each row and one 
	file for both nodes which has common minimum length sequence. This common file indicates the actual number
	of bits which can be extracted after applying mismatch bit Error Correction / Information Reconcilation
	algorithms. The second R program file Aono_RSSQuantization.R takes the common length bitstring output to
	convert it to a binary Data file, with each bit being represented by one character (1/0) in extracted 
	bitstring.


