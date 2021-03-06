rm(list=ls())
library(foreach)
library(doMC)
n <- 5000
registerDoMC(4)

system("make")

curDir <- getwd()

name <- ""


seed <- sample(1:100000000, n,  replace=FALSE)
cmds <- paste(curDir, "/sps -gens 10000 -rows 1 -cols 1 -frica 0.1 -migra 0.1 -grtha 0.02 -cpcta 2000 -smps 10 -strt 0 0 10 -smps 10 -maxs 99999 -psnd 0.1 -rcmb 0.2 -bene 4000 5000 1.0 1.02 -fixg 10 -single_bene -seed ", seed, sep="")

foreach(i =  1:n)%dopar%{
    setwd(file.path(curDir, name))
    dirName <- paste("extra_double_sel", i, sep="")
    working_dir<-file.path(paste(curDir, "/", name, sep=""), dirName)
    dir.create(working_dir, showWarnings=FALSE)
    setwd(working_dir)

    cmd <- cmds[i]
    system(cmd)
    getwd()
}
