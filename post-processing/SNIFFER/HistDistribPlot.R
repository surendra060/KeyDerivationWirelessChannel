library(plyr)
library(ggplot2)

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

plotFunc = function(x_A, x_B, mA, mB,stdDevA, stdDevB, rngA1, rngA2, rngB1, rngB2, algo, dirPath, rowcount, modFlag){
  rngA =c(rep(0,2));rngB =c(rep(0,2));
  rngA[1] = rngA1;  rngA[2] = rngA2;  rngB[1] = rngB1;  rngB[2] = rngB2;
  
  if ((algo == "Mathur") | (algo == "Sadzadi")){
    meanValueA = mA
    meanValueB = mB
    
    if (algo == "Mathur") {
     
      if(modFlag==0){
        alpha = 0.5
        LeftThA = (meanValueA - alpha*stdDevA)
        RightThA = (meanValueA + alpha*stdDevA)
        LeftThB = (meanValueB - alpha*stdDevB)
        RightThB = (meanValueB + alpha*stdDevB)
        
        xPolyA=seq(round(LeftThA,2), round(RightThA,2), by=0.1)
        xPolyB=seq(round(LeftThB,2), round(RightThB,2), by=0.1)
      }
      else if (modFlag==1){
        
        LeftThA = meanValueA - stdDevA*(abs(meanValueA - rngA[1]))/(abs(rngA[2] - rngA[1]))
        RightThA = meanValueA + stdDevA*(abs(rngA[2] - meanValueA)/abs(rngA[2] - rngA[1]))
        LeftThB = meanValueB - stdDevB*abs(meanValueB - rngB[1])/(abs(rngB[2] - rngB[1]))
        RightThB = meanValueB + stdDevB*(abs(rngB[2] - meanValueB)/abs(rngB[2] - rngB[1]))
        
        xPolyA=seq(round(LeftThA,2), round(RightThA,2), by=0.1)
        xPolyB=seq(round(LeftThB,2), round(RightThB,2), by=0.1)
      }
      
    }
    else if (algo == "Sadzadi"){
       if (modFlag==0)
          beta=3
        else if (modFlag==1)
          beta =1
      LeftThA = meanValueA - beta*(round(stdDevA, 2))
      RightThA <- LeftThA
      LeftThB = meanValueB - beta*(round(stdDevB, 2))
      RightThB <- LeftThB
      
      xPolyA=seq(round(LeftThA,2), round(RightThA,2), by=0.1)
      xPolyB=seq(round(LeftThB,2), round(RightThB,2), by=0.1)
    }
    
    xfitA<-seq(min(x_A$x)-1,max(x_A$x)+1,by=0.01)
    yfitA<-dnorm(xfitA,mean=meanValueA,sd=stdDevA)*1000
    xfitB<-seq(min(x_B$x)-1,max(x_B$x)+1, by=0.01)
    yfitB<-dnorm(xfitB,mean=meanValueB,sd=stdDevB)*1000 
    

    yMaxPolyA = rep(max(x_A$freq), length(xPolyA))
    yMaxPolyB = rep(max(x_B$freq), length(xPolyB))

    yPolyA = dnorm(xPolyA, mean = meanValueA, sd=stdDevA)*1000
    yPolyB = dnorm(xPolyB, mean = meanValueB, sd=stdDevB)*1000
  }
  
  else if ((algo == "Aono") | (algo == "Tope")){
    medValueA = mA
    medValueB = mB
    
    if (algo == "Aono"){
      if (modFlag==0){
          LeftThA = medValueA - ((medValueA - rngA[1])/2)
          RightThA = medValueA + ((rngA[2] - medValueA)/2)
          LeftThB = medValueB - ((medValueB - rngB[1])/2)
          RightThB = medValueB + ((rngB[2] - medValueB)/2)
      }
      else if (modFlag==1){
        rangeA = abs(rngA[1] - rngA[2])
        rangeB = abs(rngB[1] - rngB[2])
        LeftThA = medValueA - (rangeA/4)*(abs(medValueA - rngA[1]))/(abs(rngA[2] - rngA[1]))
        RightThA = medValueA + (rangeA/2)*(abs(rngA[2] - medValueA)/abs(rngA[2] - rngA[1]))
        LeftThB = medValueB - (rangeB/2)*abs(medValueB - rngB[1])/(abs(rngB[2] - rngB[1]))
        RightThB = medValueB + (rangeB/4)*(abs(rngB[2] - medValueB)/abs(rngB[2] - rngB[1]))
        
      }
    } else if (algo == "Tope") {
      if (modFlag==0){
          LeftThA = medValueA - ((medValueA - rngA[1])/2)
          RightThA = medValueA + ((rngA[2] - medValueA)/2)
          LeftThB = medValueB - ((medValueB - rngB[1])/2)
          RightThB = medValueB + ((rngB[2] - medValueB)/2)
      }else if (modFlag==1){
        rangeA = abs(rngA[1] - rngA[2])
        rangeB = abs(rngB[1] - rngB[2])
        LeftThA = medValueA - (rangeA/4)*(abs(medValueA - rngA[1]))/(abs(rngA[2] - rngA[1]))
        RightThA = medValueA + (rangeA/4)*(abs(rngA[2] - medValueA)/abs(rngA[2] - rngA[1]))
        LeftThB = medValueB - (rangeB/4)*abs(medValueB - rngB[1])/(abs(rngB[2] - rngB[1]))
        RightThB = medValueB + (rangeB/4)*(abs(rngB[2] - medValueB)/abs(rngB[2] - rngB[1]))
        
      }
    }  
    
    xPolyA=seq(round(LeftThA,2), round(RightThA,2), by=0.1)
    xPolyB=seq(round(LeftThB,2), round(RightThB,2), by=0.1)
 
    yMaxPolyA = rep(max(x_A$freq), length(xPolyA))
    yMaxPolyB = rep(max(x_B$freq), length(xPolyB))
    
  }

  
  ####   PLOTTING COMMENCES ##########################  
  plotfileA <- paste0(dirPath,rowcount,"_NodeA_B_", algo)
  png(plotfileA)

  hgrid = seq(0, max(x_A$freq)+100, by=50)
  
  plot(x_A$x, x_A$freq, type='l', col="black", lty="solid", lwd=2, xlim = c(min(x_A$x)-1,max(x_A$x)+1), ylim = c(0,max(x_A$freq)+150), 
       xlab = "Received Signal Strength Index (RSSI)", ylab = "Frequency", 
       main = paste0("Frequency Plot of NodeA_B RSSI values (", algo ," et al.)")) 
  points(x_A$x, x_A$freq, col= "red")
  #lines(x_A$x, x_A$freq, type = 'h', lwd=2)
  
  if ((algo == "Mathur") | (algo == "Sadzadi")){
    
    lines(xfitA, yfitA, col="blue", lwd=2, lty="dotted")
    
    lines(meanValueA, max(x_A$freq), type = 'h', lwd=2, col = "blue", lty="dashed")
    legend("topright", c("Frequency distribution","Mean RSSI value", "Approx. Normal distribution","Threshold RSSI") , 
           lty = c("solid","dashed","dotted", "dotted") , lwd = c(2,2,2,2) , col = c("black","blue","blue", "red"))
    
  }
  else if ((algo == "Aono") | (algo == "Tope")) {
    lines(medValueA, max(x_A$freq), type = 'h', lwd=2, col = "blue", lty="dashed")
    legend("topright", c("Frequency distribution","Median RSSI value","Threshold RSSI") , 
           lty = c("solid","dashed", "dotted") , lwd = c(2,2,2) , col = c("black","blue", "red"))
    
  }
    polygon(c(round(LeftThA,2), xPolyA, round(RightThA,2)), c(0,yMaxPolyA,0), border=NA, col=rgb(193/255,205/255, 205/255,0.3))
    lines(round(LeftThA,2),  max(x_A$freq), col="red", lwd=2, lty="dotted" , type="h")
    lines(round(RightThA,2),  max(x_A$freq), col="red", lwd=2, lty="dotted", type="h")

    abline(h = hgrid, v= x_A$x, col = "lightgray", lty = 3)
    dev.off()
  
  #X11()
  
  plotfileB <- paste0(dirPath,rowcount,"_NodeB_A_", algo)
  png(plotfileB)
  
  plot(x_B$x, x_B$freq, type='l', col="black", lty="solid", lwd=2, xlim = c(min(x_B$x)-1,max(x_B$x)+1), ylim = c(0,max(x_B$freq)+150), 
       xlab = "Received Signal Strength Index (RSSI)", ylab = "Frequency", 
       main = paste0("Frequency Plot of NodeB_A RSSI values (", algo ," et al.)")) 
  points(x_B$x, x_B$freq, col= "red")
  #lines(x_A$x, x_A$freq, type = 'h', lwd=2)
  
  if ((algo == "Mathur") | (algo == "Sadzadi")){
    lines(xfitB, yfitB, col="blue", lwd=2, lty="dotted")
    
    lines(meanValueB, max(x_B$freq), type = 'h', lwd=2, col = "blue", lty="dashed")
    legend("topright", c("Frequency distribution","Mean RSSI value", "Approx. Normal distribution","Threshold RSSI") , 
           lty = c("solid","dashed","dotted", "dotted") , lwd = c(2,2,2,2) , col = c("black","blue","blue", "red"))
    
  }
  else if ((algo == "Aono") | (algo == "Tope")) {
    lines(medValueB, max(x_B$freq), type = 'h', lwd=2, col = "blue", lty="dashed")
    legend("topright", c("Frequency distribution","Median RSSI value","Threshold RSSI") , 
           lty = c("solid","dashed", "dotted") , lwd = c(2,2,2) , col = c("black","blue", "red"))
    
  }
  polygon(c(round(LeftThB,2), xPolyB, round(RightThB,2)), c(0,yMaxPolyB,0), border=NA, col=rgb(193/255,205/255, 205/255,0.3))
  lines(round(LeftThB,2),  max(x_B$freq), col="red", lwd=2, lty="dotted" , type="h")
  lines(round(RightThB,2),  max(x_B$freq), col="red", lwd=2, lty="dotted", type="h")
  
  
 # print(paste0("medValueB ** medValueB", medValueA," : ", medValueB, "  Meanvalues :  ", meanValueA, " : ", meanValueB, " Fitvalues : ", xfitA, "--- ", yfitA))
  
  
  
  abline(h = hgrid, v= x_B$x, col = "lightgray", lty = 3)
  
  #dev.copy(png, filename = plotfileB)
  dev.off()
  
  Sys.sleep(0.5)
}


