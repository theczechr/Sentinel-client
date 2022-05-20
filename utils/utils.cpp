#include "utils.hpp"

bool valid::username(std::string username)
{
	// Check length
	if (!(username.length() >= 1 && username.length() <= 50))
		return false;

	// Check space
	if (username.find(' ') != std::string::npos)
		return false;

	return true;
}

bool valid::email(std::string email)
{
	// Check length
	if (!(email.length() >= 5 && email.length() <= 50))
		return false;

	// Check space
	if (email.find(' ') != std::string::npos)
		return false;

	std::string special_chars = "#!~$%^&*()-_=+/\,:?|<>";

	for (char c : special_chars)
	{
		if (email.find(c) != std::string::npos)
			return false;
	}
	// Somehow check if email exist
	return true;
}

bool valid::password(std::string password)
{
	// Check length
	if (!(password.length() >= 12 && password.length() <= 50))
		return false;

	// Check space
	if (password.find(' ') != std::string::npos)
		return false;

	return true;
}

bool valid::phone(std::string phone_number)
{
	// Check length
	if (!(phone_number.length() >= 0 && phone_number.length() <= 10))
		return false;

	// Check space
	if (phone_number.find(' ') != std::string::npos)
		return false;

	// Loop through number 0-9
	for (int n = 0; n <= 9; n++)
	{
		// Convert int to str
		std::string str_nums = std::to_string(n);

		if (phone_number.find(str_nums) != std::string::npos)
			return false;
	}
	// Somehow check if number exist
	return true;
}
