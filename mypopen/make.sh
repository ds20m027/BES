#!/bin/bash

#ToDo: make makefile

gcc52 mypopen.c -lmypopen -o mypopen -DDEBUG -Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic -fno-common -ftrapv -O3 -g -std=gnu11 -lpopentest -ldl && /usr/local/src/libpopentest.zip
