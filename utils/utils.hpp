#pragma once
#include <string>

namespace valid
{
	// Kdyztak se na to podivej, jestli to je udelany dobre
	bool username(std::string username);
	bool email(std::string email);
	bool password(std::string password);
	bool phone(std::string phone_number);
}

namespace create
{
	std::string hash(std::string item);
	std::string recovery_phrase(int lenght);
}