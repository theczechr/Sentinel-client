#include "server.hpp"
#include <iostream>
//#include "send.cpp"

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
	std::string room = "general";
	std::cout << "Generating ephemeral X25519 key..." << std::endl;
	std::string our_pub = server::generate_ephemeral_public_key();
	std::cout << "Our public key (Base64):\n" << our_pub << std::endl;

	std::cout << "Paste peer public key (Base64), then press Enter:\n> ";
	std::string peer_pub;
	std::cin >> peer_pub;

	server::set_room_key_from_exchange(room, peer_pub);
	std::cout << "Derived shared room key for '" << room << "'." << std::endl;

	std::cout << "Enter message to send:\n> ";
	std::string message;
	std::cin.ignore();
	std::getline(std::cin, message);

	server::send_message_room_key(message, room);
	return 0;
}