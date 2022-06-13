#include "menu.hpp"

void menu::start()
{
	int choice;
	bool choosing_path = true;

	while (choosing_path)
	{
		LOG_INFO << "INFO: " << "[1] Login";
		LOG_INFO << "INFO: " << "[2] Register";
		std::cin >> choice;

		switch (choice)
		{
		case 1:
			login();
			choosing_path = false;
			break;
		case 2:
			create();
			choosing_path = false;
			break;
		default:

			LOG_ERROR << "ERROR: " << "Spatne cislo\n";

			choosing_path = false;
			break;
		}
	}
}

void menu::create()
{
	std::string username;
	std::string email;
	std::string password;
	std::string phone_number;

	LOG_INFO << "INFO: " << "* - required";
	std::cout << "Enter username (1-50)*: ";
	std::cin >> username;
	std::cout << "Enter email (5-50): ";
	std::cin >> email;
	std::cout << "Enter password (12-50)*: ";
	std::cin >> password;
	std::cout << "Enter phone number (9-10): ";
	std::cin >> phone_number;
	
	Botan::AutoSeeded_RNG rng;
	/*
	* DOCS k botanu:
	* * https://botan.randombit.net/handbook/botan.pdf
	* * https://botan.randombit.net/handbook/
	*/
	email = Botan::generate_bcrypt(email, rng);
	password = Botan::generate_bcrypt(password, rng);
	phone_number = Botan::generate_bcrypt(phone_number, rng);

	server::create_account(username, email, password, phone_number);

	/* 
	* Vypise ty hashe
	LOG_DEBUG << email;
	LOG_DEBUG << password;
	LOG_DEBUG << phone_number;

	* Returnuje to, jestli to matchuje
	* Jenom jsem zkousel jestli to funguje
	LOG_INFO << Botan::check_bcrypt("karlos@protonmail.com", "$2a$12$EZO38XCgdPikYUng7l5np.KkxUB45OpsHgNI3dM1MDJn6t1hxw9lq");
	LOG_INFO << Botan::check_bcrypt("sdag12hd1dh1d^*&(", "$2a$12$1gOINTpsQm1yaj7WtKCsv.tNgHcfwFOTRPnWPmW8z2p8H7/cxsbd2");
	LOG_INFO << Botan::check_bcrypt("789123658", "$2a$12$qDyufSzMRl.N/GkFWLMU3.HPRSKYWCWAJIRKhMKiwsJ.rRYs157B.");
	*/
}

void menu::login()
{
	Botan::AutoSeeded_RNG rng;

	std::string username;
	std::string password;

	LOG_INFO << "ERROR: " << "Leave blank if none";
	std::cout << "Enter username: ";
	std::cin >> username;
	std::cout << "Enter password: ";
	std::cin >> password;

	password = Botan::generate_bcrypt(password, rng);
	server::login_account(username, password);
}