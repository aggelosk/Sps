rm(list=ls())
library(foreach)
library(doMC)
n <- 2
registerDoMC(n)

system("make")

randInt <- floor(runif(n, 0, 1000000))

write.table(randInt, file="seeds.txt", quote=F, row.names=F, col.names=F)

curDir <- getwd()

name <- ""




#./sps -gens 6000 -smps 10 -rows 10 -cols 10 -cpct 10 -inpt bridge_mat.txt -strt 0 0
#./sps -gens 6000 -smps 20 -rows 10 -cols 10 -cpct 10 -inpt bridge_mat.txt -strt 0 0

foreach(i = 1:n) %dopar% {
	system(paste("rm -r run.", i, sep=""))

    setwd(file.path(curDir, name))
    seed <- randInt[i]
    dirName <- paste("run", i, sep="")
    working_dir<-file.path(paste(curDir, "/", name, sep=""), dirName)
    dir.create(working_dir, showWarnings=FALSE)
    setwd(working_dir)
    print(working_dir)

    #system(paste("Rscript ",curDir, "/write_file.R > ", working_dir,"/R1R2_input.txt",sep=""))
    system(paste("cp ",curDir,"/","bridge_mat.txt ",working_dir,sep=""))


    cmd <- paste(curDir, "/sps -gens 6000 -smps 10 -rows 10 -cols 10 -cpct 10 -inpt bridge_mat.txt -strt 0 0 ", sep="")
    system(cmd)
    getwd()
}


## st <- Sys.time() 
## st1 <- Sys.time()
## timeDif <- as.numeric(st1-st, units="secs")

## cmdKill <- paste("killall --older-than 30s ", curDir, "/sps", sep="")
## X <- ""
## while( timeDif < 40){
        
##     if( timeDif > 5 ){
##         X <- system(cmdKill, intern=TRUE)
##     }
##     st1 <- Sys.time()
##     timeDif <- as.numeric(st1-st, units="secs")
## }
## stop()

