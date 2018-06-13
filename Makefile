all: build clean

build: main.o
	gcc -pthread -o project main.o

main.o: main.c main.h
	gcc -c main.c -o main.o

clean:
	rm -rf *.o

mrproper: clean
	rm -rf main

run: build
	./project
