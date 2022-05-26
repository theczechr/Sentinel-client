#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>

namespace server
{
	void ping();
	bool is_online();
	void send_message(std::string &message, std::string& room_name);
}