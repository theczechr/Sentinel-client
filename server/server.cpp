#include "server.hpp"
#include <drogon/WebSocketClient.h>
#include <drogon/PubSubService.h>
#include <drogon/HttpAppFramework.h>
#include "../crypto/e2ee.hpp"
#include "../crypto/key_exchange.hpp"
#include "../crypto/key_store.hpp"
#include "../crypto/identity.hpp"
#include "../crypto/persist.hpp"
using namespace std::chrono_literals;

namespace {
    // Simple process-level keystore and our ephemeral private key
    keystore::InMemoryKeyStore g_room_keys;
    std::string g_private_pem;
    std::string g_public_b64;
}


void server::ping()
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/ping";
    drogon::optional<uint16_t> port = 8848;

    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;
    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();

    req->setPath(path);

    wsPtr->setMessageHandler([](const std::string& message, const drogon::WebSocketClientPtr&, const drogon::WebSocketMessageType& type)
    {
        LOG_INFO << "Received response from server '" << message << "'";
    });

    wsPtr->setConnectionClosedHandler([](const drogon::WebSocketClientPtr&)
    {
        LOG_INFO << "WebSocket connection closed!";
    });

    LOG_INFO << "Connecting to WebSocket at: " << server;
    wsPtr->connectToServer(req, [](drogon::ReqResult r, const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& wsPtr)
        {
            if (r != drogon::ReqResult::Ok)
            {
                LOG_ERROR << "Failed to establish WebSocket connection!";
                LOG_ERROR << "Request result: " << r;
                wsPtr->stop();
                return;
            }
            LOG_INFO << "WebSocket connected!";
            wsPtr->getConnection()->setPingMessage("Testing server connection", 2s);
    });
    drogon::app().getLoop()->runAfter(15, []() { drogon::app().quit(); }); // Stop after x seconds

    drogon::app().setLogLevel(trantor::Logger::kDebug);
    drogon::app().run();
}

bool server::is_online()
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/connectionTest";
    drogon::optional<uint16_t> port = 8848;

    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;
    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();

    req->setPath(path);

    wsPtr->connectToServer(req, [](drogon::ReqResult r, const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& wsPtr)
    {
        if (r != drogon::ReqResult::Ok)
        {
            LOG_ERROR << "Server is offline";
            LOG_ERROR << "Request result: " << r;
            wsPtr->stop();
            return false;
        }
        LOG_INFO << "Server is online";
        wsPtr->stop();
    });
    return true;
}

void server::send_message(std::string message, std::string room_name)
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/sendtxt";
    drogon::optional<uint16_t> port = 8848;

    const std::string key = "room_name";
    const std::string value = "hello";

    std::string serverString;

    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;

    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();

    req->setPath(path);
    req->setMethod(drogon::HttpMethod::Head);
    req->setParameter(key, room_name);

    wsPtr->connectToServer(
        req,
        [message](drogon::ReqResult r,
            const drogon::HttpResponsePtr& a,
            const drogon::WebSocketClientPtr& wsPtr) {
                if (r != drogon::ReqResult::Ok)
                {
                    LOG_ERROR << "Failed to establish WebSocket connection!";
                    LOG_ERROR << "Request result: " << r;
                    wsPtr->stop();
                    return;
                }
                LOG_INFO << "WebSocket connected!";
                
                wsPtr->getConnection()->send(message, drogon::WebSocketMessageType::Text);
                /*
                auto asd = a->getHeaders();
                for (auto str: asd)
                {
                    auto v = std::get<std::string>(str);
                    std::cout << v << std::endl;
                }
                auto cd = a->getBody();
                std::cout << cd << std::endl;
                */
                
        });
    wsPtr->setMessageHandler([](const std::string& message,
        const drogon::WebSocketClientPtr&,
        const drogon::WebSocketMessageType& type) {
            std::string messageType = "Unknown";
            if (type == drogon::WebSocketMessageType::Text)
                messageType = "text";
            else if (type == drogon::WebSocketMessageType::Pong)
                messageType = "pong";
            else if (type == drogon::WebSocketMessageType::Ping)
                messageType = "ping";
            else if (type == drogon::WebSocketMessageType::Binary)
                messageType = "binary";
            else if (type == drogon::WebSocketMessageType::Close)
                messageType = "Close";

            LOG_INFO << "new message (" << messageType << "): " << message;
        });

    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
}

