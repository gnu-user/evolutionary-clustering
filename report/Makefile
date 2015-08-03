TEX = pdflatex -interaction nonstopmode
BIB = bibtex
 
PROPOSAL = proposal
PRELIMINARY = lit_review
FINAL = final_report
BIBFILE = references.bib
BUNDLE = all_reports.pdf

all: $(FINAL).pdf $(PRELIMINARY).pdf $(PROPOSAL).pdf clean

$(FINAL).pdf: $(FINAL).tex $(FINAL).bbl
	$(TEX) $(FINAL) 
	$(TEX) $(FINAL)
 
$(FINAL).bbl: $(FINAL).tex $(BIBFILE)
	$(TEX) $(FINAL)
	$(BIB) $(FINAL)

$(PRELIMINARY).pdf: $(PRELIMINARY).tex $(PRELIMINARY).bbl
	$(TEX) $(PRELIMINARY) 
	$(TEX) $(PRELIMINARY)
 
$(PRELIMINARY).bbl: $(PRELIMINARY).tex $(BIBFILE)
	$(TEX) $(PRELIMINARY)
	$(BIB) $(PRELIMINARY)

$(PROPOSAL).pdf: $(PROPOSAL).tex $(PROPOSAL).bbl
	$(TEX) $(PROPOSAL) 
	$(TEX) $(PROPOSAL)
 
$(PROPOSAL).bbl: $(PROPOSAL).tex $(BIBFILE)
	$(TEX) $(PROPOSAL)
	$(BIB) $(PROPOSAL)

view: $(BUNDLE)
	open $(BUNDLE)
 
spell::
	ispell *.tex

clean::
	rm -fv *.aux *.log *.bbl *.blg *.toc *.out *.lot *.lof

clean-all::
	rm -fv *.aux *.log *.bbl *.blg *.toc *.out *.lot *.lof *.pdf