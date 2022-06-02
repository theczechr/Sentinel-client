#include "server.hpp"
#include "send.cpp"
#include <iostream>

//int main()
//{
//	std::string message;
//	std::string room;
//	int i = 0;
//	std::cout << "Poslat/prijmout zpravu: ";
//	std::cin >> i;
//	switch (i)
//	{
//	case(0):
//		std::cout << "Zadejte zpravu: ";
//		std::cin >> message;
//		std::cout << "Zadejte room: ";
//		std::cin >> room;
//		server::send_message(message, room);
//		break;
//	case(1):
//		receive_message();
//		break;
//
//	default:
//		break;
//	}
//
//}

int main()
{
	server::login_account("tesssst", "asdhusaujdh7823jsandhj");
	server::create_account("for", "real", "real", "for");
	drogon::app().run();
	//std::cout << server::is_online() << std::endl;
}