#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/


void checkTextFiles(char* plaintext, char* keytext){
	FILE *plaintext_File = fopen(plaintext, "r");
	FILE *keytext_File = fopen(keytext, "r");
	char plaintext_String[8000];
	char keytext_String[8000];

	fgets(plaintext_String, 8000, (FILE*)plaintext_File);
	fgets(keytext_String, 8000, (FILE*)keytext_File);

	if(strlen(plaintext_String) > strlen(keytext_String)){
		fprintf(stderr,"plaintext is not the same size as key\n"); 
		exit(1); 
	}

	int i;
	for(i = 0; i < strlen(plaintext_String); i++){
		if(plaintext_String[i] == ' ' || plaintext_String[i] == '\n')
			continue;
		else if(plaintext_String[i] < 65 || plaintext_String[i] > 90){
			fprintf(stderr,"plaintext has bad characters\n"); 
			exit(1);
		}
	}

	for(i = 0; i < strlen(keytext_String); i++){
		if(keytext_String[i] == ' ' || keytext_String[i] == '\n')
			continue;
		else if(keytext_String[i] < 65 || keytext_String[i] > 90){
			fprintf(stderr,"keytext has bad characters\n"); 
			exit(1);
		}
	}
	fclose(plaintext_File);
	fclose(keytext_File);

}

// Error function used for reporting issues
void error(const char *msg) { 
	perror(msg); 
	exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
					int portNumber, 
					char* hostname){
 
	// Clear out the address struct
	memset((char*) address, '\0', sizeof(*address)); 

	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);

	// Get the DNS entry for this host name
	struct hostent* hostInfo = gethostbyname(hostname); 
	if (hostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0); 
	}
	// Copy the first IP address from the DNS entry to sin_addr.s_addr
	memcpy((char*) &address->sin_addr.s_addr, 
		hostInfo->h_addr_list[0],
		hostInfo->h_length);
}

int main(int argc, char *argv[]) {
	int socketFD, portNumber, charsWritten, charsWritten1, charsWritten2, charsRead;
	struct sockaddr_in serverAddress;
	char buffer[8000];
	char plaintext[8000];
	char key[8000];
	FILE* plaintext_File;
	FILE* key_File;
	// Check usage & args
	if (argc < 4) { 
		fprintf(stderr,"Not enough arguments"); 
		exit(0); 
	}
	
	//Check and gather Text Files
	//checkTextFiles(argv[1], argv[2]);
	plaintext_File = fopen(argv[1], "r");
	key_File = fopen(argv[2], "r");
	fgets(plaintext, 8000, (FILE*)plaintext_File);
	fgets(key, 8000, (FILE*)key_File);
	fclose(plaintext_File);
	fclose(key_File);

	// Create a socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0){
		error("CLIENT: ERROR opening socket");
	}

	// Set up the server address struct
	setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		fprintf(stderr,"Couldn't connect to server"); 
		exit(2); 
	}

	// Send message to server
	// Write to the server
	charsWritten = send(socketFD, "dec_client", 100, 0); 
	sleep(1);
	charsWritten1 = send(socketFD, plaintext, strlen(plaintext), 0); 
	sleep(1);
	charsWritten2 = send(socketFD, key, strlen(key), 0); 

	// Get return message from server
	// Clear out the buffer again for reuse
	memset(buffer, '\0', sizeof(buffer));
	// Read data from the socket, leaving \0 at end
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
	if (charsRead < 0){
		error("CLIENT: ERROR reading from socket");
	}
	fprintf(stdout, "%s", buffer); 
	// Close the socket
	close(socketFD); 
	return 0;
}