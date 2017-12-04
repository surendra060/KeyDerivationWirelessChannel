##### ************************* SADZADI ****************************************************************

# BitString Generation using Sadzadi et al
nodeData_Sadzadi = function(nodeRSSA, infileA, outfileBitStringA,outfileDataA, nodeRSSB, infileB, outfileBitStringB,outfileDataB, entropyTestFile){
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
    medValueA  = median(as.numeric(nodeRSSA[rowcount, 1:1000]))
    medValueB  = median(as.numeric(nodeRSSB[rowcount, 1:1000]))
    rngA =c(rep(0,2));rngB =c(rep(0,2));
    
     RngA = range(as.numeric(nodeRSSA[rowcount, 1:1000]))
     RngB = range(as.numeric(nodeRSSB[rowcount, 1:1000]))
    rngA[1] = as.numeric(sort(nodeRSSA[rowcount, 1:1000],partial=outLier_i)[outLier_i])
    rngA[2] = as.numeric(sort(nodeRSSA[rowcount, 1:1000],partial=ncol(nodeRSSA)-outLier_i)[ncol(nodeRSSA)-outLier_i])
    
    rngB[1] = as.numeric(sort(nodeRSSB[rowcount, 1:1000],partial=outLier_i)[outLier_i])
    rngB[2] = as.numeric(sort(nodeRSSB[rowcount, 1:1000],partial=ncol(nodeRSSB)-outLier_i)[ncol(nodeRSSB)-outLier_i])
    
   # Change of LeftTh and RightTh values bwetween Aono and Sadzadi ;
    # LeftThA = medValueA - ((medValueA - rngA[1])/2)
    # RightThA = medValueA + ((rngA[2] - medValueA)/4)
    # LeftThB = medValueB - ((medValueB - rngB[1])/2)
    # RightThB = medValueB + ((rngB[2] - medValueB)/4)
    LeftThA = medValueA 
    RightThA = medValueA 
    LeftThB = medValueB 
    RightThB = medValueB
    
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
    
    rowLenMismatch = ((lenA-1) -(lenB-1))
    rowBitMismatchCount = min((lenA-1), (lenB-1)) - MatchBits;
    AvgBitExtractionRate = (AvgBitExtractionRate*(rowcount-1)   + (min((lenA-1), (lenB-1))))/rowcount
    if (max((lenA-1), (lenB-1)) > 0){
      AvgLenMismatchRate = (AvgLenMismatchRate*(rowcount-1)   + (abs(rowLenMismatch)/max((lenA-1), (lenB-1))))/rowcount
    }
    if (min((lenA-1), (lenB-1)) > 0 ){
      AvgBitMismatchRate = (AvgBitMismatchRate*(rowcount-1)   + (rowBitMismatchCount/min((lenA-1), (lenB-1))))/rowcount
    }
    
    print(paste("RowA = ", rowcount, "rowLenMismatch = ", rowLenMismatch, " : rowBitMismatchCount = ",  rowBitMismatchCount, " : LenBS_A = ", lenA-1, " : LenBS_B = ", lenB-1))
    print(paste("MedianA = ",  medValueA, " :  RangeA = ", rngA[1],"-", rngA[2], "LeftThA - RightThA ", LeftThA,"<->", RightThA))
    print(paste("MedianB = ",  medValueB, " :  RangeB = ", rngB[1],"-", rngB[2], "LeftThB - RightThB ", LeftThB,"<->", RightThB))
    
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
  
  print(paste("AvgLenMismatchRate  = ", round(AvgLenMismatchRate,4) , " : AvgBitMismatchRate = ",  round(AvgBitMismatchRate,4), "AvgBitExtractionRate", round(AvgBitExtractionRate,4)))

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

system("rm nodeA_*", wait=FALSE)
system("rm nodeB_*", wait=FALSE)
system("rm nodeAB_*", wait=FALSE)

nodeData_Sadzadi(A, "nodeA.txt", "nodeA_BS_Sadzadi.txt", "nodeA_BS_data_Sadzadi.txt", B, "nodeB.txt", "nodeB_BS_Sadzadi.txt", "nodeB_BS_data_Sadzadi.txt","nodeAB_Sadzadi_Entropy.txt")
print(paste(" Sadzadi - TX-#0's : TX - #1's "))
NumZero <- system("grep -o '0' nodeA_BS_Sadzadi.txt | wc -l", wait=TRUE)
NumOne <- system("grep -o '1' nodeA_BS_Sadzadi.txt | wc -l", wait=TRUE)
print(paste(NumZero , "<->", NumOne ))




