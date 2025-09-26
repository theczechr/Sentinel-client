#pragma once
#include <string>

namespace e2ee
{
    // Encrypts plaintext using a shared secret. Returns payload in format:
    // v1:<salt_b64>:<nonce_b64>:<ciphertext_b64>
    std::string encrypt_message(const std::string& plaintext, const std::string& shared_secret);

    // Decrypts payload produced by encrypt_message. Returns plaintext on success;
    // if decryption fails, returns an empty string.
    std::string decrypt_message(const std::string& payload, const std::string& shared_secret);

	// v2: Encrypt using a raw 32-byte key (Base64-encoded input). Returns
	// v2:<nonce_b64>:<ciphertext_b64>
	std::string encrypt_message_v2(const std::string& plaintext, const std::string& key32_b64);

	// v2: Decrypt using the raw 32-byte key (Base64-encoded input).
	std::string decrypt_message_v2(const std::string& payload, const std::string& key32_b64);
}


