rm(list=ls())
library(foreach)
library(doMC)
n <- 173
registerDoMC(2)

system("make")

curDir <- getwd()

name <- ""


seed <- sample(1:100000000, n,  replace=FALSE)
gens <- as.numeric(unlist(read.table("fix_01.gen", h=F)))
cmds <- paste(curDir, "/sps -gens ", gens, " -rows 1 -cols 1 -frica 0.1 -migra 0.1 -grtha 0.02 -cpcta 200 -smps 10 -strt 0 0 10 -smps 10 -maxs 99999 -psnd 0.1 -rcmb 0.1 -seed ", seed, sep="")

foreach(i =  1:n)%dopar%{

    setwd(file.path(curDir, name))
    dirName <- paste("no_01", i, sep="")
    working_dir <- file.path(paste(curDir, "/", name, sep=""), dirName)
    dir.create(working_dir, showWarnings=FALSE)
    setwd(working_dir)

    cmd <- cmds[i]
    system(cmd)
    getwd()
}
