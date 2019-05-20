CC = mpicc
CFLAGS = -I./include -g -Wall -O3
LDFLAGS = -lm 

all: mapreduce

usecase.o: src/usecase.c include/usecase.h 
	$(CC) $(CFLAGS) src/usecase.c

backend.o: src/backend.c include/backend.h
	$(CC) $(CFLAGS) src/backend.c

main.o: src/main.c src/backend.c src/usecase.c include/usecase.h include/backend.h
	$(CC) $(CFLAGS) src/main.c src/usecase.c src/backend.c

mapreduce: src/usecase.o src/backend.o src/main.o
	$(CC) -o mapreduce.o $(CFLAGS) src/usecase.c src/backend.c src/main.c $(LDFLAGS)

clean: rm -f bin/*.o
