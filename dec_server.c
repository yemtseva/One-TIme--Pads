#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MOD 27
#define SPACE 91

void error(const char *msg) {
	perror(msg);
	exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
					int portNumber){
 
	// Clear out the address struct
	memset((char*) address, '\0', sizeof(*address)); 

	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);
	// Allow a client at any address to connect to this server
	address->sin_addr.s_addr = INADDR_ANY;
}

char* decryptMessage(char* message, char* key, int textLength){
	char* ciphertext = malloc(textLength);
	int i;
	for(i = 0; i < strlen(message); i++){
		if(message[i] == ' ' && key[i] == ' ')
			ciphertext[i] = SPACE - SPACE + 65; // have to subtract 65 since A is 65 on ASCII table
		else if(message[i] == ' ')
			ciphertext[i] = SPACE - key[i] + 65; 
		else if(key[i] == ' ')
			ciphertext[i] = message[i] - SPACE + 65;
		else
			ciphertext[i] = message[i] - key[i] + 65;  

		if(ciphertext[i] < 65) // have to add 65 since A is 65 on ASCII table
			ciphertext[i] = ciphertext[i] + MOD;
		
		if(ciphertext[i] == SPACE)
			ciphertext[i] = ' ';
	}
	ciphertext[strlen(ciphertext)-1] = '\n';
	return ciphertext;
}


int main(int argc, char *argv[]){
	int connectionSocket, charsRead, charsRead2;
	char buffer[8000];
	char plaintext[8000];
	char key[8000];
	char* ciphertext;
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);

	// Check usage & args
	if (argc < 2) { 
		fprintf(stderr,"USAGE: %s port\n", argv[0]); 
		exit(1);
	} 
  
	// Create the socket that will listen for connections
	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket < 0) {
		error("ERROR opening socket");
	}

	// Set up the address struct for the server socket
	setupAddressStruct(&serverAddress, atoi(argv[1]));

	// Associate the socket to the port
	if (bind(listenSocket, 
			(struct sockaddr *)&serverAddress, 
			sizeof(serverAddress)) < 0){
		error("ERROR on binding");
	}

	
	// Start listening for connetions. Allow up to 5 connections to queue up
	listen(listenSocket, 5); 

	// Accept a connection, blocking if one is not available until one connects
	while(1){
	// Accept the connection request which creates a connection socket

		connectionSocket = accept(listenSocket, 
					(struct sockaddr *)&clientAddress, 
					&sizeOfClientInfo); 
		if (connectionSocket < 0){
			error("ERROR on accept");
		}

		pid_t pid = fork();

		if(pid == 0){
			// Get the message from the client and display it
			int i = 0;
			while(i < 3){
				memset(buffer, '\0', 8000);
				// Read the client's message from the socket
				charsRead = recv(connectionSocket, buffer, 8000, 0); 

				if (charsRead < 0){
					error("ERROR reading from socket");
				}
				if(i == 0){
					if(strcmp(buffer, "dec_client") != 0){
						fprintf(stderr,"enc_client tried to connect to enc_server - NOT POSSIBLE", argv[3]); 
						exit(2);
					}
				}
				else if(i == 1)
					strcpy(plaintext, buffer);
				else
					strcpy(key, buffer);
				i++;
			}
			ciphertext = decryptMessage(plaintext, key, strlen(plaintext));
			// Send a Success message back to the client
			charsRead = send(connectionSocket, ciphertext, 8000, 0); 
			if (charsRead < 0){
				error("ERROR writing to socket");
			}
			// Close the connection socket for this client
			exit(0);
		}
		close(connectionSocket);

	}
	// Close the listening socket
	close(listenSocket); 
	return 0;
}