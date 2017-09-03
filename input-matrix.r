n<-20
mig_mat <- matrix(ncol=n,nrow=n/2)

for (i in (1:n-6)){
  mig_mat[,i]<-0.05
}

for (i in ((n-5):n)){
  mig_mat[,i]<-0.7
}

for (i in (1:3)){
  mig_mat[i,]<-0.7
}
mig<-as.vector(mig_mat)

write.table(t(mig),file="mig_mat.txt",row.names = FALSE,col.names = FALSE)
