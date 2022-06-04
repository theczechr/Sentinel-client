#include "server.hpp"
using namespace std::chrono_literals;

// Musime fixnout to aby slo volat x funkcii najednou
/*
 IDEA: Vytvorit nejaky seznam proste erroru chapes
 Napr error ze user uz existuje nebo tak
 Proste takovej seznam anebo to muzem proste normalne vypisovat to je jedno
 Muzem se tom pobavit
*/
// Jeste dalsi comment na line 215

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

void server::send_message(std::string &message, std::string &room_name)
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/test";
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
    req->setParameter(key, room_name);

    wsPtr->connectToServer(
        req,
        [message](drogon::ReqResult r,
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
                wsPtr->getConnection()->send(message, drogon::WebSocketMessageType::Text);
        });
    drogon::app().getLoop()->runAfter(15, []() { drogon::app().quit(); });
    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
    LOG_INFO << "bye!";
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

        LOG_INFO << "Response from the server '" << message << "', message type '" << messageType << "'";

        if (message[0] == '0') // No nevim jestli to je nejlepsi reseni.. prislo mi blby tam davat cely string tak na zacatku te message je "bool"
        {
            LOG_ERROR << message;
        }
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