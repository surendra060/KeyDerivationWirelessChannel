#! /bin/bash

FOLDER=$(date -I)

MYPATH=/media/surendra/DATA/MasarykCourse/MasterThesisProj/WSN/Quatization_JeeNodePgms/RSSI_app/RSSI_data/$FOLDER/

echo $MYPATH

mkdir -p $MYPATH

java -jar /usr/local/bin/JeeTool.jar -a /media/surendra/DATA/MasarykCourse/MasterThesisProj/WSN/Quatization_JeeNodePgms/RSSI_app/motesRSSI.txt -u /media/surendra/DATA/MasarykCourse/MasterThesisProj/WSN/Quatization_JeeNodePgms/RSSI_app -t &> /media/surendra/DATA/MasarykCourse/MasterThesisProj/WSN/Quatization_JeeNodePgms/RSSI_app/rssi_log.txt

#jeeTool -a /home/xnemec/motesRSSI.txt -u /home/xnemec/RSSI_app -t >> /home/xnemec/rssi_log

#timeout 1435m python /home/xnemec/serialRead.py $MYPATH >> /home/xnemec/rssi_log
#timeout 4m java -jar /home/xnemec/RSSI_parser.jar $MYPATH 
