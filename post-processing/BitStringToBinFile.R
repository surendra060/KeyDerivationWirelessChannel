split8char <- function(x) {
  m <- matrix(strsplit(x, '')[[1]], nrow=8)
  apply(m, 2, paste, collapse='')
}

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

#entropyTestFile("nodeAB_Aono_Entropy.txt" , "entropyAono.bin")
#entropyTestFile("nodeAB_Tope_Entropy.txt" , "entropyTope.bin")
entropyTestFile("nodeAB_Mathur_Entropy.txt" , "entropyMathur.bin")
#entropyTestFile("nodeAB_Sadzadi_Entropy.txt" , "entropySadzadi.bin")



