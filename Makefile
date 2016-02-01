CC=gcc

webserver: webserver.c
	$(CC) webserver.c -o webserver

clean:
	rm webserver
