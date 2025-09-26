#include "key_exchange.hpp"
#include <botan/auto_rng.h>
#include <botan/pubkey.h>
#include <botan/pk_keys.h>
#include <botan/pkcs8.h>
#include <botan/x25519.h>
#include <botan/hash.h>
#include <botan/kdf.h>
#include <botan/base64.h>

namespace kx
{
	Keypair generate_x25519_keypair()
	{
		Botan::AutoSeeded_RNG rng;
		Botan::X25519_PrivateKey priv(rng);
		std::string priv_pem = Botan::PKCS8::PEM_encode(priv);
		std::vector<uint8_t> pub_raw = priv.public_value(); // 32 bytes
		std::string pub_b64 = Botan::base64_encode(pub_raw);
		return Keypair{ priv_pem, pub_b64 };
	}

	std::string derive_shared_key_b64(const std::string& private_pem, const std::string& peer_public_b64)
	{
		try
		{
			Botan::AutoSeeded_RNG rng;
			std::unique_ptr<Botan::Private_Key> priv(Botan::PKCS8::load_key(private_pem));
			auto* xpriv = dynamic_cast<Botan::X25519_PrivateKey*>(priv.get());
			if (!xpriv)
				return std::string();

			std::vector<uint8_t> peer_pub = Botan::base64_decode(peer_public_b64);
			Botan::X25519_PublicKey pub(peer_pub);

			Botan::PK_Key_Agreement ka(*xpriv, rng, "Raw");
			Botan::secure_vector<uint8_t> shared = ka.derive_key(0, pub.public_value()).bits_of();

			auto kdf = Botan::KDF::create("HKDF(SHA-256)");
			if (!kdf)
				return std::string();
			std::vector<uint8_t> salt; // optional empty salt
			std::vector<uint8_t> info; // optional context
			std::vector<uint8_t> okm = kdf->kdf(32, shared, salt, info);
			return Botan::base64_encode(okm);
		}
		catch (...)
		{
			return std::string();
		}
	}
}



