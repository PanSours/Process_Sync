






all: process_sync




process_sync: process_sync.o

	gcc -Wall -o process_sync process_sync.o -lm



process_sync.o: process_sync.c
	
	gcc -c process_sync.c



clean:
	
	rm -rf *.o logfile.txt process_sync

