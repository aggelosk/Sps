rm(list=ls())
library(foreach)
library(doMC)
n <- 10000
registerDoMC(20)

system("make")

curDir <- getwd()

name <- ""


seed <- sample(1:100000000, n,  replace=FALSE)
#seed <- read.table("seed_left.txt", header=FALSE)[,1]

#write.table(seed, file="seed_left.txt", quote=F, row.names=F, col.names=F)
cmds <- paste(curDir, "/sps -gens 5000 -rows 5 -cols 5 -cpcta 100 -smps 10 -strt 0 0 10 -smps 10 -maxs 999 -psnd 0.005 -rcmb 0.2 -seed ", seed, sep="")
#write.table(x=cmds, file="cmd.txt", append=F, quote=F, row.names=F, col.names=F)

foreach(i =  1:n)%dopar%{
    setwd(file.path(curDir, name))
    dirName <- paste("neut_rec", i, sep="")
    working_dir<-file.path(paste(curDir, "/", name, sep=""), dirName)
    dir.create(working_dir, showWarnings=FALSE)

    setwd(working_dir)

    system(paste("cp ", curDir, "/spat*.txt .", sep=""))

    cmd <- cmds[i]
    system(cmd)
    getwd()
}
