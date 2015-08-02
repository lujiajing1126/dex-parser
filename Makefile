CC = gcc
CFLAGS = -std=gnu99
parser: parser.o
	$(CC) -o parser parser.o
parser.o: parser.c
.PHONY: clean
clean:
	rm *.o parser