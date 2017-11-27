#!/bin/bash

#inFile=$1
#outFile_TX="Node10.txt"
#outFile_RX="Node20.txt"
tempFileTX="tempTx.txt"
tempFileRX="tempRx.txt"
finalTX="nodeA.txt"
finalRX="nodeB.txt"
tempFile="temp"
pattern=" 10 : "
replacement=";"

sed -n '/10 :/p' $1 > $tempFile
cat $tempFile | tee  $tempFileTX

sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileTX  # extract only TX node(10) data
sed  -i 's/\&\&\r\*[1][0]\s\:\s//g' $tempFileTX  # stictch two concescutive lines together
sed -i 's/[1][0]\s\:\s//g' $tempFileTX # remove 10 : from middile of joined line
sed -i 's/\&\&/\&/g' $tempFileTX  # replcae && -> &
sed -r '/^.{,4001}$/d' $tempFileTX > $finalTX # remove lines < 4000 characters(4x1000values(-78;)
sed -i 's/-//g' nodeA.txt # remove Negative sign of all values

sed -n '/20 :/p' $1 > $tempFile
cat $tempFile | tee  $tempFileRX

sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileRX
sed  -i 's/\&\&\r\*[2][0]\s\:\s//g' $tempFileRX
sed -i 's/[2][0]\s\:\s//g' $tempFileRX
sed -i 's/\&\&/\&/g' $tempFileRX
sed -r '/^.{,4001}$/d' $tempFileRX > $finalRX  # remove lines < 4000 characters(4x1000values(-78;)
sed -i 's/-//g' nodeB.txt 

