CC=gcc
CFLAGS=-Wall
MAINPROG=-o ExecSound
MAINFILE=son_a_completer


default:
	$(CC) -c gnuplot_i.c
	$(CC) -c $(MAINFILE).c
	$(CC) $(CFLAGS) gnuplot_i.o $(MAINFILE).o $(MAINPROG)

.PHONY: clean

clean:
	rm -f *.o *~