void server::send_message_encrypted(std::string plaintext, std::string room_name, std::string shared_secret)
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/sendtxt";
    drogon::optional<uint16_t> port = 8848;

    std::string payload = e2ee::encrypt_message(plaintext, shared_secret);
    if (payload.empty())
    {
        LOG_ERROR << "Encryption failed";
        return;
    }

    std::string serverString;

    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;

    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();

    req->setPath(path);
    req->setMethod(drogon::HttpMethod::Head);
    req->setParameter("room_name", room_name);

    wsPtr->connectToServer(
        req,
        [payload, room_name](drogon::ReqResult r,
            const drogon::HttpResponsePtr&,
            const drogon::WebSocketClientPtr& wsPtr) {
                if (r != drogon::ReqResult::Ok)
                {
                    LOG_ERROR << "Failed to establish WebSocket connection!";
                    LOG_ERROR << "Request result: " << r;
                    wsPtr->stop();
                    return;
                }
                LOG_INFO << "WebSocket connected!";

                wsPtr->getConnection()->send(payload, drogon::WebSocketMessageType::Text);
                persist::append_history(room_name, "out", payload);
        });

    wsPtr->setMessageHandler([](const std::string& message,
        const drogon::WebSocketClientPtr&,
        const drogon::WebSocketMessageType& type) {
            std::string messageType = "Unknown";
            if (type == drogon::WebSocketMessageType::Text)
                messageType = "text";
            else if (type == drogon::WebSocketMessageType::Pong)
                messageType = "pong";
            else if (type == drogon::WebSocketMessageType::Ping)
                messageType = "ping";
            else if (type == drogon::WebSocketMessageType::Binary)
                messageType = "binary";
            else if (type == drogon::WebSocketMessageType::Close)
                messageType = "Close";

            LOG_INFO << "new message (" << messageType << "): " << message;
        });

    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
}

void server::receive_message_encrypted(std::string room_name, std::string shared_secret)
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/test";
    drogon::optional<uint16_t> port = 8848;

    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;
    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(path);
    req->setParameter("room_name", room_name);

    wsPtr->setMessageHandler([shared_secret](const std::string& message,
        const drogon::WebSocketClientPtr&,
        const drogon::WebSocketMessageType& type) {
            std::string messageType = "Unknown";
            if (type == drogon::WebSocketMessageType::Text)
                messageType = "text";
            else if (type == drogon::WebSocketMessageType::Pong)
                messageType = "pong";
            else if (type == drogon::WebSocketMessageType::Ping)
                messageType = "ping";
            else if (type == drogon::WebSocketMessageType::Binary)
                messageType = "binary";
            else if (type == drogon::WebSocketMessageType::Close)
                messageType = "Close";

            std::string plaintext = e2ee::decrypt_message(message, shared_secret);
            if (plaintext.empty())
            {
                LOG_ERROR << "Failed to decrypt incoming message";
                return;
            }

            LOG_INFO << "new message (" << messageType << "): " << plaintext;
        });

    wsPtr->setConnectionClosedHandler([](const drogon::WebSocketClientPtr&) {
        LOG_INFO << "WebSocket connection closed!";
        });

    LOG_INFO << "Connecting to WebSocket at " << server;
    wsPtr->connectToServer(
        req,
        [](drogon::ReqResult r,
            const drogon::HttpResponsePtr&,
            const drogon::WebSocketClientPtr& wsPtr) {
                if (r != drogon::ReqResult::Ok)
                {
                    LOG_ERROR << "Failed to establish WebSocket connection!";
                    wsPtr->stop();
                    return;
                }
                LOG_INFO << "WebSocket connected!";
        });

    drogon::app().getLoop()->runAfter(30, []() { drogon::app().quit(); });
    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
}

std::string server::generate_ephemeral_public_key()
{
    auto kp = kx::generate_x25519_keypair();
    g_private_pem = kp.private_pem;
    g_public_b64 = kp.public_b64;
    return g_public_b64;
}

void server::set_our_private_key_pem(const std::string& private_pem)
{
    g_private_pem = private_pem;
}

void server::set_room_key_from_exchange(const std::string& room_name, const std::string& peer_public_b64)
{
    if (g_private_pem.empty())
    {
        LOG_ERROR << "Private key is not set. Call generate_ephemeral_public_key first.";
        return;
    }
    std::string key_b64 = kx::derive_shared_key_b64(g_private_pem, peer_public_b64);
    if (key_b64.empty())
    {
        LOG_ERROR << "Failed to derive shared key";
        return;
    }
    g_room_keys.set_room_key(room_name, key_b64);
    persist::save_room_key(room_name, key_b64);
}

