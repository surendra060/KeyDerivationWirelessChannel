#!/bin/bash
echo "\n **************************************************\n"> EntropyTestResultsMathur.txt
echo "Entropy Test for Mathur 0.5 SD Threshold et al ----------" >> EntropyTestResultsMathur.txt
echo "**************************************************">> EntropyTestResultsMathur.txt

echo $(python iid_main.py entropyMathurHalfSD.bin 8) >> EntropyTestResultsMathur.txt
echo "...............................................">> EntropyTestResultsMathur.txt
echo $(python noniid_main.py  -v entropyMathurHalfSD.bin 8) >> EntropyTestResultsMathur.txt

echo "\n **************************************************\n">> EntropyTestResultsMathur.txt
echo "Entropy Test for Mathur et al  1 SD Threshold ----------" >> EntropyTestResultsMathur.txt
echo "**************************************************">> EntropyTestResultsMathur.txt

echo $(python iid_main.py entropyMathurHalfSD.bin 8) >> EntropyTestResultsMathur.txt
echo "...............................................">> EntropyTestResultsMathur.txt
echo $(python noniid_main.py  -v entropyMathurHalfSD.bin 8) >> EntropyTestResultsMathur.txt


echo "\n **************************************************\n">> EntropyTestResultsMathur.txt
echo "Entropy Test for Mathur et al  Range Ratio SD Threshold ----------" >> EntropyTestResultsMathur.txt
echo "**************************************************">> EntropyTestResultsMathur.txt

echo $(python iid_main.py entropyMathurRangeRatio.bin 8) >> EntropyTestResultsMathur.txt
echo "...............................................">> EntropyTestResultsMathur.txt
echo $(python noniid_main.py -v entropyMathurRangeRatio.bin 8) >> EntropyTestResultsMathur.txt

echo "\n **************************************************\n">> EntropyTestResultsMathur.txt
echo "----END OF MATHUR VARIATION TESTS----------" >> EntropyTestResultsMathur.tx


