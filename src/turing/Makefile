T ?= C

ifeq ($(T),HB)
FLAG=-DHB
endif


all:
	gcc send_and_recv.c $(FLAG) -o main -lpthread
