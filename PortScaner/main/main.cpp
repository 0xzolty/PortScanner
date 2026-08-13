#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <chrono>

struct Service {
	int port;
	const char* name;
};

struct PendingPort {
	SOCKET socket;
	int port;
	std::chrono::steady_clock::time_point startedAt;
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

	std::cout << "suggested service: unknown" << std::endl << std::endl;
}

enum class PortStatus {
	Open,
	Closed,
	Timeout,
	Unknown
};


int main() {
	WSADATA wsaData;

	int open = 0;
	int close = 0;
	int timeoutCount = 0;
	int unknownCount = 0;
	int speed = 0;

	// initialize Winsock 2.2 result holds the error code

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	// check if winsock work

	if (result != 0) {
		std::cerr << "fail ,fail code : " << result << std::endl;
		return 1;
	}

	// geting port to scan from user

	int portfrom, portto;

	std::cout << "give 1st port to scan : " << std::endl;
	if (!(std::cin >> portfrom)) {
		std::cerr << "wrong port, please enter a number" << std::endl;
		WSACleanup();
		return 1;
	}

	std::cout << "give last port to scan : " << std::endl;
	if (!(std::cin >> portto)) {
		std::cerr << "wrong port, please enter a number" << std::endl;
		WSACleanup();
		return 1;
	}

	std::cout << "chose speed of the scan from 1 to 10s : ";
	if (!(std::cin >> speed)) {
		std::cerr << "wrong timeout, please enter a number" << std::endl;
		WSACleanup();
		return 1;
	}

	if (speed < 1 || speed > 10) {
		std::cout << "wrong speed (1-10s) : " << std::endl;
		WSACleanup();
		return 1;
	}

	// chose destenity

	std::string ip_port;

	// geting ip addres to scan from uuser

	std::cout << "IP address or host name (e.g. '127.0.0.1' or 'localhost'):" << std::endl;
	if (!(std::cin >> ip_port)) {
		std::cerr << "wrong IP address input" << std::endl;
		WSACleanup();
		return 1;
	}

	if (portfrom < 1 || portfrom > 65535 || portto < 1 || portto > 65535 || portfrom > portto) {
		std::cout << "wrong port must be in range of 1 - 65535" << std::endl;
		WSACleanup();
		return 1;
	}
	addrinfo hints{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* addressList = nullptr;
	int addrResult = getaddrinfo(ip_port.c_str(), nullptr, &hints, &addressList);

	// error / user issue check

	if (addrResult != 0 || addressList == nullptr) {
		std::cerr << "could not resolve address: " << gai_strerrorA(addrResult) << std::endl;
		WSACleanup();
		return 1;
	}

	sockaddr_in target = *reinterpret_cast<const sockaddr_in*>(addressList->ai_addr);
	freeaddrinfo(addressList);

	auto reportStatus = [&](int port, PortStatus status) {
		if (status == PortStatus::Open) {
			std::cout << "port " << port << " open" << std::endl;
			open++;
			ServiceCheck(port);
		}
		else if (status == PortStatus::Closed) {
			std::cout << "port " << port << " closed" << std::endl << std::endl;
			close++;
		}
		else if (status == PortStatus::Timeout) {
			std::cout << "port " << port << " timeout / filtered" << std::endl << std::endl;
			timeoutCount++;
		}
		else {
			std::cout << "port " << port << " unknown error" << std::endl << std::endl;
			unknownCount++;
		}
	};

	constexpr std::size_t maxPendingPorts = 5;
	std::vector<PendingPort> pending;
	int nextPort = portfrom;

	while (nextPort <= portto || !pending.empty()) {
		while (pending.size() < maxPendingPorts && nextPort <= portto) {
			const int port = nextPort++;
			std::cout << "scaning port " << port << std::endl;

			SOCKET soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (soc == INVALID_SOCKET) {
				reportStatus(port, PortStatus::Unknown);
				continue;
			}

			u_long nonBlocking = 1;
			int nonBlockingResult = ioctlsocket(soc, FIONBIO, &nonBlocking);
			if (nonBlockingResult == SOCKET_ERROR) {
				closesocket(soc);
				reportStatus(port, PortStatus::Unknown);
				continue;
			}

			sockaddr_in portTarget = target;
			portTarget.sin_port = htons(static_cast<u_short>(port));
			auto startedAt = std::chrono::steady_clock::now();
			int connectResult = connect(soc, reinterpret_cast<sockaddr*>(&portTarget), sizeof(portTarget));

			if (connectResult == 0) {
				closesocket(soc);
				reportStatus(port, PortStatus::Open);
				continue;
			}

			int errorCode = WSAGetLastError();
			if (errorCode == WSAECONNREFUSED) {
				closesocket(soc);
				reportStatus(port, PortStatus::Closed);
			}
			else if (errorCode == WSAEWOULDBLOCK || errorCode == WSAEINPROGRESS) {
				pending.push_back({ soc, port, startedAt });
			}
			else {
				closesocket(soc);
				reportStatus(port, PortStatus::Unknown);
			}
		}

		if (pending.empty()) {
			continue;
		}

		fd_set writeSet;
		fd_set errorSet;
		FD_ZERO(&writeSet);
		FD_ZERO(&errorSet);

		auto earliestDeadline = pending.front().startedAt + std::chrono::seconds(speed);
		for (const PendingPort& pendingPort : pending) {
			FD_SET(pendingPort.socket, &writeSet);
			FD_SET(pendingPort.socket, &errorSet);

			auto deadline = pendingPort.startedAt + std::chrono::seconds(speed);
			if (deadline < earliestDeadline) {
				earliestDeadline = deadline;
			}
		}

		auto now = std::chrono::steady_clock::now();
		auto remaining = earliestDeadline - now;
		timeval timeout{};
		if (remaining > std::chrono::steady_clock::duration::zero()) {
			auto timeoutMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(remaining);
			if (std::chrono::duration_cast<std::chrono::steady_clock::duration>(timeoutMicroseconds) < remaining) {
				++timeoutMicroseconds;
			}

			timeout.tv_sec = static_cast<long>(timeoutMicroseconds.count() / 1000000);
			timeout.tv_usec = static_cast<long>(timeoutMicroseconds.count() % 1000000);
		}

		int selectResult = select(0, nullptr, &writeSet, &errorSet, &timeout);
		if (selectResult == SOCKET_ERROR) {
			for (const PendingPort& pendingPort : pending) {
				closesocket(pendingPort.socket);
				reportStatus(pendingPort.port, PortStatus::Unknown);
			}
			pending.clear();
			continue;
		}

		now = std::chrono::steady_clock::now();
		for (auto it = pending.begin(); it != pending.end();) {
			bool socketReady = FD_ISSET(it->socket, &writeSet) || FD_ISSET(it->socket, &errorSet);
			bool timedOut = now - it->startedAt >= std::chrono::seconds(speed);
			if (!socketReady && !timedOut) {
				++it;
				continue;
			}

			PortStatus status = PortStatus::Timeout;
			if (socketReady) {
				int socketError = 0;
				int socketErrorSize = sizeof(socketError);
				int socketResult = getsockopt(it->socket, SOL_SOCKET, SO_ERROR,
					reinterpret_cast<char*>(&socketError), &socketErrorSize);

				if (socketResult == SOCKET_ERROR) {
					status = PortStatus::Unknown;
				}
				else if (socketError == 0) {
					status = PortStatus::Open;
				}
				else if (socketError == WSAECONNREFUSED) {
					status = PortStatus::Closed;
				}
				else {
					status = PortStatus::Unknown;
				}
			}

			closesocket(it->socket);
			reportStatus(it->port, status);
			it = pending.erase(it);
		}
	}

	// timeout , close , open ports count

	std::cout << "port(s) open : " << open << std::endl;
	std::cout << "port(s) closed : " << close << std::endl;
	std::cout << "total ports scanned : " << portto - portfrom + 1 << std::endl;
	std::cout << "total timeouted port(s) : " << timeoutCount << std::endl;
	std::cout << "total unknown port(s) : " << unknownCount << std::endl;

	// clean socket api

	WSACleanup();

	return 0;
}
