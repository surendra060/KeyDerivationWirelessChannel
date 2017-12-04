********************************************************************
@ Surendra Sharma 

Implementation of Post-Procesing on a PC/WorkStation - Qunatization for Bit stream 
Extraction using R - programming environment.

********************************************************************
1. This folder implements four quantization algorithms in four folders to extract bitstrings, row wise 
statistical parameters used to calculate threshold, performance parameters and final binary output file 
to genererate raw binary file for entropy analysis. It has ywo following programs which must be executed 
in this sequence:-

********************************
ALGO_RSSQuantization.R
********************************

2. INPUT : - The conditioned output files from shell script are nodeA.txt and nodeB.txt which are 
used as input for  program ALGO_RSSQuantization.R (ALGO refers to particular Algorithm in folder)

3. OUTPUT :- Following files are output of ALGO_RSSQuantization.R program :-
	
	(a)	nodeA_BS_ALGO.txt and nodeB_BS_ALGO.txt - Bitstring output files with one ROW representing bits
	extracted from one row of input file of 1000 RSS values.
	
	(b)	nodeA_BS_data_ALGO.txt and nodeB_BS_data_ALGO.txt files have the same bits string in rows as in files 
	in (a) above But it also contains all statistical values used in calculation of thresholds and performance 
	parameters for each row. At the end the cumulative performance parameters are also stored. The row output 
	is in the following format :
	
	Median/Mean : Range : BitString_Len : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring
	
	(c)	nodeAB_ALGO_Entropy.txt file contains the Minimum length output of each row between row of bitstreams 
	extracted by both nodes. This file represents the maximum bits which can be extracted after error correction  
	of mismatch bits after information reconciliation stage. 


********************************
BitStringToBinFile.R
********************************
	
4. This is a common file to all the difrent algorithms as it converts one of the input file of Bit stream
extraction for entropy analysis.

5. INPUT :- nodeAB_ALGO_Entropy.txt

6. OUTPUT:- entropyALGO.bin is a binary file output which converts a text/string file to actual binary data file 
for Entropy analysis.


