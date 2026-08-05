#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>

int main() {
	WSADATA wsaData;
	
	// initialize Winsock 2.2 result holds the error code
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	// check if winsock work 
	if (result != 0) {
		std::cerr << "błąd , kod błedu : "<< result << std::endl;
		getchar();
		return 1;

	}
	// close socket api 
	WSACleanup();

	getchar();
	return 0;
}