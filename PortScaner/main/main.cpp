#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>

int main() {
	WSADATA wsaData;
	
	// inicjacja winsocket api z wersja 2 , 2 i przypisanie do int result w celu odczutania bledow 
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	//sprawdzenie dzialania winsocket
	if (result != 0) {
		std::cerr << "błąd , kod błedu : "<< result << std::endl;
		getchar();
		return 1;

	}
	// zamknicie socket api 
	WSACleanup();

	getchar();
	return 0;
}