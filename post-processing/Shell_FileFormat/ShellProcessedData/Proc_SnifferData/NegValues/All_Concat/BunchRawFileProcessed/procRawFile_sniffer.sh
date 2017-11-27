#!/bin/bash

#inFile=$1
#outFile_TX="Node10.txt"
#outFile_RX="Node20.txt"
tempFileRX_TX="temp20_10.txt"
tempFileTX_RX="temp10_20.txt"
tempFileEV_TX="temp31_10.txt"
tempFileEV_RX="temp31_20.txt"


finalRX_TX="nodeB_A.txt"
finalTX_RX="nodeA_B.txt"
finalEV_TX="nodeE_A.txt"
finalEV_RX="nodeE_B.txt"

tempFile="temp"

####### Processing for 20_10 Lines  ##########
sed -n '/20_10 :/p' $1 > $tempFile
sed -i 's/;0;/;-72;/g' $tempFile
sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileRX_TX  #insert star in every alternate line
sed  -i 's/\&\&\r\*[2][0][_][1][0]\s\:\s//g' $tempFileRX_TX  # stictch two concescutive lines together
mv $tempFileRX_TX $tempFile
sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileRX_TX  #insert star in every alternate line
sed  -i 's/\&\&\r\*[2][0][_][1][0]\s\:\s//g' $tempFileRX_TX  # stictch two concescutive lines together
sed -i 's/[2][0][_][1][0]\s\:\s//g' $tempFileRX_TX # remove 10 : from middile of joined line
sed -i 's/\&\&/\&/g' $tempFileRX_TX  # replcae && -> &
sed -r '/^.{,4001}$/d' $tempFileRX_TX > $finalRX_TX # remove lines < 4000 characters(4x1000values(-78;)
#sed -i 's/-//g' nodeA.txt # remove Negative sign of all values

####### Processing for 10_20 Lines  ##########

sed -n '/10_20 :/p' $1 > $tempFile
sed -i 's/\ 0;/\ -70;/g' $tempFile
sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileTX_RX
sed  -i 's/\&\&\r\*[1][0]_[2][0]\s\:\s//g' $tempFileTX_RX
mv $tempFileTX_RX $tempFile
sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileTX_RX
sed  -i 's/\&\&\r\*[1][0]_[2][0]\s\:\s//g' $tempFileTX_RX
sed -i 's/[1][0]_[2][0]\s\:\s//g' $tempFileTX_RX
sed -i 's/\&\&/\&/g' $tempFileTX_RX
sed -r '/^.{,4001}$/d' $tempFileTX_RX > $finalTX_RX  # remove lines < 4000 characters(4x1000values(-78;)

####### Processing for 31_10 Lines  ##########
sed -n '/31_10 :/p' $1 > $tempFile
sed -i 's/;0;/;-66;/g' $tempFile
sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileEV_TX  #insert star in every alternate line
sed  -i 's/\&\&\r\*[3][1][_][1][0]\s\:\s//g' $tempFileEV_TX  # stictch two concescutive lines together
mv $tempFileEV_TX $tempFile
sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileEV_TX  #insert star in every alternate line
sed  -i 's/\&\&\r\*[3][1][_][1][0]\s\:\s//g' $tempFileEV_TX  # stictch two concescutive lines together
sed -i 's/[3][1][_][1][0]\s\:\s//g' $tempFileEV_TX # remove 10 : from middile of joined line
sed -i 's/\&\&/\&/g' $tempFileEV_TX  # replace && -> &
sed -r '/^.{,4001}$/d' $tempFileEV_TX > $finalEV_TX # remove lines < 4000 characters(4x1000values(-78;)
#sed -i 's/-//g' nodeA.txt # remove Negative sign of all values

####### Processing for 31_20 Lines  ##########

sed -n '/31_20 :/p' $1 > $tempFile
sed -i 's/;0;/;-67;/g' $tempFile
sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileEV_RX
sed  -i 's/\&\&\r\*[3][1]_[2][0]\s\:\s//g' $tempFileEV_RX
mv $tempFileEV_RX $tempFile
sed  '$!N;s/\n/\*/g' $tempFile | tee $tempFileEV_RX
sed  -i 's/\&\&\r\*[3][1]_[2][0]\s\:\s//g' $tempFileEV_RX
sed -i 's/[3][1]_[2][0]\s\:\s//g' $tempFileEV_RX
sed -i 's/\&\&/\&/g' $tempFileEV_RX
sed -r '/^.{,4001}$/d' $tempFileEV_RX > $finalEV_RX  # remove lines < 4000 characters(4x1000values(-78;)

