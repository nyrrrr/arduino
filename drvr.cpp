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

using namespace std;

// start socket
int startWinsock(void) {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 0), &wsa);
}
// error handling for empty buffer
void quit()
{
	fprintf(stderr, "memory exhausted\n");
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
			buffer[i] = 0;
			break;
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
	printf("Sending Message: ");
	// define address
	SOCKADDR_IN receiver;
	receiver.sin_addr.s_addr = inet_addr("127.0.0.1");
	receiver.sin_family = AF_INET;
	receiver.sin_port = htons(1337);

	// send
	int returnCode = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &receiver, sizeof(SOCKADDR_IN));
	if (returnCode == SOCKET_ERROR) {
		printf("FAILURE %d\n", WSAGetLastError());
		quit();
	}
	else {
		printf("SUCCESS\n");
		printf("Message sent: %s\n", msg);
		return EXIT_SUCCESS;
	}
}
// this method loads the JSON file that contains the string mappings
// necessary for sending instructions to the arduino board
char* loadDictionaryFile() {
	FILE *fpointer = fopen("./dictionary.json", "r");
	long size;
	char* buffer = 0;
	int c, i = 0;

	if (fpointer) {
		// determine size of file
		fseek(fpointer, 0, SEEK_END); // jump to end
		size = ftell(fpointer);
		fseek(fpointer, 0, SEEK_SET); // jump back
		buffer = (char*) malloc(size + 1); // set buffer size to file length

		while ((c = fgetc(fpointer)) != EOF) {
			buffer[i++] = (char)c;
		}
		buffer[i] = 0; // add terminating zero

		fclose(fpointer);
		return buffer;
	}
	else {
		perror("FAILURE\nError");
		exit(1);
	}
}
map<string, string> buildDictionary(char* dictcharstring) {

	map<string, string> dictionary;
	string dict(dictcharstring);

	// remove special chars, linebreaks and tabulators and quotes
	char chars[] = "{} \n\t\"";
	for (int i = 0; i < strlen(chars); ++i)
	{
		dict.erase(std::remove(dict.begin(), dict.end(), chars[i]), dict.end());
	}

	stringstream ss(dict); // convert to stringstream for easier editing
	string keyvaluepair; // "example": "value"

	// split string by comma
	while (getline(ss, keyvaluepair, ','))
	{
		// splitting keyvaluepair into one key and one value string for dictionary usage
		dictionary[(string) keyvaluepair.substr(0, keyvaluepair.find(':'))] = keyvaluepair.substr(keyvaluepair.find(':') + 1, strlen(keyvaluepair.c_str())-1); // dictionary[key] = value
	}

	return dictionary;
}
char* convertString(char* str, char* dict) {
	//create dictionary hashmap from the json string

	map<string, string> dictionary = buildDictionary(dict);
	map<string, string>::iterator it;

	string result;
	string msg(str);
	stringstream ss(msg);

	int i = 0;

	while (getline(ss, msg, ',')) { // split by comma
			
			it = dictionary.find(msg); // find input in dictionary
			if (it == dictionary.end()) { // if string not found try next string
				continue;
			}
			else { // create result string
				result += it->second;
			}
			
		//printf("%s\n", msg.c_str());
	}
	result += "\0"; // ending zero

	return strcpy(str, result.c_str());
}
int main()
{
	// init socket
	printf("Init Socket: ");
	long ws = startWinsock();
	SOCKET sock;

	if (ws != 0) {
		printf("FAILURE %d\n", ws);
		return -1;
	}
	else {
		printf("SUCCESS\n");
	}

	// actually start socket
	printf("Start Socket: ");
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("FAILURE %d\n", WSAGetLastError());
	}
	else {
		printf("SUCCESS\n");
	}

	// read dictionary from disk
	printf("Load Dictionary file: ");
	char* dict = loadDictionaryFile();

	printf("SUCCESS\n");

	// buffer 
	int max = 4192;
	char* message;

	while (1) { // prevents program from stopping
		message = (char*)malloc(max); // allocate buffer
		if (message == 0) quit();

		printf("Enter a string: ");

		whitespaceCheck();
		message = readString(max, message);
		message = convertString(message, dict);

		sendUDPMessage(sock, message);

		free(message); // release memory 
	}
	return 0;
}

