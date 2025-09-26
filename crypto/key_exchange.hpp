#pragma once
#include <string>

namespace kx
{
	struct Keypair
	{
		std::string private_pem; // PKCS#8 PEM encoded private key
		std::string public_b64;  // Base64-encoded 32-byte X25519 public key
	};

	// Generates a new ephemeral X25519 keypair.
	Keypair generate_x25519_keypair();

	// Derives a shared 32-byte key using our private key (PEM) and peer public (b64).
	// Returns Base64-encoded 32-byte key using HKDF(SHA-256).
	std::string derive_shared_key_b64(const std::string& private_pem, const std::string& peer_public_b64);
}



