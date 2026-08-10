#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <array>

struct Service {
	int port;
	const char* name;
};

const std::array<Service, 31> services = { {
	{20, "FTP data"},
	{21, "FTP control"},
	{22, "SSH"},
	{23, "Telnet"},
	{25, "SMTP"},
	{53, "DNS"},
	{80, "HTTP"},
	{110, "POP3"},
	{111, "RPCbind"},
	{135, "Microsoft RPC"},
	{139, "NetBIOS Session"},
	{143, "IMAP"},
	{389, "LDAP"},
	{443, "HTTPS"},
	{445, "SMB"},
	{465, "SMTPS"},
	{587, "SMTP Submission"},
	{636, "LDAPS"},
	{993, "IMAPS"},
	{995, "POP3S"},
	{1433, "Microsoft SQL Server"},
	{1521, "Oracle Database"},
	{2049, "NFS"},
	{3306, "MySQL"},
	{3389, "RDP"},
	{5432, "PostgreSQL"},
	{5900, "VNC"},
	{6379, "Redis"},
	{8080, "HTTP alternative"},
	{8443, "HTTPS alternative"},
	{27017, "MongoDB"}
} };

//checking if port number is in suggested port services array

void ServiceCheck(int port) {
	for (const Service& service : services) {
		if (service.port == port) {
			std::cout << "suggested service: " << service.name << std::endl << std::endl;

			return;
		}
	}

	std::cout << "suggested service: unknown"
		<< std::endl << std::endl;
}

int main() {
	WSADATA wsaData;
	
	int open = 0;
	int close = 0;
	int timeoutCount = 0;
	
	// initialize Winsock 2.2 result holds the error code

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	// check if winsock work 

	if (result != 0) {
		std::cerr << "fail ,fail code : " << result << std::endl;
		getchar();
		return 1;
	}

	// geting port to scan from user 

	int portfrom, portto;

	std::cout << "give 1st port to scan : " << std::endl;
	std::cin >> portfrom;

	std::cout << "give last port to scan : " << std::endl;
	std::cin >> portto;

	// chose destenity 

	std::string ip_port;

	// geting ip addres to scan from uuser 

	std::cout << "ip addres in format : '127.0.0.1'" << std::endl;
	std::cin >> ip_port;

	if (portfrom < 1 || portto > 65535 || portfrom > portto) {
		std::cout << "wrong port must be in range of 1 - 65535" << std::endl;
		WSACleanup();
		return 1;
	}
	sockaddr_in target{};
	target.sin_family = AF_INET;

	int addrResult = inet_pton(AF_INET, ip_port.c_str(), &target.sin_addr);

	// error / user issue check 

	if (addrResult == 0) {
		std::cerr << "wrong ip format, please put correct one eg. 127.0.0.1" << std::endl;

		WSACleanup();
		return 1;
	}
	else if (addrResult == -1) {
		std::cerr << "error: " << WSAGetLastError() << std::endl;

		
		WSACleanup();
		return 1;
	}

	for (int port = portfrom; port <= portto; ++port) {
		std::cout << "scaning port " << port << std::endl;

		// initialize socket with ipv4, reliable transmission, tcp and error check

		SOCKET soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (soc == INVALID_SOCKET) {
			std::cerr << "fail to connect ,fail code : " << WSAGetLastError() << std::endl;

			WSACleanup();
			return 1;
		}
		
		// setting socket in non blocking mode

		u_long nonBlocking = 1;
		int nonblockingsoc = ioctlsocket(soc, FIONBIO, &nonBlocking);

		if (nonblockingsoc == SOCKET_ERROR) {
			std::cerr << "error" << WSAGetLastError() << std::endl;
			
			closesocket(soc);
			WSACleanup();
			getchar();
			return 0;
		}

		// create target virable with sockaddr_in 

		target.sin_port = htons(port);

		// establish connection check if port is open 

		int con = connect(soc, (sockaddr*)&target, sizeof(target));

		if (con == SOCKET_ERROR) {
			int errorCode = WSAGetLastError();

			if (errorCode == WSAEWOULDBLOCK) {

				fd_set writeSet;
				fd_set errorSet;
				FD_ZERO(&writeSet);
				FD_ZERO(&errorSet);
				FD_SET(soc, &writeSet);
				FD_SET(soc, &errorSet);

				timeval timeout{};
				timeout.tv_sec = 2;
				timeout.tv_usec = 0;

				int selectResult = select(0, nullptr, &writeSet, &errorSet, &timeout);

				if (selectResult == 0) {
					std::cout << "port " << port << " timeout / filtered" << std::endl << std::endl;
					timeoutCount++;
				}
				else if (selectResult == SOCKET_ERROR) {
					std::cerr << "select error: " << WSAGetLastError() << std::endl << std::endl;
				}
				else {

					int SError = 0;
					int SErrorSize = sizeof(SError);
					int getResult = getsockopt(soc, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&SError), &SErrorSize);

					if (getResult == SOCKET_ERROR) {
						std::cerr << "getsockopt error: " << WSAGetLastError() << std::endl;
					}
					else if (SError == 0) {
						std::cout << "port " << port << " open" << std::endl  ;
						open++;
						ServiceCheck(port);
						
					}
					else if (SError == WSAECONNREFUSED) {
						std::cout << "port " << port << " closed" << std::endl;
						close++;
					}
					else {
						std::cout << "port " << port << " unknown error: " << SError << std::endl;
					}
				}
			}
			else if (errorCode == WSAECONNREFUSED) {
				std::cout << "port " << port << " closed" << std::endl << std::endl;
				close++;
			
			}
			else {
				std::cout << "port " << port << " unknown error: " << errorCode << std::endl << std::endl;
			}
			
			}
			else {
				std::cout << "port " << port << " open" << std::endl << std::endl;
				open++;
				ServiceCheck(port);

			}

		// close socket

		closesocket(soc);
	}
	
	// timeout , close , open ports count 

	std::cout << "port(s) open : " << open << std::endl;
	std::cout << "port(s) closed : " << close << std::endl; 
	std::cout << "total ports scanned : " << portto - portfrom + 1 << std::endl;
	std::cout << "total timeouted port(s) : " << timeoutCount << std::endl; 
	
	// clean socket api 

	WSACleanup();

	getchar();
	return 0;
}
