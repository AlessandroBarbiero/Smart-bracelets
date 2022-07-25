CONTIKI = ../../contiki

all: smart-bracelet.c                                                                         
                                                     
                                                       

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