graphPlot = function(inFileA, inFileB, algo, modFlag){
 # inFileA="nodeA.txt"
#  inFileB="nodeB.txt"
  print(inFileA)
  print(inFileB)
  
  nodeRSSDataA =  read.table(textConnection(gsub("&", "\t", readLines(inFileA))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  nodeRSSDataB =  read.table(textConnection(gsub("&", "\t", readLines(inFileB))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
  
  nodeRSSmatA = data.matrix(nodeRSSDataA)
  nodeRSSmatB = data.matrix(nodeRSSDataB)
  
  if (algo=="Tope"){
    nodeRSSDataA <- getDiff_mat(nodeRSSmatA)
    nodeRSSDataB <- getDiff_mat(nodeRSSmatB)
    nodeRSSmatA = data.matrix(nodeRSSDataA)
    nodeRSSmatB = data.matrix(nodeRSSDataB)
  }
  
  rowindex  = 1:nrow(nodeRSSmatA)
  colindex = 1:ncol(nodeRSSmatA)
  
  outLier_i = 10;
  rowno=0;
  
  #print(nodeRSSData)
  #hist(as.numeric(nodeRSSmatA))
  par(mfrow=c(1,2))
  rowLen = 1:100
  Data=c(0,1000)
  workDir<-getwd()
  if (modFlag==0)
  dirPath<-paste0(workDir,"/SNIFFER_GRAPHS/NODES/",algo, "/")
  else if (modFlag==1)
    dirPath<-paste0(workDir,"/SNIFFER_GRAPHS/NODES/",algo, "/Modified/")
  
  #filename<-"DensityPlot.png"
  
  for (rowcount in 340:350) {
    
    DataA = as.numeric(nodeRSSmatA[rowcount,colindex])
    DataB = as.numeric(nodeRSSmatB[rowcount,colindex])
    
    meanValueA = mean(nodeRSSmatA[rowcount, colindex])
    stdDevA = sd(nodeRSSmatA[rowcount, colindex])
    meanValueB = mean(nodeRSSmatB[rowcount, colindex])
    stdDevB = sd(nodeRSSmatB[rowcount, colindex])
    
    rngA =c(rep(0,2));rngB =c(rep(0,2));
    
    medValueA  = median(as.numeric(nodeRSSDataA[rowcount, colindex]))
    medValueB  = median(as.numeric(nodeRSSDataB[rowcount, colindex]))
    
    RngA = range(as.numeric(nodeRSSDataA[rowcount, colindex]))
    RngB = range(as.numeric(nodeRSSDataB[rowcount, colindex]))
    rngA[1] = as.numeric(sort(nodeRSSDataA[rowcount, colindex],partial=outLier_i)[outLier_i])
    rngA[2] = as.numeric(sort(nodeRSSDataA[rowcount, colindex], decreasing = TRUE)[outLier_i])

    rngB[1] = as.numeric(sort(nodeRSSDataB[rowcount, colindex],partial=outLier_i)[outLier_i])
    rngB[2] = as.numeric(sort(nodeRSSDataB[rowcount, colindex],partial=ncol(nodeRSSDataB)-outLier_i)[ncol(nodeRSSDataB)-outLier_i])

    x_A = count(DataA)
    x_B = count(DataB)
    
    if ((algo == "Mathur") | (algo == "Sadzadi")){
      mA = meanValueA
      mB = meanValueB
     }  else if ((algo == "Aono") | (algo == "Tope")){
       mA = medValueA
       mB = medValueB
     }
    
    plotFunc(x_A, x_B, mA, mB, stdDevA, stdDevB, rngA[1], rngA[2], rngB[1], rngB[2], algo, dirPath, rowcount, modFlag)
    print(x_A$x)
    print(x_A$freq)
    print(paste0("MeanA : ", meanValueA, " stdDevA : ", stdDevA, " MeanB : ", meanValueB, " stdDevB : ", stdDevB))
    print(paste0("rngA[1] : ", rngA[1], " rngA[2] : ", rngA[2], " rngB[1] : ", rngB[1], " rngB[2] : ", rngB[2]))
  }
}

# graphPlot("nodeE_B.txt", "nodeE_A.txt", "Aono", 0)
# graphPlot("nodeE_B.txt", "nodeE_A.txt", "Aono", 1)
# 
# graphPlot("nodeE_B.txt", "nodeE_A.txt", "Tope", 0)
# graphPlot("nodeE_B.txt", "nodeE_A.txt", "Tope", 1)
# 
# graphPlot("nodeE_B.txt", "nodeE_A.txt", "Sadzadi", 0)
# graphPlot("nodeE_B.txt", "nodeE_A.txt", "Sadzadi", 1)
# 
# graphPlot("nodeE_B.txt", "nodeE_A.txt", "Mathur", 0)
# graphPlot("nodeE_B.txt", "nodeE_A.txt", "Mathur", 1)


graphPlot("nodeA_B.txt", "nodeB_A.txt", "Aono", 0)
graphPlot("nodeA_B.txt", "nodeB_A.txt", "Aono", 1)

graphPlot("nodeA_B.txt", "nodeB_A.txt", "Tope", 0)
graphPlot("nodeA_B.txt", "nodeB_A.txt", "Tope", 1)

graphPlot("nodeA_B.txt", "nodeB_A.txt", "Sadzadi", 0)
graphPlot("nodeA_B.txt", "nodeB_A.txt", "Sadzadi", 1)

graphPlot("nodeA_B.txt", "nodeB_A.txt", "Mathur", 0)
graphPlot("nodeA_B.txt", "nodeB_A.txt", "Mathur", 1)
