TRACES    = $(wildcard *.bz2)
TRACENAME = $(patsubst %.bz2,% ,$(TRACES))
LOGNAMES  = $(addsuffix .txt, $(TRACENAME))
GRAPHNAMES= $(addsuffix .pdf, $(TRACENAME))


all: $(GRAPHNAMES) $(LOGNAMES)
	echo "Generated Plots"
cleanall:
	rm -f *.txt *.pdf

cleanpdf:
	rm -f *.pdf

%.txt: %.bz2
	logfile=$@ ../predictor $* 

%.pdf: %.txt
	./analysis.R $* 

