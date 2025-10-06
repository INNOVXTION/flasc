# Build the flasc executable by default
CFLAGS = -g -O0
BUILD_DIR = bin

all: flasc

# Link step
flasc: flasc.o http.o serversock.o strings.o
	gcc -o flasc.exe $(CFLAGS) flasc.o http.o serversock.o strings.o -lws2_32

# Compile steps
flasc.o: flasc.c flasc.h
	gcc -c $(CFLAGS) flasc.c

http.o: http.c flasc.h
	gcc -c $(CFLAGS) http.c
	
serversock.o: serversock.c flasc.h
	gcc -c $(CFLAGS) serversock.c

strings.o: strings.c flasc.h
	gcc -c $(CFLAGS) strings.c

	
# Optional cleanup rule
clean:
	rm *.o