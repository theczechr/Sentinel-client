#pragma once
#include <string>

namespace identity
{
	// Ensure identity key exists at path; generates new Ed25519 key if missing.
	void ensure_identity(const std::string& pem_path);

	// Returns PEM-encoded Ed25519 public key (X.509 SubjectPublicKeyInfo)
	std::string get_public_pem();

	// Returns PEM-encoded PKCS#8 private key
	std::string get_private_pem();

	// Sign an arbitrary message using the identity private key; returns signature (Base64)
	std::string sign_message(const std::string& message);

	// Verify a signature (Base64) over message using provided public key PEM
	bool verify_message(const std::string& public_pem, const std::string& message, const std::string& signature_b64);
}


