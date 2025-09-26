#include "e2ee.hpp"
#include <botan/auto_rng.h>
#include <botan/aead.h>
#include <botan/base64.h>
#include <botan/pbkdf2.h>
#include <botan/hex.h>
#include <sstream>
#include <vector>

namespace
{
	const char* kVersionPrefix = "v1";
	const char* kVersionPrefixV2 = "v2";
	const size_t kSaltSizeBytes = 16; // 128-bit salt
	const size_t kNonceSizeBytes = 12; // 96-bit nonce for GCM
	const size_t kKeySizeBytes = 32; // 256-bit AES key
	const size_t kPbkdf2Iterations = 200000; // sensible default

	std::vector<std::string> split(const std::string& str, char delimiter)
	{
		std::vector<std::string> parts;
		std::stringstream ss(str);
		std::string item;
		while (std::getline(ss, item, delimiter))
		{
			parts.push_back(item);
		}
		return parts;
	}
}

namespace e2ee
{
	std::string encrypt_message(const std::string& plaintext, const std::string& shared_secret)
	{
		try
		{
			Botan::AutoSeeded_RNG rng;
			Botan::secure_vector<uint8_t> salt = rng.random_vec(kSaltSizeBytes);

			auto pbkdf = Botan::PBKDF::create("PBKDF2(SHA-256)");
			if (!pbkdf)
			{
				return std::string();
			}
			Botan::secure_vector<uint8_t> derived_key = pbkdf->derive_key(
				kKeySizeBytes,
				shared_secret,
				salt.data(),
				salt.size(),
				kPbkdf2Iterations);

			auto enc = Botan::AEAD_Mode::create("AES-256/GCM", Botan::ENCRYPTION);
			if (!enc)
			{
				return std::string();
			}
			enc->set_key(derived_key);

			Botan::secure_vector<uint8_t> nonce = rng.random_vec(kNonceSizeBytes);
			enc->start(nonce.data(), nonce.size());

			Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
			enc->finish(buffer, 0);

			std::string salt_b64 = Botan::base64_encode(salt.data(), salt.size());
			std::string nonce_b64 = Botan::base64_encode(nonce.data(), nonce.size());
			std::string ciphertext_b64 = Botan::base64_encode(buffer.data(), buffer.size());

			std::string payload = std::string(kVersionPrefix) + ":" + salt_b64 + ":" + nonce_b64 + ":" + ciphertext_b64;
			return payload;
		}
		catch (...)
		{
			return std::string();
		}
	}

	std::string decrypt_message(const std::string& payload, const std::string& shared_secret)
	{
		try
		{
			auto parts = split(payload, ':');
			if (parts.size() != 4)
			{
				return std::string();
			}
			if (parts[0] != kVersionPrefix)
			{
				return std::string();
			}

			std::vector<uint8_t> salt = Botan::base64_decode(parts[1]);
			std::vector<uint8_t> nonce = Botan::base64_decode(parts[2]);
			std::vector<uint8_t> ciphertext = Botan::base64_decode(parts[3]);

			auto pbkdf = Botan::PBKDF::create("PBKDF2(SHA-256)");
			if (!pbkdf)
			{
				return std::string();
			}
			Botan::secure_vector<uint8_t> derived_key = pbkdf->derive_key(
				kKeySizeBytes,
				shared_secret,
				salt.data(),
				salt.size(),
				kPbkdf2Iterations);

			auto dec = Botan::AEAD_Mode::create("AES-256/GCM", Botan::DECRYPTION);
			if (!dec)
			{
				return std::string();
			}
			dec->set_key(derived_key);
			dec->start(nonce.data(), nonce.size());

			Botan::secure_vector<uint8_t> buffer(ciphertext.begin(), ciphertext.end());
			dec->finish(buffer, 0);

			return std::string(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		}
		catch (...)
		{
			return std::string();
		}
	}
}

	std::string encrypt_message_v2(const std::string& plaintext, const std::string& key32_b64)
	{
		try
		{
			Botan::AutoSeeded_RNG rng;
			std::vector<uint8_t> key_raw = Botan::base64_decode(key32_b64);
			if (key_raw.size() != kKeySizeBytes)
				return std::string();

			auto enc = Botan::AEAD_Mode::create("AES-256/GCM", Botan::ENCRYPTION);
			if (!enc)
				return std::string();
			enc->set_key(key_raw);

			Botan::secure_vector<uint8_t> nonce = rng.random_vec(kNonceSizeBytes);
			enc->start(nonce.data(), nonce.size());

			Botan::secure_vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
			enc->finish(buffer, 0);

			std::string nonce_b64 = Botan::base64_encode(nonce.data(), nonce.size());
			std::string ciphertext_b64 = Botan::base64_encode(buffer.data(), buffer.size());
			return std::string(kVersionPrefixV2) + ":" + nonce_b64 + ":" + ciphertext_b64;
		}
		catch (...)
		{
			return std::string();
		}
	}

	std::string decrypt_message_v2(const std::string& payload, const std::string& key32_b64)
	{
		try
		{
			auto parts = split(payload, ':');
			if (parts.size() != 3)
				return std::string();
			if (parts[0] != kVersionPrefixV2)
				return std::string();

			std::vector<uint8_t> key_raw = Botan::base64_decode(key32_b64);
			if (key_raw.size() != kKeySizeBytes)
				return std::string();

			std::vector<uint8_t> nonce = Botan::base64_decode(parts[1]);
			std::vector<uint8_t> ciphertext = Botan::base64_decode(parts[2]);

			auto dec = Botan::AEAD_Mode::create("AES-256/GCM", Botan::DECRYPTION);
			if (!dec)
				return std::string();
			dec->set_key(key_raw);
			dec->start(nonce.data(), nonce.size());

			Botan::secure_vector<uint8_t> buffer(ciphertext.begin(), ciphertext.end());
			dec->finish(buffer, 0);

			return std::string(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		}
		catch (...)
		{
			return std::string();
		}
	}
}


