#include "key_store.hpp"

namespace keystore
{
	void InMemoryKeyStore::set_room_key(const std::string& room_name, const std::string& key_b64)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		room_to_key_[room_name] = key_b64;
	}

	std::string InMemoryKeyStore::get_room_key(const std::string& room_name) const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = room_to_key_.find(room_name);
		if (it == room_to_key_.end())
			return std::string();
		return it->second;
	}
}



