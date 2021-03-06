##### *****************************************************************************************************

# Author - Surendra Sharma, Dec 2017
# Implementation of Postprocessing for BitString Key extraction using Wireless Signal Strength

# Generation using Aono et al Algorithm (Reference - "Wireless secret key 
# generation exploiting reactance-domain scalar response of multipath fading channels",
# in IEEE Transactions on Antennas and Propagation, vol. 53, No. 11, 2005)) 

##### *****************************************************************************************************

## Main Function to input Raw RSS files, calculate statistical params for each Row, Extract Bits for Each Row
## and calculate Performance Parametrs across all rows
nodeData_Aono = function(nodeRSSA, infileA, outfileBitStringA,outfileDataA, nodeRSSB, infileB, outfileBitStringB,outfileDataB, entropyTestFile){
  nodeRSSA =  read.table(textConnection(gsub("&", "\t", readLines(infileA))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  nodeRSSB =  read.table(textConnection(gsub("&", "\t", readLines(infileB))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  
  rowindex  = 1:nrow(nodeRSSA)
  colindex = 1:ncol(nodeRSSA)
  
  outLier_i = 10;
  rowno=0;
  AvgBitExtractionRate = 0;
  AvgLenMismatchRate = 0;
  AvgBitMismatchRate = 0;
  cat("Median : Range : BitString_Len : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring  \n", file = outfileDataA, sep = " ", append = T)
  cat("Median : Range : BitString_Len : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring\n", file = outfileDataB, sep = " ", append = T)
  
  for(rowcount in rowindex)
  {
    lenA=1; lenB=1;  bitstringA = c(rep(100,1000)) ; bitstringB = c(rep(100,1000))
    MatchBits = 0;  rowBitMismatchCount = 0; rowLenMismatch = 0;
    medValueA  = median(as.numeric(nodeRSSA[rowcount, colindex]))
    medValueB  = median(as.numeric(nodeRSSB[rowcount, colindex]))
    rngA =c(rep(0,2));rngB =c(rep(0,2));
    
    # Ranges calculation on either side of Mean / Median
    
    RngA = range(as.numeric(nodeRSSA[rowcount, colindex]))
    RngB = range(as.numeric(nodeRSSB[rowcount, colindex]))
    rngA[1] = as.numeric(sort(nodeRSSA[rowcount, colindex],partial=outLier_i)[outLier_i])
    rngA[2] = as.numeric(sort(nodeRSSA[rowcount, colindex],partial=ncol(nodeRSSA)-outLier_i)[ncol(nodeRSSA)-outLier_i])
    
    rngB[1] = as.numeric(sort(nodeRSSB[rowcount, colindex],partial=outLier_i)[outLier_i])
    rngB[2] = as.numeric(sort(nodeRSSB[rowcount, colindex],partial=ncol(nodeRSSB)-outLier_i)[ncol(nodeRSSB)-outLier_i])
    
    # Thresholds calculation
    LeftThA = medValueA - ((medValueA - rngA[1])/4)
    RightThA = medValueA + ((rngA[2] - medValueA)/2)
    
    LeftThB = medValueB - ((medValueB - rngB[1])/4)
    RightThB = medValueB + ((rngB[2] - medValueB)/2)
    
    # Bit extraction from RSS values of Matrix by comparison with Thresholds
      for(colcount in colindex){
        flagA = 1000;flagB=1000;
        if (nodeRSSA[rowcount, colcount]<LeftThA)
            {
              bitstringA[lenA] = 0
              flagA = 0
              lenA=lenA+1
            }
          else if (nodeRSSA[rowcount, colcount]>RightThA)
          {
            bitstringA[lenA] = 1
            flagA = 1
            lenA=lenA+1
          }
        
        if (nodeRSSB[rowcount, colcount]<LeftThB)
        {
          bitstringB[lenB] = 0
          flagB = 0
          lenB=lenB+1
        }
        else if (nodeRSSB[rowcount, colcount]>RightThB)
        {
          bitstringB[lenB] = 1
          flagB = 1
          lenB=lenB+1
        }
        
        if ((flagA == flagB) && flagA !=1000 && flagB!=1000)
          MatchBits = MatchBits + 1;
        
      }
    # calculation of cumulative performance params after processing of each row
    
    rowLenMismatch = ((lenA-1) -(lenB-1))
    rowBitMismatchCount = min((lenA-1), (lenB-1)) - MatchBits;
    AvgBitExtractionRate = (AvgBitExtractionRate*(rowcount-1)   + (min((lenA-1), (lenB-1))))/rowcount
    if (max((lenA-1), (lenB-1)) > 0){
      AvgLenMismatchRate = (AvgLenMismatchRate*(rowcount-1)   + (abs(rowLenMismatch)/max((lenA-1), (lenB-1))))/rowcount
    }
    if (min((lenA-1), (lenB-1)) > 0 ){
      AvgBitMismatchRate = (AvgBitMismatchRate*(rowcount-1)   + (rowBitMismatchCount/min((lenA-1), (lenB-1))))/rowcount
    }
    
    # Print the Statistical params of each row
    
    print(paste("RowA = ", rowcount, "rowLenMismatch = ", rowLenMismatch, " : rowBitMismatchCount = ",  rowBitMismatchCount, " : LenBS_A = ", lenA-1, " : LenBS_B = ", lenB-1))
    print(paste("MedianA = ",  medValueA, " :  RangeA = ", rngA[1],"-", rngA[2], "LeftThA - RightThA ", LeftThA,"<->", RightThA))
    print(paste("MedianB = ",  medValueB, " :  RangeB = ", rngB[1],"-", rngB[2], "LeftThB - RightThB ", LeftThB,"<->", RightThB))
    
    # Write extracted Binary Strings to each output BitString file
 
    cat(rbind(bitstringA[1:lenA-1]), file = outfileBitStringA, sep = " ", append = T)
    cat("\n", file = outfileBitStringA, sep = " ", append = T)
    cat(rbind(bitstringB[1:lenB-1]), file = outfileBitStringB, sep = " ", append = T)
    cat("\n", file = outfileBitStringB, sep = " ", append = T)
    
    # Write the extracted bitstring for each row with length of min(lenA, lenB) to entropy Test File for common min len bits extracted
 
     if(lenA<=lenB) {
      cat(rbind(bitstringA[1:lenA-1]), file = entropyTestFile, sep = " ", append = T)
      cat("\n", file = entropyTestFile, sep = " ", append = T)
    }
    else {
      cat(rbind(bitstringB[1:lenB-1]), file = entropyTestFile, sep = " ", append = T)
      cat("\n", file = entropyTestFile, sep = " ", append = T)
    }
    
    # Write Statistical parameter values of each row for Data file (of each rows) 
    # followed by extracted bitstring for each row
    
    cat(medValueA, file = outfileDataA, sep = " ", append = T)
    cat(":", file = outfileDataA, sep = " ", append = T)
    cat(rngA, file = outfileDataA, sep = " ", append = T)
    cat(":", file = outfileDataA, sep = " ", append = T)
    cat(lenA-1, file = outfileDataA, sep = " ", append = T)
    cat(":", file = outfileDataA, sep = " ", append = T)
    cat(rowLenMismatch, file = outfileDataA, sep = " ", append = T)
    cat(":", file = outfileDataA, sep = " ", append = T)
    cat(rowBitMismatchCount, file = outfileDataA, sep = " ", append = T)
    cat(":", file = outfileDataA, sep = " ", append = T)
    cat(MatchBits, file = outfileDataA, sep = " ", append = T)
    cat(":", file = outfileDataA, sep = " ", append = T)
    
    cat(medValueB, file = outfileDataB, sep = " ", append = T)
    cat(":", file = outfileDataB, sep = " ", append = T)
    cat(rngB, file = outfileDataB, sep = " ", append = T)
    cat(":", file = outfileDataB, sep = " ", append = T)
    cat(lenB-1, file = outfileDataB, sep = " ", append = T)
    cat(":", file = outfileDataB, sep = " ", append = T)
    cat(rowLenMismatch, file = outfileDataB, sep = " ", append = T)
    cat(":", file = outfileDataB, sep = " ", append = T)
    cat(rowBitMismatchCount, file = outfileDataB, sep = " ", append = T)
    cat(":", file = outfileDataB, sep = " ", append = T)
    cat(MatchBits, file = outfileDataB, sep = " ", append = T)
    cat(":", file = outfileDataB, sep = " ", append = T)
    
    cat(rbind(bitstringA[1:lenA-1]), file = outfileDataA, sep = " ", append = T)
    cat("\n", file = outfileDataA, sep = " ", append = T)
    cat(rbind(bitstringB[1:lenB-1]), file = outfileDataB, sep = " ", append = T)
    cat("\n", file = outfileDataB, sep = " ", append = T)
  }
  
  # print in Rstudio console - Performance parameter values of entire file (all Rows) at the end of both Data Files

    print(paste("AvgLenMismatchRate  = ", round(AvgLenMismatchRate,4) , " : AvgBitMismatchRate = ",  round(AvgBitMismatchRate,4), "AvgBitExtractionRate", round(AvgBitExtractionRate,4)))

  
  # Write Performance parameter values of entire file (all Rows) at the end of both Data Files

  cat("\n\n AvgLenMismatchRate : ", file = outfileDataA, sep = " ", append = T)
  cat(round(AvgLenMismatchRate,4), file = outfileDataA, sep = " ", append = T)
  cat("\n AvgBitMismatchRate", file = outfileDataA, sep = " ", append = T)
  cat(round(AvgBitMismatchRate,4) , file = outfileDataA, sep = " ", append = T)
  cat("\n AvgBitExtractionRate", file = outfileDataA, sep = " ", append = T)
  cat(round(AvgBitExtractionRate,4), file = outfileDataA, sep = " ", append = T)
  
  cat("\n\n AvgLenMismatchRate : ", file = outfileDataB, sep = " ", append = T)
  cat(round(AvgLenMismatchRate,4), file = outfileDataB, sep = " ", append = T)
  cat("\n AvgBitMismatchRate : ", file = outfileDataB, sep = " ", append = T)
  cat(round(AvgBitMismatchRate,4), file = outfileDataB, sep = " ", append = T)
  cat("\n AvgBitExtractionRate : ", file = outfileDataB, sep = " ", append = T)
  cat(round(AvgBitExtractionRate,4), file = outfileDataB, sep = " ", append = T)
}

# Remove Old Files
system("rm nodeA_*", wait=FALSE)
system("rm nodeB_*", wait=FALSE)
system("rm nodeAB_*", wait=FALSE)



# Call main Fucntion to calculate statistical values and extract bitstrings and other Performance Metric Params
# Input to fucntion are two data files nodeA.txt and nodeB.txt, 
# Output Files - nodeA_BS_data_XXXX.txt and nodeB_BS_data_XXXX.txt for Bitstrings and all statistical values
#                nodeA_BS_XXXX.txt and nodeB_BS_XXXX.txt  for only Bitstrings extracted
#                nodeAB_XXXX_Entropy.txt  for only common length  of values min(La, Lb) (1s and 0's) - for ENtropy Tests


nodeData_Aono(A, "nodeA.txt", "nodeA_BS_Aono.txt", "nodeA_BS_data_Aono.txt", B, "nodeB.txt", "nodeB_BS_Aono.txt", "nodeB_BS_data_Aono.txt","nodeAB_Aono_Entropy.txt")
print(paste(" Aono - TX-#0's : TX - #1's "))
NumZero <- system("grep -o '0' nodeA_BS_Aono.txt | wc -l", wait=TRUE)
NumOne <- system("grep -o '1' nodeA_BS_Aono.txt | wc -l", wait=TRUE)
print(paste(NumZero , "<->", NumOne ))




