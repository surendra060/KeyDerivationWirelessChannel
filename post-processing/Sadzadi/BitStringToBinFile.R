##### *****************************************************************************************************

# Author - Surendra Sharma, Dec 2017
# R program file to convert Binary Bitstring of arbitrary length to Bin data file 
# Each Binary Value in string is a bit in the Binary Data file (i.e. each byte has 
# 8 consecutive extracted Bit string values)

##### *****************************************************************************************************

# Auxilary function used in Main function to split the input Matrix row values in blocks of 8 values 
split8char <- function(x) {
  m <- matrix(strsplit(x, '')[[1]], nrow=8)
  apply(m, 2, paste, collapse='')
}

# Main function to convert input Bitstring files (text) to binary Output data (.bin) file. 
entropyTestFile = function(infile, outfile){
  
#data_line = read.table("nodeA_BS_Mathur.txt", header=FALSE, sep =" ", fill = TRUE, stringsAsFactors = FALSE)
data_line = readLines(infile)
myString<-paste(unlist(strsplit(data_line," ")),collapse="") 
matA = split8char(myString)

fileout = file(outfile, "wb")
#print((data_line[1]))
print(length(matA))
print(paste0(matA[7552], " - ", matA[7553], " - ", matA[7554], " - ", matA[7555]))
i=1; 
Range=100000000/8;
numBytes = floor(length(matA)/8)
  if(length(matA) < Range){
      numBytes = length(matA)
  }    

 if(length(matA) >= Range) { 
   numBytes = Range  
 } 
 for (i in 1:numBytes){
    ch = strtoi(matA[i], base =2)
   #print(paste0(matA[i], "---", ch))
   writeBin(ch, fileout, size=1, endian = .Platform$endian, useBytes = FALSE)
 }
#print(data_line[140])
print(numBytes)
}


## Main Function Call with appropriate Input and Output Filenames

entropyTestFile("nodeAB_Sadzadi_Entropy.txt" , "entropySadzadi.bin")



