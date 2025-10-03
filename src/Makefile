all: final

flasc: flasc.o http.o
	gcc -o flasc flasc.o http.o -lws2_32

flasc.o: flasc.c flasc.h http.h sockhead.h
	gcc -c flasc.c -lws2_32

http.o: http.c http.h server.o
	gcc -c http.c server.o -lws2_32

server.o: server.c sockhead.h
	gcc -c serversock.c -lws2_32

clean:
	rm flasc.o http.o final