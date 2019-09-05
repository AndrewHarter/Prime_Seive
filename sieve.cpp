#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <chrono>
#include <stdexcept>
#include <array>
#include <math.h>
#include "socket.h"

#define DEBUG
//#define OUTPUT

using namespace std;
using namespace chrono;

void parseFlags(int argc, char* argv[]);
void runAsClient();
void runAsServer();

// Global variables
char flags = 0;
const char HELP = 1;
const char RUN_AS_SERVER = 1<<1;
const char SERVER_HOST = 1<<2;
const int UPPER_BOUND = 20;


char serverHost[100] = "localhost"; // Default
int portno = 9090;

int upperBound = UPPER_BOUND;
int upperBoundSqrt = 0;
bool *prime;
bool isNextNum = 0;
int nextNum = 3;

int main(int argc, char* argv[]) {
	
	upperBoundSqrt = (int)sqrt((double)upperBound);
	prime = (bool*)malloc(UPPER_BOUND*sizeof(bool));
	//memset(prime, true, sizeof(prime));

	//Sets all number as prime numbers
	for( int i = 0; i < upperBound; i++)
		prime[i] = 1;

	// Parse flags
	parseFlags(argc, argv);

	// Run as Client or Server
	(flags & RUN_AS_SERVER) ? runAsServer() : runAsClient();
	free(prime);
}

void parseFlags(int argc, char* argv[]) {
	// For getopt
	extern char* optarg;
	extern int optind;
	int c, err = 0;

	// Useage if command is entered incorrectly
	char usage[] = "usage: %s [-hr] [-r]\n";

	// Parse flags
	while ((c = getopt(argc, argv, "hrs:")) != -1) {
		switch (c) {
			case 'h':
				flags += HELP;
				break;
			case 'r':
				flags += RUN_AS_SERVER;
				break;
			case '?':
				err = 1;
				break;
		}
	}

	// If there were invalid flags, inform the user and die
	if (err) {
		fprintf(stderr, usage, argv[0]);
		exit(1);
	}

	// If the user asked for help, print help screen and die
	if (flags & HELP) {
		printf("sieve [OPTIONS]\n");
		printf("\t-h\t\tPrint this help screen\n");
		printf("\t-r\t\tRun as server\n");
		printf("\tNo flag will run as client\n");
		exit(0);
	}
}



void runAsServer() {
	printf("Running as Server\n------------------\n\n");
	// Open socket and wait
	int server_socket = setupServerSocket(portno);
	int client_socket = serverSocketAccept(server_socket);

	while(nextNum <= upperBoundSqrt){
		read(client_socket, prime, upperBound*sizeof(bool));
		read(client_socket, &nextNum, sizeof(int));

#ifndef DEBUG
	printf("Next number recvd: %d\n", nextNum);
#endif
#ifndef OUTPUT
	printf("Recvd: ");
	for(int i = nextNum; i < upperBound; i++){
		if(prime[i]){
			printf("%d, ",i);
			}
		}
		printf("\n");
#endif	
		//Find next prime number
		isNextNum = 0;
		while(!isNextNum){
			if(prime[nextNum] == 1){
				isNextNum = 1;
#ifndef OUTPUT
				printf("Prime: %d\n", nextNum);
#endif
			}else{
				nextNum++;
			}
		}

		//Finds multiples for next prime and discards them
		for(int i = nextNum * 2; i<=upperBound; i += nextNum){
			prime[i] = 0;
		}
		//Find next prime to pass to client
		bool foundNextPrime = 0;
		int count = nextNum;
		while(!foundNextPrime){
			if(prime[count]){
				foundNextPrime = 1;
			}
			count++;
						
		}
		nextNum = count;
#ifndef OUTPUT
		printf("Sent: ");
		for(int i = nextNum; i < upperBound; i++){
			if(prime[i]){
				printf("%d, ",i);
			}
		}
		printf("\n\n");
#endif
		write(client_socket, prime, upperBound*sizeof(bool));
		write(client_socket, &nextNum, sizeof(int));
	
	}

	// Clean resources
	close(server_socket);
	close(client_socket);
}


void runAsClient() {
	printf("Running as Client\n------------------\n\n");
	// Open socket and connect to server
	int client_socket = callServer(serverHost, portno);
	
	//remove all even number except 2
	for(int i = 2; i <= upperBound; i = i+2){
		prime[i] = 0;
	}
	
#ifndef OUTPUT
	printf("Prime: %d\n", nextNum-1);
	printf("Sent: ");
	for(int i = nextNum; i < upperBound; i++){
		if(prime[i]){
			printf("%d, ",i);
		}
	}
	printf("\n\n");
#endif
	write(client_socket, prime, upperBound*sizeof(bool));
	write(client_socket, &nextNum, sizeof(int));

	while(nextNum <= upperBoundSqrt){
		read(client_socket, prime, upperBound*sizeof(bool));
		read(client_socket, &nextNum, sizeof(int));
#ifndef OUTPUT
		printf("Recvd: ");
		for(int i = nextNum; i < upperBound; i++){
			if(prime[i]){
				printf("%d, ",i);
				}
		}
		printf("\n");
#endif
		while(!isNextNum){
			if(prime[nextNum] == 1){
				isNextNum = 1;
#ifndef OUTPUT
				printf("Prime: %d\n", nextNum);
#endif
				
			}else{
				nextNum++;
			}
		}

		//Finds multiples for next prime and discards them
		for(int i = nextNum * 2; i<=upperBound; i += nextNum){
			prime[i] = 0;

		}
			//Finds next prime to pass to server
			bool foundNextPrime = 0;
			int count = nextNum;
			while(!foundNextPrime){
				if(prime[count]){
					foundNextPrime = 1;
				}
				count++;
							
			}
			nextNum = count;
#ifndef OUTPUT
			printf("Sent: ");
			for(int i = nextNum; i < upperBound; i++){
				if(prime[i]){
					printf("%d, ",i);
				}
			}	
			printf("\n\n");
#endif
#ifndef DEBUG
			printf("Next number recvd: %d\n", nextNum);
#endif
			write(client_socket, prime, upperBound*sizeof(bool));
			write(client_socket, &nextNum, sizeof(int));
	}
#ifndef DEBUG
	for(int i = 2; i<upperBound; i++){
		if(prime[i]){
			cout << i <<"\n";
		}
	}
#endif
	close(client_socket);
}

