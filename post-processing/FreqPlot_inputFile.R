inFile="nodeB.txt"
nodeRSSData =  read.table(textConnection(gsub("&", "\t", readLines(inFile))), header = F, sep = ";", stringsAsFactors = T, colClasses = c(rep("integer", 1000), rep("NULL", 1)))
#print(nodeRSSData)

rowLen = 1:100
Data=c(0,1000)
for (i in rowLen) {
  Data = as.numeric(nodeRSSData[i,1:1000])
  x = count(Data)
  print(paste0("Density ", x$x," : ", x$freq, "      "))
  dataColor= rgb((i+100)/200,(100-i)/100,i/100,0.2)
  if (i==1){
    plot(x$x, x$freq , type = 'l', pch=10, cex=0.7, xlab = "Values(Negative(-) dBm)", ylab = "Freq", xlim = c(65,90), ylim=c(1,500), main = paste0("RSS Distribution Plot_", inFile))
    # plot(Data, type = 'l', xlab = "Values", ylab = "Freq", xlim = c(65,90), ylim=c(1,500), main = paste0("Main Plot_", inFile))
    # polygon(Data, density = -1, col = dataColor)
    
    } else {
      # par(new=T)
      # polygon(Data, density = -1, col = dataColor)
      points(x$x, x$freq, type = 'l', pch=20, cex=0.7, col = dataColor)
    }
  
}
print(Data)


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