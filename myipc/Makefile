##
## @file Makefile
## Betriebssysteme myipc Makefile
## Beispiel 3
##
## @author Ralf Ziefuhs <ic17b065@technikum-wien.at>
## @author Clemens Fritzsche <ic17b087@technikum-wien.at>
##  
## @date 2018/06/15
##
## @version $Revision: 1 $
##
## Last Modified: $Author: Ralf Ziefuhs $
##
## ------------------------------------------------------------- variables --
##

CC=gcc52
CFLAGS=-DDEBUG -Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic -fno-common -ftrapv -O3 -g -std=gnu11
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen

OBJECTS=myipc.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

.PHONY: all
all: sender empfaenger

sender: $(OBJECTS) 
	$(CC) $(CFLAGS) -o $@ $^ -lsem182 -lpthread -lrt

empfaenger: $(OBJECTS) 
	$(CC) $(CFLAGS) -o $@ $^ -lsem182 -lpthread -lrt


.PHONY: clean
clean:
	$(RM) -f *.o *~ sender
	$(RM) -f *.o *~ empfaenger	

.PHONY: distclean
distclean: clean
	$(RM) -r doc

doc: html pdf

html:
	$(DOXYGEN) doxygen.dcf

pdf: html
	$(CD) doc/pdf && \
	$(MV) refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	$(RM) refman_save.tex && \
	make && \
	$(MV) refman.pdf refman.save && \
	$(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	      *.ilg *.toc *.tps Makefile && \
	$(MV) refman.save refman.pdf

##
## ---------------------------------------------------------- dependencies --
##

##
## =================================================================== eof ==
##
