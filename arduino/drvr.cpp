#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>

// start socket
int startWinsock(void) {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 0), &wsa);
}

// error handling for empty buffer
void quit()
{
	printf(stderr, "memory exhausted\n");
	exit(1);
}
// check for empty string and leading whitespaces
void whitespaceCheck() {
	while (1) {
		int c = getchar();

		if (c == EOF) break; // end of file 
		if (!isspace(c)) {
			ungetc(c, stdin); // reset
			break;
		}
	}
}
// reads and returns string from user input
char* readString(int max, char* name) {
	// read string 
	int i = 0;
	while (1) {
		// add terminating zero 
		int c = getchar();
		if (isspace(c) || c == EOF) { // read til whitespace or end of file
			name[i] = 0;
			break;
		}
		name[i] = c;
		if (i == max - 1) { // buffer is full
			max += max;
			name = (char*)realloc(name, max); // get new and larger buffer
			if (name == 0) quit();
		}
		i++;
	}
	return name;
}
char* convertString(char* str) { return str; }
char* sendUDPMessage(SOCKET sock, char* msg) {

	// define address
	SOCKADDR_IN receiver;
	receiver.sin_addr.s_addr = inet_addr("127.0.0.1");
	receiver.sin_family = AF_INET;
	receiver.sin_port = htons(1337);

	// send
	int returnCode = sendto(sock, msg, strlen(msg), 0, &receiver, sizeof(SOCKADDR_IN));
	if (returnCode == SOCKET_ERROR) {
		printf("Socket could not be started: %d\n", WSAGetLastError());
		return EXIT_FAILURE;
	}
	else {
		printf("Message sent: %s\n", msg);
		return EXIT_SUCCESS;
	}
}
int main()
{
	// init socket
	long ws = startWinsock();
	SOCKET sock;

	if (ws != 0) {
		printf("Socket could not be initialized: %d\n", ws);
		return 0;
	}

	// start socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("Socket could not be started: %d\n", WSAGetLastError());
	}
	else {
		printf("Socket successfully started.\n");
	}


	// buffer 
	int max = 20;
	char* name;

	while (1) {
		name = (char*)malloc(max); // allocate buffer
		if (name == 0) quit();

		printf("Enter a string: \n");

		whitespaceCheck();
		name = readString(max, name);
		name = convertString(name);

		sendUDPMessage(sock, name);

		free(name); // release memory 
	}

	return 0;
}

