# BitString Generation using Aono et al
nodeData_Aono = function(nodeRSS, infile, outfileBitString,outfileData){
  nodeRSS =  read.table(textConnection(gsub("&", "\t", readLines(infile))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))

  nodeRowSum =  rowSums(nodeRSS)
  nodeRowAvg = nodeRowSum/1000
  print(nodeRowAvg[1])
  print(sum(nodeRSS[1, 1:1000])/1000)
  rowindex  = 1:nrow(nodeRSS)
  colindex  = 1:ncol(nodeRSS)
  
  len = 1
  rowno=0
  cat("Median : Range : BitString_Len : Bitstring ", file = outfileData, sep = " ", append = T)
  cat("\n", file = outfileData, sep = " ", append = T)
  # library(plyr)
  # t = count(nodeRSS, c('67', '68', '69','70', '71', '72', '73', '74', '75','76', '77', '78', '79', '80', '81' ))
  #print(nodeRSS);
  # 
  
  # for(rowcount in rowindex)
  # {
  #     for(colcount in colindex){
  #       Data[colcount] = as.numeric(nodeRSS[rowcount, colcount])
  #       if (colcount==1){
  #         plot(Data, type = 'p', xlab = "Values", ylab = "Freq", main = paste0("Main Plot_", infile))
  #       } else { 
  #         points(Data, type = 'p') 
  #       }
  #  }
  #   }
  
  
  for(rowcount in rowindex)
  {
    len=1;   bitstring = c(rep(100,1000))
    medValue  = median(as.numeric(nodeRSS[rowcount, 1:1000]))
    rng = range(as.numeric(nodeRSS[rowcount, 1:1000]))
    LeftTh = medValue - ((medValue - rng[1])/2)
    RightTh = medValue + ((rng[2] - medValue)/4)
      for(colcount in colindex){
        if (nodeRSS[rowcount, colcount]<LeftTh)
            {
              bitstring[len] = 0
              len=len+1
            }
          else if (nodeRSS[rowcount, colcount]>RightTh)
          {
            bitstring[len] = 1
            len=len+1
          }
      }
    print(paste("Row = ", rowcount, " : Median = ",  medValue, " :  Range = ", rng[1],"-", rng[2], "LeftTh - RightTh ", LeftTh,"<->", RightTh, "Length of BitString = ", len-1))
    
    cat(rbind(bitstring[1:len-1]), file = outfileBitString, sep = " ", append = T)
    cat("\n", file = outfileBitString, sep = " ", append = T)
    
    cat(medValue, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    cat(rng, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    cat(len-1, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    
    cat(rbind(bitstring[1:len-1]), file = outfileData, sep = " ", append = T)
    cat("\n", file = outfileData, sep = " ", append = T)
    
  }
}

## BitSTring from Tope
nodeData_Tope = function(nodeRSS, infile, outfileBitString,outfileData) {
  nodeRSS =  read.table(textConnection(gsub("&", "", readLines(infile))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  nodeRSSmat = data.matrix(nodeRSS)
  i=1
  print(nrow(nodeRSSmat))
  temp = matrix(data=0,nrow = nrow(nodeRSSmat), ncol=1)
  
  temp[,1] = nodeRSSmat[,1]
    for (i in 1:1000) {
    if (i<1000)
    nodeRSSmat[,i] = nodeRSSmat[ ,i] - nodeRSSmat[ ,(i+1)]
    else if (i==1000)
      nodeRSSmat[,i] = nodeRSSmat[ ,i] - temp[,1]
  }
  rowindex  = 1:nrow(nodeRSSmat)
  colindex  = 1:ncol(nodeRSSmat)
  len = 1
  cat("Median : Range : BitString_Len : Bitstring ", file = outfileData, sep = " ", append = T)
  cat("\n", file = outfileData, sep = " ", append = T)
  
  for(rowcount in rowindex)
  {
    len=1;   bitstring = rep(100,1000)

    medValue = median(nodeRSSmat[rowcount, rowindex])
    rng = range(nodeRSSmat[rowcount, rowindex])

    for(colcount in colindex){
      if (nodeRSSmat[rowcount, colcount]<=(medValue-1))
      {
        bitstring[len] = 0
        len=len+1
      }
      if (nodeRSSmat[rowcount, colcount]>=(medValue+1))
      {
        bitstring[len] = 1
        len=len+1
      }
      
    }
    print(paste("Row = ", rowcount, " : Median = ",  medValue, " :  Range = ", rng[1],"to", rng[2], "Length of BitString = ", len-1, "*** ", nodeRSSmat[rowcount,1], ":", nodeRSSmat[rowcount,2], ": ", nodeRSSmat[rowcount,1000]))
    
    print(bitstring[1:len-1])
    print(bitstring)
    
    if (len>0)
    cat(rbind(bitstring[1:len-1]), file = outfileBitString, sep = " ", append = T)
    cat("\n", file = outfileBitString, sep = " ", append = T)

    cat(medValue, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    cat(rng, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    cat(len-1, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)

    cat(rbind(bitstring[1:len-1]), file = outfileData, sep = " ", append = T)
    cat("\n", file = outfileData, sep = " ", append = T)
  }

}

## BitSTring from Mathur
nodeData_Mathur = function(nodeRSS, infile, outfileBitString,outfileData, alpha) {
  nodeRSS =  read.table(textConnection(gsub("&", "", readLines(infile))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  nodeRSSmat = data.matrix(nodeRSS)
  i=1
  print(nrow(nodeRSSmat))
  rowindex  = 1:nrow(nodeRSSmat)
  colindex  = 1:ncol(nodeRSSmat)
  len = 1
  cat("Mean : Std Dev : Alpha : Range : BitString_Len : Bitstring ", file = outfileData, sep = " ", append = T)
  cat("\n", file = outfileData, sep = " ", append = T)
  
  for(rowcount in rowindex)
  {
    len=1;  bitstring = c(rep(100,1000))

    meanValue = mean(nodeRSSmat[rowcount, rowindex])
    stdDev = sd(nodeRSSmat[rowcount, rowindex])
    rng = range(nodeRSSmat[rowcount, rowindex])
    Lfactor = min((meanValue - rng[1])/(rng[2] - rng[1]), 1)
    Rfactor = (rng[2] - meanValue)/(rng[2] - rng[1])/2
    print(paste(meanValue, stdDev, alpha, rng[1], rng[2]))
    
    for(colcount in colindex){
      if (nodeRSSmat[rowcount, colcount]<(meanValue-(Lfactor*alpha*stdDev)))
      {
        bitstring[len] = 0
        len=len+1
      }
      else if (nodeRSSmat[rowcount, colcount]>(meanValue+(Rfactor*alpha*stdDev)))
      {
        bitstring[len] = 1
        len=len+1
      }
    }
    print(paste("Mean is ", round(meanValue,2)," Lth -Rth = ", (meanValue-(Lfactor*alpha*stdDev)), " - ", (meanValue+(Rfactor*alpha*stdDev)),   "  Length of BitString = ", len-1))
    #print(bitstring[1:len-1])
    
    cat(rbind(bitstring[1:len-1]), file = outfileBitString, sep = " ", append = T)
    cat("\n", file = outfileBitString, sep = " ", append = T)
    
    cat(round(meanValue,2), file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    cat(round(stdDev,2), file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    cat(alpha, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    
    cat(rng, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    cat(len-1, file = outfileData, sep = " ", append = T)
    cat(":", file = outfileData, sep = " ", append = T)
    
    cat(rbind(bitstring[1:len-1]), file = outfileData, sep = " ", append = T)
    cat("\n", file = outfileData, sep = " ", append = T)
  }
  
}

system("rm nodeA_*", wait=FALSE)
system("rm nodeB_*", wait=FALSE)


# nodeData_Aono(A, "nodeA.txt", "nodeA_BS_Aono.txt", "nodeA_BS_data_Aono.txt")
# nodeData_Aono(B, "nodeB.txt", "nodeB_BS_Aono.txt", "nodeB_BS_data_Aono.txt")
# 
# nodeData_Tope(A, "nodeA.txt", "nodeA_BS_Tope.txt", "nodeA_BS_data_Tope.txt")
# nodeData_Tope(B, "nodeB.txt", "nodeB_BS_Tope.txt", "nodeB_BS_data_Tope.txt")
# 
# alpha = 1
# nodeData_Mathur(A, "nodeA.txt", "nodeA_BS_Mathur.txt", "nodeA_BS_data_Mathur.txt", alpha)
# nodeData_Mathur(B, "nodeB.txt", "nodeB_BS_Mathur.txt", "nodeB_BS_data_Mathur.txt", alpha)
# 
# print(paste("TX-#0's : TX - #1's "))
# print(paste(system("grep -o '0' nodeA_BS_Aono.txt | wc -l", wait=TRUE), "<->", system("grep -o '1' nodeA_BS_Aono.txt | wc -l", wait=TRUE)))




