<p align="center">
  <img src="drogon.png" alt="Sentinel Logo" height="120" />
</p>

<h2 align="center">Sentinel Client</h2>

<p align="center">
  Privacy and security-focused messaging application with end-to-end encryption
  <br />
  <em>Built with Drogon HTTP framework, WebSockets, and SQL database</em>
</p>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#building">Building</a> •
  <a href="#secure-chat">Secure Chat</a> •
  <a href="#license">License</a>
</p>

---

## About

Sentinel is a proof-of-concept, privacy and security-focused messaging application co-created as a demonstration of secure communication protocols. It features end-to-end encryption and utilizes WebSockets, the Drogon HTTP web framework, and an SQL database to provide a secure and efficient communication platform. After completing the text-based user interface and proof of concept, the project was open-sourced for the community to learn from and build upon.

## Secure Chat

### Overview

The client implements end-to-end encrypted messaging over WebSockets using Botan:

- X25519 for key exchange (ephemeral per session)
- HKDF-SHA256 to derive a 32-byte room key
- AES-256-GCM for authenticated encryption (v2 scheme)
- Ed25519 identity for authentication (optional, recommended)
- TOFU (Trust On First Use) contacts and room-to-contact mapping
- Per-room message history logs

### Running the demo

1) Build and run the client (see Building section).
2) In the menu, select `[3] Secure chat (demo)`.
3) Enter a room name. The last 10 messages for the room will be shown if present.
4) The app will:
   - Generate an ephemeral X25519 key and show your Base64 public key and its Ed25519 signature.
   - Ask for the peer's X25519 public key (Base64).
   - If the room is mapped to a contact, it will auto-verify the peer's signature using the stored identity and abort on mismatch.
   - Otherwise, you can either:
     - Provide a path to a peer identity PEM and their signature (verifies immediately), or
     - Use TOFU: enter a contact id and paste the peer's identity PEM once (end with a line containing `---END---`). The identity and room mapping will be stored for future sessions.
5) After verification/exchange, choose to send messages or listen for 30 seconds for incoming messages. Messages are encrypted with AES-256-GCM and logged to `history_<room>.log` (outgoing ciphertext and incoming plaintext).

### Files and modules

- `crypto/e2ee.*`: AES-256-GCM encryption/decryption. `v1` password-based; `v2` raw 32-byte keys.
- `crypto/key_exchange.*`: X25519 keypair generation and key agreement (HKDF-SHA256).
- `crypto/identity.*`: Ed25519 identity (generate/load PEM), sign/verify.
- `crypto/key_store.*`: In-memory per-room key store.
- `crypto/persist.*`: File-backed storage for room keys, contacts (TOFU), room-contact mapping, and room history.
- `server/server.*`: WebSocket send/receive helpers with E2EE and history integration.
- `menu/menu.*`: Secure chat demo flow (identity, exchange, verify/TOFU, send/receive loop).

### Notes

- This is a demo client; the server must transparently route ciphertext and not inspect contents.
- For production, add proper error handling, key rotation policies, and secure storage paths.
- Consider binding additional associated data (AAD) like room or sender to the AEAD if the server includes it in routing.

## Installation

### Prerequisites

- C++ compiler with C++17 support
- CMake 3.5 or higher
- vcpkg package manager

### Windows

Follow the vcpkg installation instructions [here](https://vcpkg.io/en/getting-started.html) and then:

```bash
.\vcpkg.exe install drogon:x64-windows openssl:x64-windows sqlitecpp:x64-windows botan:x64-windows
```

> **Note:** Change `x64-windows` to `x86-windows` if you have a 32-bit OS

### Linux/macOS

```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install

# Install dependencies
./vcpkg install drogon openssl sqlitecpp botan
```

## Building

```bash
# Clone the repository
git clone https://github.com/theczechr/Sentinel-client.git
cd Sentinel-client

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)
```

## License

This project is open source and available under the [MIT License](LICENSE).

## Acknowledgments

- Built with [Drogon](https://github.com/drogonframework/drogon) - A C++14/17-based HTTP application framework
- Encryption powered by [Botan](https://botan.randombit.net/) - Crypto and TLS for Modern C++
- Database operations via [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp) - A smart and easy to use C++ SQLite3 wrapper

---

<p align="center">
  Made with ❤️ for secure communication
</p>