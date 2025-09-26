#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

namespace keystore
{
	// Stores per-room Base64-encoded 32-byte keys
	class InMemoryKeyStore
	{
	public:
		void set_room_key(const std::string& room_name, const std::string& key_b64);
		std::string get_room_key(const std::string& room_name) const;

	private:
		mutable std::mutex mutex_;
		std::unordered_map<std::string, std::string> room_to_key_;
	};
}



