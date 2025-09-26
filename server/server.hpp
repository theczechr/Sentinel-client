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
	void send_message_encrypted(std::string plaintext, std::string room_name, std::string shared_secret);
	void receive_message_encrypted(std::string room_name, std::string shared_secret);

	// Key management helpers
	std::string generate_ephemeral_public_key();
	void set_our_private_key_pem(const std::string& private_pem);
	void set_room_key_from_exchange(const std::string& room_name, const std::string& peer_public_b64);
	void send_message_room_key(std::string plaintext, std::string room_name);
	void receive_message_room_key(std::string room_name);

	// Ephemeral signing helpers (sign/verify the X25519 base64 key using Ed25519 identity)
	std::string sign_ephemeral_pub(const std::string& ephemeral_pub_b64);
	bool verify_peer_ephemeral(const std::string& peer_identity_pub_pem, const std::string& peer_ephemeral_pub_b64, const std::string& signature_b64);

	// Import a persisted room key directly into keystore
	void import_room_key(const std::string& room_name, const std::string& key_b64);
}