library(plyr)
library(ggplot2)

inFileA="nodeA.txt"
inFileB="nodeB.txt"

nodeRSSDataA =  read.table(textConnection(gsub("&", "\t", readLines(inFileA))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
nodeRSSDataB =  read.table(textConnection(gsub("&", "\t", readLines(inFileB))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))

nodeRSSmatA = data.matrix(nodeRSSDataA)
nodeRSSmatB = data.matrix(nodeRSSDataB)
rowindex  = 1:nrow(nodeRSSmatA)
colindex = 1:ncol(nodeRSSmatA)

outLier_i = 10;
rowno=0;

#print(nodeRSSData)
#hist(as.numeric(nodeRSSmatA))
par(mfrow=c(1,2))
rowLen = 1:100
Data=c(0,1000)

for (rowcount in 1:10) {
  
  DataA = as.numeric(nodeRSSmatA[rowcount,colindex])
  DataB = as.numeric(nodeRSSmatB[rowcount,colindex])
  
  meanValueA = mean(nodeRSSmatA[rowcount, colindex])
  stdDevA = sd(nodeRSSmatA[rowcount, colindex])
  meanValueB = mean(nodeRSSmatB[rowcount, colindex])
  stdDevB = sd(nodeRSSmatB[rowcount, colindex])
  
  rngA =c(rep(0,2));rngB =c(rep(0,2));
  
  medValueA  = median(as.numeric(nodeRSSDataA[rowcount, colindex]))
  medValueB  = median(as.numeric(nodeRSSDataB[rowcount, colindex]))
  
  #   rngA =c(rep(0,2));rngB =c(rep(0,2));
  # 
  # RngA = range(as.numeric(nodeRSSDataA[rowcount, colindex]))
  # RngB = range(as.numeric(nodeRSSDataB[rowcount, colindex]))
  # rngA[1] = as.numeric(sort(nodeRSSDataA[rowcount, colindex],partial=outLier_i)[outLier_i])
  # rngA[2] = as.numeric(sort(nodeRSSDataB[rowcount, colindex],partial=ncol(nodeRSSDataA)-outLier_i)[ncol(nodeRSSDataA)-outLier_i])
  # 
  # rngB[1] = as.numeric(sort(nodeRSSDataB[rowcount, colindex],partial=outLier_i)[outLier_i])
  # rngB[2] = as.numeric(sort(nodeRSSDataB[rowcount, colindex],partial=ncol(nodeRSSDataB)-outLier_i)[ncol(nodeRSSDataB)-outLier_i])
  # 
  
  
  
  
  x_A = count(DataA)
  x_B = count(DataB)
  
  
  print(x_A$x)
  print(x_A$freq)
  #y = hist(Data, breaks = 4*(max(Data) - min(Data)), plot = TRUE)
  # print(paste0(y$counts, " : ", y$breaks, " : ", y$density, " : ", y$mids, " : ", y$xname  ))
  #ggplot2::stat_count(mapping = NULL,data = nodeRSSDataA)
  #hist(y)
  #plot(y$breaks, y$counts)
  #z = lines(density(Data))
  #plot(y)
  #axis(side=1,at=y$mids,labels=seq(x$x))
  hgrid = seq(0, max(x_A$freq), by=50)
  xfitA<-seq(min(x_A$x),max(x_A$x),by=1)
  yfitA<-dnorm(xfitA,mean=meanValueA,sd=stdDevA)*1000

  xfitB<-seq(min(x_B$x),max(x_B$x), by=1)
  yfitB<-dnorm(xfitB,mean=meanValueB,sd=stdDevB)*1000 
  
  
 hgrid = seq(0, max(x_A$freq), by=50)
  
  plot(x_A$x, x_A$freq, type='l', col="red", lty="solid", lwd=2, xlim = c(-85,-65), ylim = c(0,400), xlab = 
         "Received Signal Strength Index (RSSI)", ylab = "Frequency", 
       main = "Node A - frequency Plot of RSSI (1000 Measurements)") 
  points(x_A$x, x_A$freq)
  lines(x_A$x, x_A$freq, type = 'h', lwd=2)
  lines(meanValueA, max(x_A$freq), type = 'h', lwd=2, col = "blue", lty="dashed")
  lines(medValueA, max(x_A$freq), type = 'h', lwd=2, col = "green", lty="dotted")
  lines(xfitA, yfitA, col="magenta", lwd=2, lty="dotted") 
  abline(h = hgrid, v= x_A$x, col = "lightgray", lty = 3)
  #grid(nx = (max(Data) - min(Data)), ny = max(x_A$freq/10), col = "lightgray", lty = "dotted", lwd = par("lwd"), equilogs = TRUE)
  print(paste0("StdDev ", stdDevA," : ", stdDevB, "  Meanvalues :  ", medValueA, " : ", medValueB, " Fitvalues : ", xfitA, "--- ", yfitA))
  #line <- readline()
  
  
  
  hgrid = seq(0, max(x_B$freq), by=50)
    plot(x_B$x, x_B$freq, type='l', col="red", lty="solid", lwd=2, xlim = c(-85,-65), ylim = c(0,400), xlab = 
         "Received Signal Strength Index (RSSI)", ylab = "Frequency", 
       main = "Node B - frequency Plot of RSSI (1000 Measurements)") 
  points(x_B$x, x_B$freq)
  lines(x_B$x, x_B$freq, type = 'h', lwd=2)
  lines(meanValueB, max(x_B$freq), type = 'h', lwd=2, col = "blue", lty="dashed")
  lines(medValueB, max(x_B$freq), type = 'h', lwd=2, col = "green", lty="dotted")
  lines(xfitB, yfitB, col="magenta", lwd=2, lty="dotted") 
  abline(h = hgrid, v= x_B$x, col = "lightgray", lty = 3)
  
  Sys.sleep(0.5)
  
}

#print(Data)


# 
# for (i in rowLen) {
#  # Data[i] = rep(0,1000)
#   Data[i] = as.numeric(nodeRSSData[i,])
#   if (i==1){
#             plot(Data[i], type = 'p', xlab = "Values", ylab = "Freq", main = paste0("Main Plot_", infile))
#           } else {
#             points(Data[i], type = 'p')
#           }
# }

# for (i in rowLen){
# hist((as.numeric(nodeRSSData[i,1:1000])), xlim=c(65,90), ylim=c(10,650), plot=TRUE, type = 'p',breaks=25)
# par=(new=TRUE)
# 
# }