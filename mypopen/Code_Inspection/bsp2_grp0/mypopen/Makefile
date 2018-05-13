##
## @file Makefile
## Betriebssysteme mypopen Makefile
## Beispiel 2
##
## @author Sebastian Scheiber <ic17b084@technikum-wien.at>
## @date 2018/04/24
##
## @version $Revision: 1 $
##
## @todo
##
## URL: $HeadURL:  $
##
## Last Modified: $Author: Sebastian $
##

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

OBJECTS=src/mypopen.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.c: %.o
	$(CD) src
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

.PHONY: all
all: distclean popentest test-pipe doc

.PHONY: popentest
popentest: $(OBJECTS)
	$(CC) $(CFLAGS) -o src/popentest $(OBJECTS) -lpopentest -ldl

.PHONY: test-pipe
test-pipe: $(OBJECTS)
	$(CC) $(CFLAGS) -o src/test-pipe $(OBJECTS) -ltest-pipe

.PHONY: clean
clean:
	$(RM) src/*.o src/*~ src/mypopen src/popentest src/test-pipe

.PHONY: distclean
distclean: clean
	$(RM) doc/html/* doc/pdf/*

.PHONY: doc
doc: html pdf

.PHONY: html
html:
	$(DOXYGEN) doc/doxygen.dcf

.PHONY: pdf
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
