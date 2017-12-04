********************************************************************
@ Surendra Sharma 

Implementation of Post-Procesing on a PC/WorkStation - Freq 
Distribution Plots for all three scenarios (STATIC, MOBILE, SNIFFER)

********************************************************************
1. This folder has three folders, one for each scenario  (i.e. STATIC, SNIFFER, MOBILE). 
It uses a R program to draw frequency plots in .png format in respective folders. These 
graphs also depict the various threshholds set by different qunatization schemes, and has 
necessary input files (nodeA.txt and nodeB.txt (or nodeX_Y.txt format files for SNIFFER folders))

	
2. The program HistDistribPlot.R is used in each folder to read input files and create plots.
The plots are also superimposed with the Mean/Median values and threshhold values in each of 
the algorithm as relevant.

3. For SNIFFER folder, the graph plots for sniffer node EVE are plotted in a seperate folder than the 
plots for nodeA and nodeB.

