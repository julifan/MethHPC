CC = mpicxx
CFLAGS = -I./include -g -Wall -O3
LDFLAGS = -lm 

all: mapreduce

src/hash.o: src/hash.cpp include/hash.h
	$(CC) $(CFLAGS) -c src/hash.cpp

src/usecase.o: src/usecase.cpp include/usecase.h 
	$(CC) $(CFLAGS) -c src/usecase.cpp

src/backend.o: src/backend.cpp include/backend.h src/hash.o
	$(CC) $(CFLAGS) -c src/backend.cpp

src/main.o: src/main.cpp src/backend.cpp src/usecase.cpp include/usecase.h include/backend.h
	$(CC) $(CFLAGS) -c src/main.cpp src/usecase.cpp src/backend.cpp

mapreduce: src/usecase.o src/backend.o src/main.o src/hash.o
	$(CC) -o mapreduce.o $(CFLAGS) src/usecase.cpp src/backend.cpp src/main.cpp src/hash.cpp $(LDFLAGS)

clean: rm -f src/*.o *.o
