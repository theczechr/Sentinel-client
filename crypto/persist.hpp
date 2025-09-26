#pragma once
#include <string>
#include <unordered_map>

namespace persist
{
	// Simple JSON-lines like storage for keys and contacts
	// Paths default to files in working dir; caller can override
	void set_paths(const std::string& room_keys_path, const std::string& contacts_path);

	// Room keys: map room -> key_b64
	std::unordered_map<std::string, std::string> load_room_keys();
	bool save_room_key(const std::string& room, const std::string& key_b64);

	// Contacts (TOFU): map contact_id -> identity_public_pem
	std::unordered_map<std::string, std::string> load_contacts();
	bool save_contact_if_absent(const std::string& contact_id, const std::string& identity_public_pem);

	// Room-contact mapping for auto-verification
	// map room -> contact_id
	std::unordered_map<std::string, std::string> load_room_contacts();
	bool save_room_contact(const std::string& room, const std::string& contact_id);

	// Message history per room
	bool append_history(const std::string& room, const std::string& direction, const std::string& message);
	std::vector<std::string> load_history_tail(const std::string& room, size_t max_lines);
}


