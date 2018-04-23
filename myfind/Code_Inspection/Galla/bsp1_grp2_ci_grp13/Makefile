##
## @file Makefile
## Betriebssysteme myFind Makefile
## Beispiel 1
##
## @author Magdalena Andrae <ic17b079@technikum-wienat>
## @author Rainhardt Gabriel <ic17b078@technikum-wien.at> 
## @date 2018/04/07
##
## @version $Revision: 001 $
##
## Adapted Makefile of Thomas M. Galla / Bernd Petrovitsch
##
## Last Modified: $Author: Magdalena Andrae $
##
## ------------------------------------------------------------- variables --
##
CC=gcc52 
CFLAGS=-DDEBUG -Wall -pedantic -Werror -Wextra -Wstrict-prototypes -Wformat=2 -fno-common -ftrapv -g -O3 -std=gnu11
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen
## define object files here
OBJECTS=myfind.o 

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

##
## ### FB_TMG: sowohl myfind als auch clean als dependencies von
## all zu haben ist nicht so schlau ... - haben Sie make all
## jemals ausprobiert?
##
.PHONY: all
all: clean myfind 

myfind: $(OBJECTS)
	$(CC) $(CFLAGS) -o "$@" "$^"

.PHONY: clean
clean:
	$(RM) *.o *~
	$(RM) myfind
	
.PHONY: distclean
distclean: clean
	$(RM) -r doc
	$(RM) myfind

doc: html pdf

.PHONY: html
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
## ### FM_TMG: Dependencies fehlen
##

##
## =================================================================== eof ==
##