void server::send_message_room_key(std::string plaintext, std::string room_name)
{
    std::string key_b64 = g_room_keys.get_room_key(room_name);
    if (key_b64.empty())
    {
        LOG_ERROR << "No key set for room '" << room_name << "'";
        return;
    }
    std::string payload = e2ee::encrypt_message_v2(plaintext, key_b64);
    if (payload.empty())
    {
        LOG_ERROR << "Encryption failed";
        return;
    }

    std::string server = "ws://127.0.0.1";
    std::string path = "/sendtxt";
    drogon::optional<uint16_t> port = 8848;

    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;

    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(path);
    req->setMethod(drogon::HttpMethod::Head);
    req->setParameter("room_name", room_name);

    wsPtr->connectToServer(
        req,
        [payload](drogon::ReqResult r,
            const drogon::HttpResponsePtr&,
            const drogon::WebSocketClientPtr& wsPtr) {
                if (r != drogon::ReqResult::Ok)
                {
                    LOG_ERROR << "Failed to establish WebSocket connection!";
                    LOG_ERROR << "Request result: " << r;
                    wsPtr->stop();
                    return;
                }
                LOG_INFO << "WebSocket connected!";
                wsPtr->getConnection()->send(payload, drogon::WebSocketMessageType::Text);
        });

    wsPtr->setMessageHandler([](const std::string& message,
        const drogon::WebSocketClientPtr&,
        const drogon::WebSocketMessageType& type) {
            std::string messageType = "Unknown";
            if (type == drogon::WebSocketMessageType::Text)
                messageType = "text";
            else if (type == drogon::WebSocketMessageType::Pong)
                messageType = "pong";
            else if (type == drogon::WebSocketMessageType::Ping)
                messageType = "ping";
            else if (type == drogon::WebSocketMessageType::Binary)
                messageType = "binary";
            else if (type == drogon::WebSocketMessageType::Close)
                messageType = "Close";
            LOG_INFO << "new message (" << messageType << "): " << message;
        });

    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
}

void server::receive_message_room_key(std::string room_name)
{
    std::string key_b64 = g_room_keys.get_room_key(room_name);
    if (key_b64.empty())
    {
        LOG_ERROR << "No key set for room '" << room_name << "'";
        return;
    }

    std::string server = "ws://127.0.0.1";
    std::string path = "/test";
    drogon::optional<uint16_t> port = 8848;

    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;
    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(path);
    req->setParameter("room_name", room_name);

    wsPtr->setMessageHandler([key_b64, room_name](const std::string& message,
        const drogon::WebSocketClientPtr&,
        const drogon::WebSocketMessageType& type) {
            std::string messageType = "Unknown";
            if (type == drogon::WebSocketMessageType::Text)
                messageType = "text";
            else if (type == drogon::WebSocketMessageType::Pong)
                messageType = "pong";
            else if (type == drogon::WebSocketMessageType::Ping)
                messageType = "ping";
            else if (type == drogon::WebSocketMessageType::Binary)
                messageType = "binary";
            else if (type == drogon::WebSocketMessageType::Close)
                messageType = "Close";

            std::string plaintext = e2ee::decrypt_message_v2(message, key_b64);
            if (plaintext.empty())
            {
                LOG_ERROR << "Failed to decrypt incoming message";
                return;
            }
            LOG_INFO << "new message (" << messageType << "): " << plaintext;
            persist::append_history(room_name, "in", plaintext);
        });

    wsPtr->setConnectionClosedHandler([](const drogon::WebSocketClientPtr&) {
        LOG_INFO << "WebSocket connection closed!";
        });

    LOG_INFO << "Connecting to WebSocket at " << server;
    wsPtr->connectToServer(
        req,
        [](drogon::ReqResult r,
            const drogon::HttpResponsePtr&,
            const drogon::WebSocketClientPtr& wsPtr) {
                if (r != drogon::ReqResult::Ok)
                {
                    LOG_ERROR << "Failed to establish WebSocket connection!";
                    wsPtr->stop();
                    return;
                }
                LOG_INFO << "WebSocket connected!";
        });

    drogon::app().getLoop()->runAfter(30, []() { drogon::app().quit(); });
    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
}

std::string server::sign_ephemeral_pub(const std::string& ephemeral_pub_b64)
{
    return identity::sign_message(ephemeral_pub_b64);
}

