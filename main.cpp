#include "server.hpp"
#include <iostream>
#include <vector>
#include <fstream>
int main()
{
	std::vector<std::string> kontakty = { "Karel", "Kornel", "Robin", "Valon", "Kuba" };

	std::string message;
	std::string receiver;
	int r = 0;
	int i = 0;
	std::cout << "Poslat/prijmout zpravu: ";
	std::cin >> i;
	switch (i)
	{
	case(0):
		for (std::string kontakt : kontakty)
		{
			std::cout << kontakt << " ";
		}
		std::cout << '\n';
		std::cout << "vyberte si kontakt: (0,1,2,3,4)";
		std::cin >> r;
		std::cout << "Zadejte zpravu: ";
		std::cin >> message;
		receiver = kontakty[r];
		server::send_message(message, receiver);
		break;
	case(1):
		server::receive_message();
		break;

	default:
		break;
	}

}