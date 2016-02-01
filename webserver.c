/* Using skeleton code from provided server.c file
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */
#include <fcntl.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

// Get filetype extension from filepath
char * getExt(char * path) {

    char * ext = strrchr(path, '.'); //get extension
    
    if (ext[1] == 'j' && ext[3] == 'g')
	ext[4] = '\0';
    else if (ext[3] == 'e')
	ext[5] = '\0';
    else if (ext[3] == 'f')
	ext[4] = '\0';

    if (!ext)
	ext = "html";

    return ext;
}

// Open file, format header, send response
void get_file(int newsockfd, char * path) {
    char * OK = "HTTP/1.1 200 OK\n"
    "Content-type: %s\n";
    
    char * MISS = "HTTP/1.1 404 NOT FOUND\n"
    "Content-type: text/html\n"
    "\n"
    "<html><body><h1><b>404 File not found\n</b></h1></body></html>";
   
    char response[2048];
    int filefd = -1;
    char * header;
    int i = 0;
    char c;

    memset(response, 0, 2048);	//reset memory

    filefd = open(++path, O_RDONLY );
    //printf("Result of open: %d\n", filefd);

    //File not found
    if ( filefd < 0 ){ 
	//printf("%s", MISS);
	write(newsockfd, MISS, 2048); 
	error("ERROR, file can't be retrieved");
    }

    char * ext = getExt(path);

    //printf("Extension: %s\n", ext);
   
    // Determine filetype and format appropriate response
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0){
	// Do stuff for jpg
	header = "image/jpeg\n";
	snprintf(response, 2048, OK, header);
	write(newsockfd, response, strlen(response));
	}
    else if (strcmp(ext, ".gif") == 0 ) {
	// Do stuff for gif
	header = "image/gif\n";
	snprintf(response, 2048, OK, header);
	write(newsockfd, response, strlen(response));
    }
    else {
	// Do stuff for text/html
	header = "text/html\n";
	snprintf(response, 2048, OK, header);
	write(newsockfd, response, strlen(response));
    }
    
    while( (i = read(filefd, &c, 1))) {
	if (i < 0) {
	    error("ERROR, file can't be read");
	}
	if (write(newsockfd, &c, 1) < 1) {
	    error("ERROR, file can't be sent");
	}
    }
}

// Parse client's request, check for proper protocol and method, call get_file()
void request_handler(int newsockfd) {
    int n = -1;
    char buffer[2048];

    memset(buffer, 0, 2048);	//reset memory

    //read client's message
    n = read(newsockfd,buffer,2048);
    if (n < 0) error("ERROR reading from socket");
    
    // Split the request into parts
    char method[sizeof(buffer)];
    char protocol[sizeof(buffer)];
    char path[sizeof(buffer)];
    
    sscanf(buffer, "%s %s %s", method, path, protocol);
    printf( "%s\n",buffer);

    // Ensure accepted protocol
    if (strcmp(protocol, "HTTP/1.1") || strcmp(protocol, "HTTP/1.0") ){
	if (strcmp(method, "GET") == 0){
	    get_file(newsockfd, path);    
	}
    } else {
	printf("protocol: %s, method: %s", protocol, method);
	error("ERROR, protocol not supported!");
    }

}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     sockfd = socket(AF_INET, SOCK_STREAM, 0);	//create socket

     if (sockfd < 0){ 
        error("ERROR opening socket");
     }

     memset((char *) &serv_addr, 0, sizeof(serv_addr));	//reset memory

     //fill in address info
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     listen(sockfd,5);	//5 simultaneous connection at most
    
    // While loop to enable webserver to run indefinitely
    while(1) { 
	 //accept connections
	 newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	     
	 if (newsockfd < 0) 
	   error("ERROR on accept");
	 
	 pid = fork();

	if (pid < 0){
	    error("ERROR on fork");
	}
 
	if (pid == 0) {
	    close(sockfd);

	    request_handler(newsockfd);

	    if (close(newsockfd) < 0) {
		error("ERROR connection close");
	     }
    
	    exit(EXIT_SUCCESS);
    	} else { 

	    if (close(newsockfd) < 0) {
		error("ERROR connection close parent");
	    }

	    waitpid(-1, NULL, 0); 
	}
   
    } 
         
     return 0; 
}
