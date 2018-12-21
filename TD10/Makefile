CC=gcc
CFLAGS=-Wall
LDFLAGS=-lm
MAINPROG=-o shepard
MAINFILE=shepard


default:
	$(CC) -c $(MAINFILE).c
	$(CC) $(CFLAGS) $(MAINFILE).o $(LDFLAGS) $(MAINPROG)

.PHONY: clean

clean:
	rm -f *.o *~
