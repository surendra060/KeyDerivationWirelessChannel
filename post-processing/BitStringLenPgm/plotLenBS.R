data_Aono = read.table("nodeB_LenBS_data_Aono.txt", stringsAsFactors = FALSE)
data_Tope = read.table("nodeB_LenBS_data_Tope.txt", stringsAsFactors = FALSE)
data_Mathur = read.table("nodeB_LenBS_data_Mathur.txt", stringsAsFactors = FALSE)

x=c(1:length(data_Aono$V1))
plot(x, data_Aono$V1, pch = 0, ylim = c(0,1000), xlab = "Sample Measurment", ylab = " BitSequence Length", main = "Length of Bit-sequence( Aono, Tope & Mathur) ")
points(x, data_Tope$V1, col = "red", pch =1 )
points(x, data_Mathur$V1, col = "blue", pch =2 )
legend("top" , "top" , y.intersp = 0.8, title = "Legend", c("Aono","Tope", "Mathur"), pch = c(0, 1,2), col = c("black","red", "blue"))
