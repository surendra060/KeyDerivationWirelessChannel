#!/bin/bash

echo "Running Entropy Tests for RSSI Bit Extraction Data" > EntropyTestResults.txt
echo "\n **************************************************\n">> EntropyTestResults.txt
echo "Entropy Test for Aono et al ----------" >> EntropyTestResults.txt
echo "**************************************************">> EntropyTestResults.txt

echo $(python iid_main.py entropyAono.bin 8) >> EntropyTestResults.txt
echo "...............................................">> EntropyTestResults.txt
echo $(python noniid_main.py -v entropyAono.bin 8) >> EntropyTestResults.txt

echo "\n **************************************************\n">> EntropyTestResults.txt
echo "Entropy Test for Tope et al ----------" >> EntropyTestResults.txt
echo "**************************************************">> EntropyTestResults.txt

echo $(python iid_main.py entropyTope.bin 8) >> EntropyTestResults.txt
echo "...............................................">> EntropyTestResults.txt
echo $(python noniid_main.py -v entropyTope.bin 8) >> EntropyTestResults.txt

echo "\n **************************************************\n">> EntropyTestResults.txt
echo "Entropy Test for Mathur et al ----------" >> EntropyTestResults.txt
echo "**************************************************">> EntropyTestResults.txt

echo $(python iid_main.py entropyMathur.bin 8) >> EntropyTestResults.txt
echo "...............................................">> EntropyTestResults.txt
echo $(python noniid_main.py -v  entropyMathur.bin 8) >> EntropyTestResults.txt

echo "\n **************************************************\n">> EntropyTestResults.txt
echo "Entropy Test for Sadzadi et al ----------" >> EntropyTestResults.txt
echo "**************************************************">> EntropyTestResults.txt

echo $(python iid_main.py entropySadzadi.bin 8) >> EntropyTestResults.txt
echo "...............................................">> EntropyTestResults.txt
echo $(python noniid_main.py -v entropySadzadi.bin 8) >> EntropyTestResults.txt

echo "\n **************************************************\n">> EntropyTestResults.txt
echo "Entropy Test for Sadzadi Median *** PROPOSED----------" >> EntropyTestResults.txt
echo "**************************************************">> EntropyTestResults.txt

echo $(python iid_main.py -v entropySadzadiMedian.bin 8) >> EntropyTestResults.txt
echo "...............................................">> EntropyTestResults.txt
echo $(python noniid_main.py -v entropySadzadiMedian.bin 8) >> EntropyTestResults.txt


