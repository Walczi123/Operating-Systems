Sopy_1: Sopy_1.o Sopy_1h.o
	gcc -o Sopy_1 Sopy_1.o Sopy_1h.o

Sopy_1.o: Sopy_1.c Sopy_1h.h	
	gcc -o Sopy_1.o -c Sopy_1.c

Sopy_1h.o: Sopy_1h.c Sopy_1h.h
	gcc -o Sopy_1h.o -c Sopy_1h.c

.PHONY: clean clean_all

clean:
	rm -f Sopy_1.o Sopy_1h.o

clean_all:
	rm -f Sopy_1.o Sopy_1h.o Sopy_1