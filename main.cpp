#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>

#include <iostream>

using namespace std::chrono_literals;

int main(int argc, char* argv[])
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
                wsPtr->getConnection()->setPingMessage("", 2s);
                wsPtr->getConnection()->send("hello!");
        });

    // Quit the application after 15 seconds
    drogon::app().getLoop()->runAfter(15, []() { drogon::app().quit(); });

    drogon::app().setLogLevel(trantor::Logger::kDebug);
    drogon::app().run();
    LOG_INFO << "bye!";
    return 0;
}
