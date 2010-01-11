args <- commandArgs(TRUE)
FileName=args[1]
dddd <- read.table(FileName, header=TRUE)

x=dddd[,1]
y=dddd[,2]
plot(x,y,pch=1,col=4)
lines(c(-50,50),c(-50,50))

