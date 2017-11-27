#!/bin/bash
echo "\n **************************************************\n"> EntropyTestResultsSadzadi3SD.txt
echo "Entropy Test for Sadzadi 3 SD Threshold et al ----------" >> EntropyTestResultsSadzadi3SD.txt
echo "**************************************************">> EntropyTestResultsSadzadi3SD.txt

#echo $(python iid_main.py entropySadzadi.bin 8) >> EntropyTestResultsSadzadi3SD.txt
#echo "...............................................">> EntropyTestResultsSadzadi3SD.txt
echo $(python noniid_main.py  -v entropySadzadi.bin 8) >> EntropyTestResultsSadzadi3SD.txt

echo "\n **************************************************\n">> EntropyTestResultsSadzadi3SD.txt
echo "----END OF Sadzadi Entropy TESTS----------" >> EntropyTestResultsSadzadi3SD.tx


