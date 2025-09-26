#include "menu.hpp"
#include "../crypto/identity.hpp"
#include "../server/server.hpp"
#include "../crypto/persist.hpp"
#include <iostream>
#include <fstream>

void menu::start()
{
	int choice;
	bool choosing_path = true;

	// Load persisted room keys into in-memory keystore
	auto persisted = persist::load_room_keys();
	for (const auto& kv : persisted)
	{
		server::import_room_key(kv.first, kv.second);
	}

	while (choosing_path)
	{
		LOG_INFO << "INFO: " << "[1] Login";
		LOG_INFO << "INFO: " << "[2] Register";
		LOG_INFO << "INFO: " << "[3] Secure chat (demo)";
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
		case 3:
			secure_chat();
			choosing_path = false;
			break;
		default:
			LOG_ERROR << "ERROR: " << "Spatne cislo";
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

void menu::secure_chat()
{
	const std::string identity_path = "identity_priv.pem";
	identity::ensure_identity(identity_path);

	std::cout << "Your identity public key (PEM):\n";
	std::cout << identity::get_public_pem() << std::endl;

	std::cout << "Enter room name:\n> ";
	std::string room;
	std::getline(std::cin, room);
	if (room.empty()) room = "general";

	// Show last 10 messages for context
	auto last10 = persist::load_history_tail(room, 10);
	if (!last10.empty())
	{
		std::cout << "--- Last messages ---" << std::endl;
		for (const auto& l : last10) std::cout << l << std::endl;
		std::cout << "---------------------" << std::endl;
	}
	std::cout << "Generating ephemeral X25519..." << std::endl;
	std::string our_pub = server::generate_ephemeral_public_key();
	std::cout << "Our X25519 public (Base64):\n" << our_pub << std::endl;
	std::string our_sig = server::sign_ephemeral_pub(our_pub);
	std::cout << "Signature over our X25519 pub (Base64):\n" << our_sig << std::endl;

	std::cout << "Paste peer X25519 public (Base64):\n> ";
	std::string peer_pub;
	std::cin >> peer_pub;

	// Auto-verify using stored contact mapping if exists
	auto room_contacts = persist::load_room_contacts();
	std::string mapped_contact;
	if (room_contacts.find(room) != room_contacts.end())
		mapped_contact = room_contacts[room];

	if (!mapped_contact.empty())
	{
		// Load stored PEM for mapped contact
		auto contacts = persist::load_contacts();
		auto it = contacts.find(mapped_contact);
		if (it == contacts.end())
		{
			LOG_ERROR << "Mapped contact '" << mapped_contact << "' not found in contacts store. Falling back to manual/TOFU.";
		}
		else
		{
			std::cout << "Known contact for this room: '" << mapped_contact << "'.\n";
			std::cout << "Paste peer signature over their X25519 pub (Base64):\n> ";
			std::string peer_sig;
			std::cin.ignore();
			std::getline(std::cin, peer_sig);
			bool ok = server::verify_peer_ephemeral(it->second, peer_pub, peer_sig);
			if (!ok)
			{
				std::cout << "WARNING: Peer identity verification FAILED for contact '" << mapped_contact << "'. Aborting." << std::endl;
				LOG_ERROR << "Peer ephemeral signature verification failed (auto)";
				return;
			}
			std::cout << "Peer identity auto-verified as contact '" << mapped_contact << "'." << std::endl;
		}
	}

	if (mapped_contact.empty())
	{
		// Manual path or TOFU if no mapping exists
		std::cout << "Paste peer identity public key PEM file path (TOFU: leave blank to trust and store first seen):\n> ";
		std::string peer_identity_path;
		std::cin.ignore();
		std::getline(std::cin, peer_identity_path);
		if (!peer_identity_path.empty())
		{
			std::cout << "Paste peer signature over their X25519 pub (Base64):\n> ";
			std::string peer_sig;
			std::getline(std::cin, peer_sig);
			// Load peer identity public pem
			std::ifstream in(peer_identity_path);
			if (in.good())
			{
				std::string peer_pub_pem((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
				bool ok = server::verify_peer_ephemeral(peer_pub_pem, peer_pub, peer_sig);
				if (!ok)
				{
					std::cout << "WARNING: Peer identity verification FAILED. Aborting." << std::endl;
					LOG_ERROR << "Peer ephemeral signature verification failed (manual)";
					return;
				}
				std::cout << "Peer identity verified." << std::endl;
			}
		}
		else
		{
			// Trust on first use: save provided identity if user wants to
			std::cout << "Enter contact id to store peer identity (or leave blank to skip):\n> ";
			std::string contact_id;
			std::getline(std::cin, contact_id);
			if (!contact_id.empty())
			{
				std::cout << "Paste peer identity public key PEM (will be stored for TOFU):\n> \n";
				std::string line, pem;
				while (std::getline(std::cin, line))
				{
					if (line == "---END---") break;
					pem += line + "\n";
				}
				persist::save_contact_if_absent(contact_id, pem);
				persist::save_room_contact(room, contact_id);
				std::cout << "Stored peer identity for TOFU. Future sessions can verify." << std::endl;
			}
		}
	}
	server::set_room_key_from_exchange(room, peer_pub);

	bool running = true;
	while (running)
	{
		std::cout << "[1] Send message  [2] Receive for 30s  [0] Exit\n> ";
		int choice = 0;
		std::string line;
		std::getline(std::cin, line);
		if (!line.empty()) choice = std::stoi(line);

		switch (choice)
		{
		case 1:
		{
			std::cout << "Enter message:\n> ";
			std::string msg;
			std::getline(std::cin, msg);
			server::send_message_room_key(msg, room);
			break;
		}
		case 2:
		{
			std::cout << "Listening for messages (30s)..." << std::endl;
			server::receive_message_room_key(room);
			break;
		}
		case 0:
			running = false;
			break;
		default:
			std::cout << "Unknown choice" << std::endl;
			break;
		}
	}
}