#!/usr/bin/Rscript
args = commandArgs(TRUE)

filename = args[1]

print(paste("Analyizing", filename))

csvdata = read.csv(paste(filename,sep='', '.txt'),head=TRUE, sep=",")

#extract branch information and separate into category.

makeplots <- function(datatable, title)
{
    ndatatable <- subset(datatable, datatable$Taken==1)
    #density
    d <- density(unique(ndatatable$InstructionAddress))
    plot(d, main=paste("Branch Density vs Address Space", title))
    #histogram
    barplot(table(ndatatable$InstructionAddress), main=paste("Branch Frequency", title))
    #Branch Distance
    displacement = ndatatable$InstructionAddress - ndatatable$Target
    barplot(table(displacement), main=paste("Taken Target Displacement", title))
}

#Analyze all instructions
makeplots(csvdata, paste("All Branches", filename))

#Analyze Conditional Branches
makeplots(subset(csvdata, csvdata$Conditional==1), 
    paste("Conditional Branches", filename))

#Analyze Calls
makeplots(subset(csvdata, csvdata$Call==1),
    paste("Calls Branches", filename))

