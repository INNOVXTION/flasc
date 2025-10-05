# Build the flasc executable by default

all: flasc

# Link step
flasc: flasc.o http.o serversock.o
	gcc -o flasc.exe flasc.o http.o serversock.o -lws2_32

# Compile steps
flasc.o: flasc.c flasc.h
	gcc -c flasc.c

http.o: http.c flasc.h
	gcc -c http.c

serversock.o: serversock.c flasc.h
	gcc -c serversock.c

# Optional cleanup rule
clean:
	rm *.o