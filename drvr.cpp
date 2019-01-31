#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <sstream> 
#include <list>
#include <fstream>
#include <json/json.h>
#include <iostream>

using namespace std;

// start socket
int startWinsock(void) {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 0), &wsa);
}
// error handling for empty buffer
void quit()
{
	cerr << stderr << endl << "memory exhausted" << endl;
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
char* readString(int max, char* buffer) {
	// read string 
	int i = 0;
	while (1) {
		// add terminating zero 
		int c = getchar();
		if (isspace(c) || c == EOF) { // read til whitespace or end of file
			if (c == ' ') {
				continue;
			}
			else {
				buffer[i] = 0;
				break;
			}
		}
		buffer[i] = c;
		if (i == max - 1) { // buffer is full
			max += max;
			buffer = (char*)realloc(buffer, max); // get new and larger buffer
			if (buffer == 0) quit();
		}
		i++;
	}
	return buffer;
}
char* sendUDPMessage(SOCKET sock, char* msg) {
	cout << "Sending Message: ";
	// define address
	SOCKADDR_IN receiver;
	receiver.sin_addr.s_addr = inet_addr("127.0.0.1");
	receiver.sin_family = AF_INET;
	receiver.sin_port = htons(1337);

	// send
	int returnCode = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &receiver, sizeof(SOCKADDR_IN));
	if (returnCode == SOCKET_ERROR) {
		cerr << "FAILURE " + WSAGetLastError() << endl;
		quit();
	}
	else {
		cout << "SUCCESS" << endl;
		return EXIT_SUCCESS;
	}
}
// load dictionary.json from disc and parse it
Json::Value buildDictionary() {
	Json::Value root;
	Json::Reader reader;
	ifstream text("dictionary.json", ifstream::binary);
	bool parsingSuccessful = reader.parse(text, root);
	if (!parsingSuccessful) {
		cerr << "FAILURE" << endl;
		quit();
	}
	return root;
}
//create dictionary hashmap from the json string
char* convertString(char* str) {

	// read dictionary from disk
	cout << "Load Dictionary file: ";
	Json::Value root = buildDictionary();
	cout << "SUCCESS" << endl;

	// iterate over building blocks
	string compare(str), result("");
	cout << "Processing user input..." << endl;
	for (Json::Value::const_iterator blocks = root.begin(); blocks != root.end(); ++blocks) {
		cout << "Checking " << blocks.name();
		result += blocks.name();
		for (Json::Value::const_iterator banks = root[blocks.name()].begin(); banks != root[blocks.name()].end(); ++banks) {
			cout << ", " << banks.name();

			for (int i = 0; i < banks->getMemberNames().size(); i++) {
				if (compare.find(banks->getMemberNames()[i]) != string::npos) {
					result += root[blocks.name()][banks.name()][banks->getMemberNames()[i]].asString();
				}
				else {
					// if not found, add zeroes
					result += "0x00";
				}
			}
		}
		cout << "..." << endl;
	}
	cout << "Preprocessing done: " << result << endl;
	return strcpy(str, result.c_str());
}
int main()
{
	// init socket
	cout << "Init Socket: ";
	long ws = startWinsock();
	SOCKET sock;

	if (ws != 0) {
		cerr << "FAILURE" <<endl << ws << endl;
		return -1;
	}
	else {
		cout << "SUCCESS" << endl;
	}

	// actually start socket
	cout << "Start Socket: ";
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		cerr << "FAILURE" << endl << WSAGetLastError() << endl;
	}
	else {
		cout << "SUCCESS" << endl;
	}

	// buffer 
	int max = 4192;
	char* message;

	while (1) { // prevents program from stopping
		message = (char*)malloc(max); // allocate buffer
		if (message == 0) quit();

		cout << "Enter a string: ";

		whitespaceCheck();
		message = readString(max, message);
		message = convertString(message);

		sendUDPMessage(sock, message);

		free(message); // release memory 
	}
	return 0;
}

