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

	// create target virable with sockaddr_in 
	sockaddr_in target;
	target.sin_family = AF_INET;
	target.sin_port = htons(80);

	// chose destenity  

	int addrResult = inet_pton(AF_INET, "1.1.1.1", &target.sin_addr);

	// error / user issue check 
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

	// establish connection check if port is open 
	int con = connect(soc, (sockaddr*)&target, sizeof(target));
	if (con == SOCKET_ERROR) {
		std::cerr << "error " << WSAGetLastError() << std::endl;
		getchar();
	}
	else{
		std::cout << "port open" << std::endl;
	}
	//close socket 
	closesocket(soc);
	
	// close socket api 
	WSACleanup();

	getchar();
	return 0;
}