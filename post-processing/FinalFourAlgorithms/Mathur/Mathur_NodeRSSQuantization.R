##**************************************** MATHUR *********************************************************

## BitSTring from Mathur
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
   
   AvgBitExtractionRate = 0;
   AvgLenMismatchRate = 0;
   AvgBitMismatchRate = 0;
   
   cat("MeanA : StdDevB : alpha : RangeA :  BitString_LenA : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring  \n", file = outfileDataA, sep = " ", append = T)
   cat("MeanB : StdDevB : alpha : RangeB : BitString_LenB : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring  \n", file = outfileDataB, sep = " ", append = T)
   
  
  for(rowcount in rowindex)
  {
    lenA=1; lenB=1;  bitstringA = c(rep(100,1000)) ; bitstringB = c(rep(100,1000))
    MatchBits = 0;  rowBitMismatchCount = 0; rowLenMismatch = 0;

    meanValueA = mean(nodeRSSmatA[rowcount, rowindex])
    stdDevA = sd(nodeRSSmatA[rowcount, rowindex])
    meanValueB = mean(nodeRSSmatB[rowcount, rowindex])
    stdDevB = sd(nodeRSSmatB[rowcount, rowindex])
    rngA =c(rep(0,2));rngB =c(rep(0,2));
    # rngA = range(nodeRSSmatA[rowcount, rowindex])
    # rngB = range(nodeRSSmatB[rowcount, rowindex])
    rngA[1] = as.numeric(sort(nodeRSSA[rowcount, 1:1000],partial=outLier_i)[outLier_i])
    rngA[2] = as.numeric(sort(nodeRSSA[rowcount, 1:1000],partial=ncol(nodeRSSA)-outLier_i)[ncol(nodeRSSA)-outLier_i])
    
    rngB[1] = as.numeric(sort(nodeRSSB[rowcount, 1:1000],partial=outLier_i)[outLier_i])
    rngB[2] = as.numeric(sort(nodeRSSB[rowcount, 1:1000],partial=ncol(nodeRSSB)-outLier_i)[ncol(nodeRSSB)-outLier_i])
    
    # LfactorA = min((abs(meanValueA - rngA[1]))/abs(rngA[2] - rngA[1]), 1)
    # RfactorA = (abs(rngA[2] - meanValueA)/abs(rngA[2] - rngA[1]))/4
    # LfactorB = min(abs(meanValueB - rngB[1])/abs(rngB[2] - rngB[1]), 1)
    # RfactorB = (abs(rngA[2] - meanValueB)/abs(rngB[2] - rngB[1]))/4
    # 
    LfactorA = (abs(meanValueA - rngA[1]))/abs(rngA[2] - rngA[1])
    RfactorA = (abs(rngA[2] - meanValueA)/abs(rngA[2] - rngA[1]))/4
    LfactorB = abs(meanValueB - rngB[1])/abs(rngB[2] - rngB[1])
    RfactorB = (abs(rngA[2] - meanValueB)/abs(rngB[2] - rngB[1]))/4
    
    
    #print(paste(meanValueA, stdDevA, alpha, rngA[1], rngA[2]))
   
    for(colcount in colindex){
      flagA = 1000;flagB=1000;
      if (nodeRSSmatA[rowcount, colcount]<floor(meanValueA-(LfactorA*alpha*stdDevA)))
      {
        bitstringA[lenA] = 0
        flagA = 0
        lenA=lenA+1
      }
      else if (nodeRSSmatA[rowcount, colcount]>ceiling(meanValueA+(RfactorA*alpha*stdDevA)))
      {
        bitstringA[lenA] = 1
        flagA = 1
        lenA=lenA+1
      }
     
      
       if (nodeRSSmatB[rowcount, colcount]<floor(meanValueB-(LfactorB*alpha*stdDevB)))
      {
        bitstringB[lenB] = 0
        flagB = 0
        lenB=lenB+1
      }
      else if (nodeRSSmatB[rowcount, colcount]>ceiling(meanValueB+(RfactorB*alpha*stdDevB)))
      {
        bitstringB[lenB] = 1
        flagB = 1
        lenB=lenB+1
      }
      
      if ((flagA == flagB) && flagA !=1000 && flagB!=1000)
        MatchBits = MatchBits + 1;
    }
    
    rowLenMismatch = ((lenA-1) -(lenB-1))
    rowBitMismatchCount = min((lenA-1), (lenB-1)) - MatchBits;
    
    
    AvgBitExtractionRate = (AvgBitExtractionRate*(rowcount-1)   + (min((lenA-1), (lenB-1))))/rowcount
    if (max((lenA-1), (lenB-1)) > 0){
    AvgLenMismatchRate = (AvgLenMismatchRate*(rowcount-1)   + (abs(rowLenMismatch)/max((lenA-1), (lenB-1))))/rowcount
    }
    if (min((lenA-1), (lenB-1)) > 0 ){
    AvgBitMismatchRate = (AvgBitMismatchRate*(rowcount-1)   + (rowBitMismatchCount/min((lenA-1), (lenB-1))))/rowcount
    }
    
    #print(paste("Mean is ", round(meanValue,2)," Lth -Rth = ", (meanValue-(Lfactor*alpha*stdDev)), " - ", (meanValue+(Rfactor*alpha*stdDev)),   "  Length of BitString = ", len-1))
    print(paste("RowA = ", rowcount, "rowLenMismatch = ", rowLenMismatch, " : rowBitMismatchCount = ",  rowBitMismatchCount, " : LenBS_A = ", lenA-1, " : LenBS_B = ", lenB-1))
    print(paste("meanValueA = ", round(meanValueA,2), " : stdDevA = ", round(stdDevA,2), "rngA[1] <-> rngA[2]  = ", rngA[1], "<-> ", rngA[2], "Lth -Rth = ", round(meanValueA-(LfactorA*alpha*stdDevA)), " <-> ", round(meanValueA+(RfactorA*alpha*stdDevA)) ));
    print(paste("meanValueB = ", round(meanValueB,2), " : stdDevA = ", round(stdDevB,2), "rngA[1] <-> rngA[2]  = ", rngB[1], "<-> ", rngB[2], "Lth -Rth = ", round(meanValueB-(LfactorB*alpha*stdDevB)), " <-> ", round(meanValueB+(RfactorB*alpha*stdDevB)) ));
    
    cat(rbind(bitstringA[1:lenA-1]), file = outfileBitStringA, sep = " ", append = T)
    cat("\n", file = outfileBitStringA, sep = " ", append = T)
    cat(rbind(bitstringB[1:lenB-1]), file = outfileBitStringB, sep = " ", append = T)
    cat("\n", file = outfileBitStringB, sep = " ", append = T)
    
    if(lenA<=lenB) {
      cat(rbind(bitstringA[1:lenA-1]), file = entropyTestFile, sep = " ", append = T)
      cat("\n", file = entropyTestFile, sep = " ", append = T)
    }
    else {
      cat(rbind(bitstringB[1:lenB-1]), file = entropyTestFile, sep = " ", append = T)
      cat("\n", file = entropyTestFile, sep = " ", append = T)
    }

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
  
   print(paste("AvgLenMismatchRate  = ", round(AvgLenMismatchRate,4) , " : AvgBitMismatchRate = ",  round(AvgBitMismatchRate,4), "AvgBitExtractionRate", round(AvgBitExtractionRate,4)))
   
   cat("\n\n AvgLenMismatchRate : ", file = outfileDataA, sep = " ", append = T)
   cat(round(AvgLenMismatchRate,4), file = outfileDataA, sep = " ", append = T)
   cat("\n AvgBitMismatchRate", file = outfileDataA, sep = " ", append = T)
   cat(round(AvgBitMismatchRate,4) , file = outfileDataA, sep = " ", append = T)
   cat("\n AvgBitExtractionRate", file = outfileDataA, sep = " ", append = T)
   cat(round(AvgBitExtractionRate,4), file = outfileDataA, sep = " ", append = T)
   cat("\n Params Used : ", file = outfileDataA, sep = " ", append = T)
   cat(round(AvgBitExtractionRate,4), file = outfileDataA, sep = " ", append = T)
   
   
   cat("\n\n AvgLenMismatchRate : ", file = outfileDataB, sep = " ", append = T)
   cat(round(AvgLenMismatchRate,4), file = outfileDataB, sep = " ", append = T)
   cat("\n AvgBitMismatchRate : ", file = outfileDataB, sep = " ", append = T)
   cat(round(AvgBitMismatchRate,4), file = outfileDataB, sep = " ", append = T)
   cat("\n AvgBitExtractionRate : ", file = outfileDataB, sep = " ", append = T)
   cat(round(AvgBitExtractionRate,4), file = outfileDataB, sep = " ", append = T)
   
}

system("rm nodeA_*", wait=FALSE)
system("rm nodeB_*", wait=FALSE)
system("rm nodeAB_*", wait=FALSE)


alpha = 1

nodeData_Mathur(A, "nodeA.txt", "nodeA_BS_Mathur.txt", "nodeA_BS_data_Mathur.txt", B, "nodeB.txt", "nodeB_BS_Mathur.txt", "nodeB_BS_data_Mathur.txt", "nodeAB_Mathur_Entropy.txt")
#nodeData_Mathur(B, "nodeB.txt", "nodeB_BS_Mathur.txt", "nodeB_BS_data_Mathur.txt", alpha)
print(paste(" Mathur - TX-#0's : TX - #1's "))
print(paste(system("grep -o '0' nodeA_BS_Mathur.txt | wc -l", wait=TRUE), "<->", system("grep -o '1' nodeA_BS_Mathur.txt | wc -l", wait=TRUE)))




