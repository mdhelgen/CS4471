FLAGS=-c -Wall

all: fritter

fritter: fritter.o
	gcc fritter.o -o fritter


fritter.o: fritter.c
	gcc $(FLAGS) fritter.c fritter.h
	

clean:
	rm -rf *.o
	rm -rf fritter

run: fritter
	./fritter
