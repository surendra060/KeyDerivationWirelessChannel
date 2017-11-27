#!/bin/bash

#inFile=$1
#outFile_TX="Node10.txt"
#outFile_RX="Node20.txt"
tempFileTX="tempTx.txt"
tempFileRX="tempRx.txt"
finalTX="nodeA.txt"
finalRX="nodeB.txt"
tempFile="temp"  # Inetrmediate file to write randomly selecetd line. COncatenetated at the ednd of orig file

finLines="1000" # How many Total Lines required Finally
inLines=$(wc -l < $1) # Num of Lines in the input file
newLine=$[$finLines-$inLines] 
echo $newLine

i="0"

while [ $i -lt $newLine ] # While loop for balance lines
do

randLine=$[$RANDOM % $inLines + 1 | bc]
echo "Line No : $randLine"

if [ $[$randLine%2] -eq 1 ] # If rand Line num is for Tx (node 10) line
then
	nextLine=$[$randLine+1]
	sed "${randLine}q;d" $1 >> $tempFile
	sed "${nextLine}q;d" $1 >> $tempFile
else							# If rand Line num is for Rx (node 20) line
	prevLine=$[$randLine-1]
	sed "${prevLine}q;d" $1 >> $tempFile
	sed "${randLine}q;d" $1 >> $tempFile
fi
i=$[$i+2]  # Jump Line counter by 2
done
cat $1 $tempFile > $2  	# Concatenate to output file in $2 arg
rm $tempFile



