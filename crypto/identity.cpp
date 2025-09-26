#include "identity.hpp"
#include <botan/auto_rng.h>
#include <botan/pubkey.h>
#include <botan/pkcs8.h>
#include <botan/ed25519.h>
#include <botan/base64.h>
#include <fstream>

namespace {
	std::string g_priv_pem;
	std::string g_pub_pem;
}

namespace identity
{
	void ensure_identity(const std::string& pem_path)
	{
		if (!g_priv_pem.empty() && !g_pub_pem.empty())
			return;

		// Try load
		{
			std::ifstream in(pem_path);
			if (in.good())
			{
				g_priv_pem.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
				try
				{
					std::unique_ptr<Botan::Private_Key> priv(Botan::PKCS8::load_key(g_priv_pem));
					Botan::Ed25519_PrivateKey* epriv = dynamic_cast<Botan::Ed25519_PrivateKey*>(priv.get());
					if (epriv)
					{
						Botan::Ed25519_PublicKey pub(epriv->public_key_bits());
						g_pub_pem = pub.subject_public_key_info();
						return;
					}
				}
				catch (...){}
			}
		}

		Botan::AutoSeeded_RNG rng;
		Botan::Ed25519_PrivateKey priv(rng);
		g_priv_pem = Botan::PKCS8::PEM_encode(priv);
		Botan::Ed25519_PublicKey pub(priv.public_key_bits());
		g_pub_pem = pub.subject_public_key_info();

		std::ofstream out(pem_path);
		out << g_priv_pem;
	}

	std::string get_public_pem()
	{
		return g_pub_pem;
	}

	std::string get_private_pem()
	{
		return g_priv_pem;
	}
}

std::string identity::sign_message(const std::string& message)
{
	try
	{
		if (g_priv_pem.empty()) return std::string();
		Botan::AutoSeeded_RNG rng;
		std::unique_ptr<Botan::Private_Key> priv(Botan::PKCS8::load_key(g_priv_pem));
		auto* epriv = dynamic_cast<Botan::Ed25519_PrivateKey*>(priv.get());
		if (!epriv) return std::string();
		Botan::PK_Signer signer(*epriv, rng, "Pure");
		auto sig = signer.sign_message(reinterpret_cast<const uint8_t*>(message.data()), message.size(), rng);
		return Botan::base64_encode(sig);
	}
	catch (...) { return std::string(); }
}

bool identity::verify_message(const std::string& public_pem, const std::string& message, const std::string& signature_b64)
{
	try
	{
		Botan::DataSource_Memory ds(public_pem);
		std::unique_ptr<Botan::Public_Key> pub(Botan::X509::load_key(ds));
		auto* epub = dynamic_cast<Botan::Ed25519_PublicKey*>(pub.get());
		if (!epub) return false;
		Botan::PK_Verifier verifier(*epub, "Pure");
		auto sig = Botan::base64_decode(signature_b64);
		return verifier.verify_message(reinterpret_cast<const uint8_t*>(message.data()), message.size(), sig);
	}
	catch (...) { return false; }
}


