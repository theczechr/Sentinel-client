#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>

namespace server
{
	void ping();
	bool is_online();
	void create_account(std::string username, std::string email_hash, std::string password_hash, std::string phone_hash);
	void login_account(std::string username, std::string password_hash);
	void change_username(std::string& old_username, std::string& new_username, std::string& password_hash);
	void send_message(std::string message, std::string room_name);
}