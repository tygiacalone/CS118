CC=gcc

webserver: webserver.c
	$(CC) webserver.c -Wall -o webserver

clean:
	rm webserver
