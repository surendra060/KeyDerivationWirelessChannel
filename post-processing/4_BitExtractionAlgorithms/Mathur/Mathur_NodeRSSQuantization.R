##### *****************************************************************************************************

# Author - Surendra Sharma, Dec 2017
# Implementation of Postprocessing for BitString Key extraction using Wireless Signal Strength

# Generation using Mathur et al Algorithm (Reference - ""Radio-telepathy: extracting a secret key
# from an unauthenticated wireless channel", in Proceedings of the 14th ACM International 
# Conference on Mobile Computing and Networking (MobiCom), pp. 128–139, 2008)) 

##### *****************************************************************************************************

## Main Function to input Raw RSS files, calculate statistical params for each Row, Extract Bits for Each Row
## and calculate Performance Parametrs across all rows

nodeData_Mathur = function(nodeRSSA, infileA, outfileBitStringA,outfileDataA, nodeRSSB, infileB, outfileBitStringB,outfileDataB, entropyTestFile){
  nodeRSSA =  read.table(textConnection(gsub("&", "\t", readLines(infileA))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  nodeRSSB =  read.table(textConnection(gsub("&", "\t", readLines(infileB))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  
  nodeRSSmatA = data.matrix(nodeRSSA)
  nodeRSSmatB = data.matrix(nodeRSSB)
 
   i=1
   lenA = 1;lenB = 1;outLier_i = 10;
   rowno=0;
   
   rowindex  = 1:nrow(nodeRSSA)
   colindex  = 1:ncol(nodeRSSA)
   
   # print(paste0( nrow(nodeRSSA)))
   # print(paste0( ncol(nodeRSSA)))
   
   AvgBitExtractionRate = 0;
   AvgLenMismatchRate = 0;
   AvgBitMismatchRate = 0;
   
   cat("MeanA : StdDevB : alpha : RangeA :  BitString_LenA : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring  \n", file = outfileDataA, sep = " ", append = T)
   cat("MeanB : StdDevB : alpha : RangeB : BitString_LenB : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring  \n", file = outfileDataB, sep = " ", append = T)
   
  
  for(rowcount in rowindex)
  {
    lenA=1; lenB=1;  bitstringA = c(rep(100,1000)) ; bitstringB = c(rep(100,1000))
    MatchBits = 0;  rowBitMismatchCount = 0; rowLenMismatch = 0;
    meanValueA = mean(nodeRSSmatA[rowcount, colindex])
    stdDevA = sd(nodeRSSmatA[rowcount, colindex])
    meanValueB = mean(nodeRSSmatB[rowcount, colindex])
    stdDevB = sd(nodeRSSmatB[rowcount, colindex])
    
    # Ranges calculation on either side of Mean / Median
    
    rngA =c(rep(0,2));rngB =c(rep(0,2));
    # rngA = range(nodeRSSmatA[rowcount, colindex])
    # rngB = range(nodeRSSmatB[rowcount, colindex])
    rngA[1] = as.numeric(sort(nodeRSSA[rowcount, colindex],partial=outLier_i)[outLier_i])
    rngA[2] = as.numeric(sort(nodeRSSA[rowcount, colindex],partial=ncol(nodeRSSA)-outLier_i)[ncol(nodeRSSA)-outLier_i])
    
    rngB[1] = as.numeric(sort(nodeRSSB[rowcount, colindex],partial=outLier_i)[outLier_i])
    rngB[2] = as.numeric(sort(nodeRSSB[rowcount, colindex],partial=ncol(nodeRSSB)-outLier_i)[ncol(nodeRSSB)-outLier_i])
    
    # Multiple/Alternate ways to calculate Lfactor and Rfactor for determining Lth and Rth
    # Assymetrical thresholds based on range ratio on either sides of Mean
    
    # LfactorA = min((abs(meanValueA - rngA[1]))/abs(rngA[2] - rngA[1]), 1)
    # RfactorA = (abs(rngA[2] - meanValueA)/abs(rngA[2] - rngA[1]))/4
    # LfactorB = min(abs(meanValueB - rngB[1])/abs(rngB[2] - rngB[1]), 1)
    # RfactorB = (abs(rngA[2] - meanValueB)/abs(rngB[2] - rngB[1]))/4
    # 
    # LfactorA = (abs(meanValueA - rngA[1]))/(4*abs(rngA[2] - rngA[1]))
    # RfactorA = (abs(rngA[2] - meanValueA)/abs(rngA[2] - rngA[1]))
    # LfactorB = abs(meanValueB - rngB[1])/(4*abs(rngB[2] - rngB[1]))
    # RfactorB = (abs(rngA[2] - meanValueB)/abs(rngB[2] - rngB[1]))
    # 
    LfactorA = 1
    RfactorA = 1
    LfactorB = 1
    RfactorB = 1
    
    
    #print(paste(meanValueA, stdDevA, alpha, rngA[1], rngA[2]))
   
    for(colcount in colindex){
      flagA = 1000;flagB=1000;
      if (nodeRSSmatA[rowcount, colcount] < (meanValueA-(LfactorA*alpha*stdDevA)))
      #if (nodeRSSmatA[rowcount, colcount]<floor(meanValueA-(LfactorA*alpha*stdDevA)))
          
      {
        bitstringA[lenA] = 0
        flagA = 0
        lenA=lenA+1
      }
      else if (nodeRSSmatA[rowcount, colcount]> (meanValueA+(RfactorA*alpha*stdDevA)))
      #else if (nodeRSSmatA[rowcount, colcount]>ceiling(meanValueA+(RfactorA*alpha*stdDevA)))
          
      {
        bitstringA[lenA] = 1
        flagA = 1
        lenA=lenA+1
      }
     
      # Bit extraction from RSS values of Matrix by comparison with Thresholds
      
       if (nodeRSSmatB[rowcount, colcount]< (meanValueB-(LfactorB*alpha*stdDevB)))
       # if (nodeRSSmatB[rowcount, colcount]<floor(meanValueB-(LfactorB*alpha*stdDevB)))
           
      {
        bitstringB[lenB] = 0
        flagB = 0
        lenB=lenB+1
      }
      else if (nodeRSSmatB[rowcount, colcount] > (meanValueB+(RfactorB*alpha*stdDevB)))
      #else if (nodeRSSmatB[rowcount, colcount]>ceiling(meanValueB+(RfactorB*alpha*stdDevB)))
          
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
    print(paste("meanValueA = ", round(meanValueA,2), " : stdDevA = ", round(stdDevA,2), "rngA[1] <-> rngA[2]  = ", rngA[1], "<-> ", rngA[2], "Lth -Rth = ", round(meanValueA-(LfactorA*alpha*stdDevA)), " <-> ", round(meanValueA+(RfactorA*alpha*stdDevA)) ));
    print(paste("meanValueB = ", round(meanValueB,2), " : stdDevA = ", round(stdDevB,2), "rngA[1] <-> rngA[2]  = ", rngB[1], "<-> ", rngB[2], "Lth -Rth = ", round(meanValueB-(LfactorB*alpha*stdDevB)), " <-> ", round(meanValueB+(RfactorB*alpha*stdDevB)) ));
    
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
    
    cat(round(meanValueA,2), file = outfileDataA, sep = " ", append = T)
    cat(":", file = outfileDataA, sep = " ", append = T)
    cat(round(stdDevA,2), file = outfileDataA, sep = " ", append = T)
    cat(":", file = outfileDataA, sep = " ", append = T)
    cat(alpha, file = outfileDataA, sep = " ", append = T)
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

    cat(round(meanValueB,2), file = outfileDataB, sep = " ", append = T)
    cat(":", file = outfileDataB, sep = " ", append = T)
    cat(round(stdDevB,2), file = outfileDataB, sep = " ", append = T)
    cat(":", file = outfileDataB, sep = " ", append = T)
    cat(alpha, file = outfileDataB, sep = " ", append = T)
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
   cat("\n Params Used : ", file = outfileDataA, sep = " ", append = T)
   cat(alpha, file = outfileDataA, sep = " ", append = T)
   
   
   cat("\n\n AvgLenMismatchRate : ", file = outfileDataB, sep = " ", append = T)
   cat(round(AvgLenMismatchRate,4), file = outfileDataB, sep = " ", append = T)
   cat("\n AvgBitMismatchRate : ", file = outfileDataB, sep = " ", append = T)
   cat(round(AvgBitMismatchRate,4), file = outfileDataB, sep = " ", append = T)
   cat("\n AvgBitExtractionRate : ", file = outfileDataB, sep = " ", append = T)
   cat(round(AvgBitExtractionRate,4), file = outfileDataB, sep = " ", append = T)
   cat("\n Params Used : ", file = outfileDataB, sep = " ", append = T)
   cat(alpha, file = outfileDataB, sep = " ", append = T)
   
}

# Remove Old Files

system("rm nodeA_*", wait=FALSE)
system("rm nodeB_*", wait=FALSE)
system("rm nodeAB_*", wait=FALSE)

#  Input param Alpha defines the multiplication factor for StdDev fors setting threshold away from the Mean 
alpha = 1

# Call main Fucntion to calculate statistical values and extract bitstrings and other Performance Metric Params
# Input to fucntion are two data files nodeA.txt and nodeB.txt, 
# Output Files - nodeA_BS_data_XXXX.txt and nodeB_BS_data_XXXX.txt for Bitstrings and all statistical values
#                nodeA_BS_XXXX.txt and nodeB_BS_XXXX.txt  for only Bitstrings extracted
#                nodeAB_XXXX_Entropy.txt  for only common length  of values min(La, Lb) (1s and 0's) - for ENtropy Tests



nodeData_Mathur(A, "nodeA.txt", "nodeA_BS_Mathur.txt", "nodeA_BS_data_Mathur.txt", B, "nodeB.txt", "nodeB_BS_Mathur.txt", "nodeB_BS_data_Mathur.txt", "nodeAB_Mathur_Entropy.txt")
print(paste(" Mathur - TX-#0's : TX - #1's "))
print(paste(system("grep -o '0' nodeA_BS_Mathur.txt | wc -l", wait=TRUE), "<->", system("grep -o '1' nodeA_BS_Mathur.txt | wc -l", wait=TRUE)))




