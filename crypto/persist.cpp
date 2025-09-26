#include "persist.hpp"
#include <fstream>
#include <sstream>
#include <vector>

namespace {
	std::string g_room_keys_path = "room_keys.txt";
	std::string g_contacts_path = "contacts.txt";
	std::string g_room_contacts_path = "room_contacts.txt";
}

namespace persist
{
	void set_paths(const std::string& room_keys_path, const std::string& contacts_path)
	{
		g_room_keys_path = room_keys_path;
		g_contacts_path = contacts_path;
	}

	std::unordered_map<std::string, std::string> load_room_keys()
	{
		std::unordered_map<std::string, std::string> res;
		std::ifstream in(g_room_keys_path);
		std::string line;
		while (std::getline(in, line))
		{
			auto pos = line.find('=');
			if (pos == std::string::npos) continue;
			std::string room = line.substr(0, pos);
			std::string key = line.substr(pos + 1);
			if (!room.empty() && !key.empty()) res[room] = key;
		}
		return res;
	}

	bool save_room_key(const std::string& room, const std::string& key_b64)
	{
		// Append or rewrite existing line naive approach: reload + overwrite file
		auto map = load_room_keys();
		map[room] = key_b64;
		std::ofstream out(g_room_keys_path, std::ios::trunc);
		for (const auto& kv : map)
		{
			out << kv.first << "=" << kv.second << "\n";
		}
		return true;
	}

	std::unordered_map<std::string, std::string> load_contacts()
	{
		std::unordered_map<std::string, std::string> res;
		std::ifstream in(g_contacts_path);
		std::string id;
		std::stringstream pem;
		while (std::getline(in, id))
		{
			std::string line;
			pem.str("");
			pem.clear();
			while (std::getline(in, line))
			{
				if (line == "---END---") break;
				pem << line << "\n";
			}
			if (!id.empty()) res[id] = pem.str();
		}
		return res;
	}

	bool save_contact_if_absent(const std::string& contact_id, const std::string& identity_public_pem)
	{
		auto map = load_contacts();
		if (map.find(contact_id) != map.end()) return true;
		std::ofstream out(g_contacts_path, std::ios::app);
		out << contact_id << "\n" << identity_public_pem;
		out << "---END---\n";
		return true;
	}

	std::unordered_map<std::string, std::string> load_room_contacts()
	{
		std::unordered_map<std::string, std::string> res;
		std::ifstream in(g_room_contacts_path);
		std::string line;
		while (std::getline(in, line))
		{
			auto pos = line.find('=');
			if (pos == std::string::npos) continue;
			std::string room = line.substr(0, pos);
			std::string contact = line.substr(pos + 1);
			if (!room.empty() && !contact.empty()) res[room] = contact;
		}
		return res;
	}

	bool save_room_contact(const std::string& room, const std::string& contact_id)
	{
		auto map = load_room_contacts();
		map[room] = contact_id;
		std::ofstream out(g_room_contacts_path, std::ios::trunc);
		for (const auto& kv : map)
		{
			out << kv.first << "=" << kv.second << "\n";
		}
		return true;
	}
}

bool persist::append_history(const std::string& room, const std::string& direction, const std::string& message)
{
	std::string path = "history_" + room + ".log";
	std::ofstream out(path, std::ios::app);
	if (!out.good()) return false;
	out << direction << ": " << message << "\n";
	return true;
}

std::vector<std::string> persist::load_history_tail(const std::string& room, size_t max_lines)
{
	std::vector<std::string> lines;
	std::string path = "history_" + room + ".log";
	std::ifstream in(path);
	if (!in.good()) return lines;
	std::string line;
	while (std::getline(in, line))
	{
		lines.push_back(line);
	}
	if (lines.size() <= max_lines) return lines;
	return std::vector<std::string>(lines.end() - max_lines, lines.end());
}


