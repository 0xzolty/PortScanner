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
		std::cerr << "fail ,fail code : "<< result << std::endl;
		getchar();
		return 1;

	}

	// initialize socket with ipv4, reliable transmission, tcp and error check
	SOCKET soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (soc == INVALID_SOCKET) {
		std::cerr << "fail to connect ,fail code : " << WSAGetLastError() << std::endl;
		getchar();
		return 1;

	}
	sockaddr_in target;
	target.sin_family = AF_INET;
	target.sin_port = htons(80);

	int addrResult = inet_pton(AF_INET, "127.0.0.1", &target.sin_addr);

	if (addrResult == 0) {
		std::cerr << "wrong ip format, please put correct one eg. 127.0.0.1" << std::endl;
		getchar();
		return 1;
	}
	else if (addrResult == -1) {
		std::cerr << "error: " << WSAGetLastError() << std::endl;
		getchar();
		return 1;
	}
	
	//close socket 
	closesocket(soc);
	// close socket api 
	WSACleanup();

	getchar();
	return 0;
}