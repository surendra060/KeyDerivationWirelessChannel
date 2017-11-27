##**************************************** TOPE *********************************************************

getDiff_mat = function(nodeRSSmat){
  colDistIndex = 600
  temp = matrix(data=0,nrow = nrow(nodeRSSmat), ncol=colDistIndex)
  
  for (i in 1:colDistIndex) {
  temp[,i] = nodeRSSmat[,i]
  }
  
  for (i in 1:1000) {
    if (i<=(1000 - colDistIndex))
      nodeRSSmat[,i] = nodeRSSmat[ ,i] - nodeRSSmat[ ,(i+colDistIndex)]
    else if (i>(1000 - colDistIndex))
      nodeRSSmat[,i] = nodeRSSmat[ ,i] - temp[, (colDistIndex - (1000 - i))]
}
  return(nodeRSSmat)
}

## BitSTring from Tope
nodeData_Tope = function(nodeRSSA, infileA, outfileBitStringA,outfileDataA, nodeRSSB, infileB, outfileBitStringB,outfileDataB, entropyTestFile){
  nodeRSSA =  read.table(textConnection(gsub("&", "\t", readLines(infileA))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  nodeRSSB =  read.table(textConnection(gsub("&", "\t", readLines(infileB))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  
  nodeRSSmatA = data.matrix(nodeRSSA)
  nodeRSSmatB = data.matrix(nodeRSSB)
  
  print(paste0( nrow(nodeRSSA)))
  print(paste0( ncol(nodeRSSA)))
  
  i=1
  
  nodeRSSA <- getDiff_mat(nodeRSSmatA)
  nodeRSSB <- getDiff_mat(nodeRSSmatB)
  
  #print(paste0(nodeRSSA[1001, 1:1000]))
  
  #print(nodeRSSmatA)
  #print(nodeRSSmatB)  
  
  rowindex  = 1:nrow(nodeRSSA)
  colindex  = 1:ncol(nodeRSSA)
  
  outLier_i = 10;
  rowno=0;
  
  AvgBitExtractionRate = 0;
  AvgLenMismatchRate = 0;
  AvgBitMismatchRate = 0;
  
  cat("Median : Range : BitString_Len : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring  \n", file = outfileDataA, sep = " ", append = T)
  cat("Median : Range : BitString_Len : rowLenMismatch : rowBitMismatchCount : MatchBits : Bitstring\n", file = outfileDataB, sep = " ", append = T)
  
  #print(paste0(nodeRSSB[nrow(nodeRSSA), 1:1000]))
  

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
    
    # print(paste("RngA[1] = ", RngA[1], " : RngA[2] = ",  RngA[2]));
    LeftThA = medValueA - ((medValueA - rngA[1])/2)
    RightThA = medValueA + ((rngA[2] - medValueA)/2)
    
    LeftThB = medValueB - ((medValueB - rngB[1])/2)
    RightThB = medValueB + ((rngB[2] - medValueB)/2)

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
     # print(paste("Row = ", rowcount, " : Median = ",  medValue, " :  Range = ", rng[1],"to", rng[2], "Length of BitString = ", len-1, "*** ", nodeRSSmat[rowcount,1], ":", nodeRSSmat[rowcount,2], ": ", nodeRSSmat[rowcount,1000]))
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
      print(paste("RowA = ", rowcount, " : MedianA = ",  medValueA, " :  RangeA = ", rngA[1],"-", rngA[2], "LeftThA - RightThA ", LeftThA,"<->", RightThA))
      print(paste("RowB = ", rowcount, " : MedianB = ",  medValueB, " :  RangeB = ", rngB[1],"-", rngB[2], "LeftThB - RightThB ", LeftThB,"<->", RightThB))
      
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



 
nodeData_Tope(A, "nodeA.txt", "nodeA_BS_Tope.txt", "nodeA_BS_data_Tope.txt", B, "nodeB.txt", "nodeB_BS_Tope.txt", "nodeB_BS_data_Tope.txt", "nodeAB_Tope_Entropy.txt")
print(paste("TX-#0's : TX - #1's "))
print(paste(system("grep -o '0' nodeA_BS_Tope.txt | wc -l", wait=TRUE), "<->", system("grep -o '1' nodeA_BS_Tope.txt | wc -l", wait=TRUE)))




