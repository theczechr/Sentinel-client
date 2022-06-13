#include "server.hpp"
#include <drogon/WebSocketClient.h>
#include <drogon/PubSubService.h>
#include <drogon/HttpAppFramework.h>
using namespace std::chrono_literals;

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
        LOG_INFO << "Sent message (ping): " << message;
    });

    wsPtr->setConnectionClosedHandler([](const drogon::WebSocketClientPtr&)
    {
        LOG_INFO << "WebSocket connection closed!";
    });

    LOG_INFO << "Connecting to WebSocket at " << server;
    wsPtr->connectToServer(req, [](drogon::ReqResult r, const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& wsPtr)
        {
            if (r != drogon::ReqResult::Ok)
            {
                LOG_ERROR << "Failed to establish WebSocket connection!";
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

    wsPtr->connectToServer(req, [](drogon::ReqResult r, const drogon::HttpResponsePtr&, const drogon::WebSocketClientPtr& wsPtr)
        {
            if (r != drogon::ReqResult::Ok)
            {
                LOG_ERROR << "Server is offline";
                wsPtr->stop();
                return false;
            }
            else
            {
                LOG_INFO << "Server is online";
                wsPtr->stop();
            }

        });
    return false;
}
void server::send_message(std::string &message, std::string &room_name)
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/connectionTest";
    drogon::optional<uint16_t> port = 8848;
    const std::string key = "room_name";
    const std::string value = "test";
    std::string serverString;

    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;

    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(path);
    req->setParameter(key, room_name);
    std::string cb;
    wsPtr->connectToServer(
        req,
        [message, cb](drogon::ReqResult r,
            const drogon::HttpResponsePtr& a,
            const drogon::WebSocketClientPtr& wsPtr) {
                if (r != drogon::ReqResult::Ok)
                {
                    LOG_ERROR << "Failed to establish WebSocket connection!";
                    wsPtr->stop();
                    return;
                }
                LOG_INFO << "WebSocket connected!";
                wsPtr->getConnection()->send(message, drogon::WebSocketMessageType::Text);
                auto asd = a->getHeaders();
                for (auto str: asd)
                {
                    auto v = std::get<std::string>(str);
                    std::cout << v << std::endl;
                }
                auto cd = a->getBody();
                std::cout << cd << std::endl;
                
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

    drogon::app().getLoop()->runAfter(15, []() { drogon::app().quit(); });
    drogon::app().setLogLevel(trantor::Logger::kInfo);
    drogon::app().run();
    LOG_INFO << "bye!";
}
void server::receive_message()
{
    std::string server = "ws://127.0.0.1";
    std::string path = "/test";
    drogon::optional<uint16_t> port = 8848;
    const std::string key = "room_name";
    const std::string value = "Karel";
    std::string serverString;
    if (port.value_or(0) != 0)
        serverString = server + ":" + std::to_string(port.value());
    else
        serverString = server;
    auto wsPtr = drogon::WebSocketClient::newWebSocketClient(serverString);
    auto req = drogon::HttpRequest::newHttpRequest();

    req->setPath(path);
    req->setParameter(key, value);
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
    LOG_INFO << "bye!";
}