bool server::verify_peer_ephemeral(const std::string& peer_identity_pub_pem, const std::string& peer_ephemeral_pub_b64, const std::string& signature_b64)
{
    return identity::verify_message(peer_identity_pub_pem, peer_ephemeral_pub_b64, signature_b64);
}

void server::import_room_key(const std::string& room_name, const std::string& key_b64)
{
    if (room_name.empty() || key_b64.empty()) return;
    g_room_keys.set_room_key(room_name, key_b64);
}

void server::create_account(std::string username, std::string email_hash, std::string password_hash, std::string phone_hash)
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/register";
    drogon::optional<uint16_t> port = 8848;

    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;

    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();

    req->setPath(path);
    req->setMethod(drogon::HttpMethod::Head);
    req->setParameter("username", username);
    req->setParameter("email_hash", email_hash);
    req->setParameter("password_hash", password_hash);
    req->setParameter("phone_hash", phone_hash);

    wsPtr->setMessageHandler([](const std::string& message, const drogon::WebSocketClientPtr&, const drogon::WebSocketMessageType& type)
    {
        LOG_INFO << "Response from the server '" << message << "'";
    });

    wsPtr->setConnectionClosedHandler([](const drogon::WebSocketClientPtr&)
    {
        LOG_INFO << "WebSocket connection closed!";
    });

    LOG_INFO << "Connecting to WebSocket at: " << server;
    wsPtr->connectToServer(req, [](drogon::ReqResult r, const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& wsPtr)
        {
            if (r != drogon::ReqResult::Ok)
            {
                LOG_ERROR << "Failed to establish WebSocket connection!";
                LOG_ERROR << "Request result: " << r;
                wsPtr->stop();
                return;
            }
            LOG_INFO << "WebSocket connected!";
        });

    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
}

void server::login_account(std::string username, std::string password_hash)
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/login";
    drogon::optional<uint16_t> port = 8848;

    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;

    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();

    req->setPath(path);
    req->setMethod(drogon::HttpMethod::Head);
    req->setParameter("username", username);
    req->setParameter("password_hash", password_hash);

    wsPtr->setMessageHandler([](const std::string& message, const drogon::WebSocketClientPtr&, const drogon::WebSocketMessageType& type)
    {
        LOG_INFO << "Response from the server '" << message << "'";

        if (message[0] == '0') // No nevim jestli to je nejlepsi reseni.. prislo mi blby tam davat cely string tak na zacatku te message je "bool"
            LOG_ERROR << message;
    });

    wsPtr->setConnectionClosedHandler([](const drogon::WebSocketClientPtr&)
    {
        LOG_INFO << "WebSocket connection closed!";
    });

    LOG_INFO << "Connecting to WebSocket at: " << server;
    wsPtr->connectToServer(req, [](drogon::ReqResult r, const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& wsPtr)
    {
        if (r != drogon::ReqResult::Ok)
        {
            LOG_ERROR << "Failed to establish WebSocket connection!";
            LOG_ERROR << "Request result: " << r;
            wsPtr->stop();
            return;
        }
        LOG_INFO << "WebSocket connected!";
    });

    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
}

void server::change_username(std::string& old_username, std::string& new_username, std::string& password_hash)
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/login";
    drogon::optional<uint16_t> port = 8848;

    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;

    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();

    req->setPath(path);
    req->setMethod(drogon::HttpMethod::Head);
    req->setParameter("username", old_username);
    req->setParameter("password_hash", password_hash);
    req->setParameter("new_username", new_username);

    wsPtr->setMessageHandler([](const std::string& message, const drogon::WebSocketClientPtr&, const drogon::WebSocketMessageType& type)
        {
            LOG_INFO << "Response from the server '" << message << "'";

            if (message[0] == '0') // No nevim jestli to je nejlepsi reseni.. prislo mi blby tam davat cely string tak na zacatku te message je "bool"
                LOG_ERROR << message;
        });

    wsPtr->setConnectionClosedHandler([](const drogon::WebSocketClientPtr&)
        {
            LOG_INFO << "WebSocket connection closed!";
        });

    LOG_INFO << "Connecting to WebSocket at: " << server;
    wsPtr->connectToServer(req, [](drogon::ReqResult r, const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& wsPtr)
        {
            if (r != drogon::ReqResult::Ok)
            {
                LOG_ERROR << "Failed to establish WebSocket connection!";
                LOG_ERROR << "Request result: " << r;
                wsPtr->stop();
                return;
            }
            LOG_INFO << "WebSocket connected!";
        });

    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
}