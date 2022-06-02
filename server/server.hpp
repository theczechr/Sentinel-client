#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>

namespace server
{
	void ping();
	bool is_online();
	// Tyto funkce by chtelo udelat, ale nejdriv musime udelat db na serveru
	void check_account_existation(std::string username, std::string email_hash, std::string password_hash, std::string phone_hash);
	void create_account(std::string username, std::string email_hash, std::string password_hash, std::string phone_hash);
	void login_account(std::string username, std::string password_hash);
	void send_message(std::string &message, std::string &room_name);
}