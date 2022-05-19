#include "server.hpp"

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
        });
    LOG_INFO << "Server is online";
    wsPtr->stop();
    return true;
}
