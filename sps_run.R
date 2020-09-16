
rm(list=ls())
library(foreach)
library(doMC)
n <- 20000
registerDoMC(2)

system("make")

curDir <- getwd()

name <- ""


seed <- sample(1:100000000, n,  replace=FALSE)

bene <- ""
bene <- " -bene 400 5000 1.05 -fixg 10 "
rec <- 0.3
cmds <- paste(curDir, "/sps -gens 2000 -rows 1 -cols 1 -frica 0.1 -migra 0.1 -grtha 0.02 -cpcta 200 -smps 10 -strt 0 0 10 -smps 10 -maxs 99999 -psnd 0.1 -rcmb ", rec, bene, "  -seed ", seed, sep="")

total.info<- foreach(i =  1:n)%dopar%{
    print(i)
    setwd(file.path(curDir, name))
    dirName <- paste("fix_03", i, sep="")
    working_dir<-file.path(paste(curDir, "/", name, sep=""), dirName)
    dir.create(working_dir, showWarnings=FALSE)
    setwd(working_dir)

    cmd <- cmds[i]
    system(cmd)
#    sfs <- read.table("ntons.txt", h=F)
#    tree <- read.table("total_tree.txt", h=F)[1,1]
    
#    return(list(sfs=sfs, anc=tree))
}

q()

s.tmp <- unlist(total.info, recursive=F)
s <- s.tmp[ (1:length(s.tmp)) %% 2 == 1]
ancestors <- s.tmp[ (1:length(s.tmp)) %% 2 == 0]

smat <- matrix(unlist(s), nrow=n, byrow=TRUE)

sfs <- apply(smat, 2, mean)

pdf("sfs.pdf")
plot(sfs[2:length(sfs)])
dev.off()

pdf("rec_mut.pdf")
plot(rec, smat[,1])
dev.off()


pdf("anc_mut.pdf")
plot(ancestors, smat[,1])
dev.off()


smat
