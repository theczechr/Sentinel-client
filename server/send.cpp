#include <drogon/WebSocketClient.h>
#include <drogon/PubSubService.h>
#include <drogon/HttpAppFramework.h>
using namespace std::chrono_literals;
void receive_message()
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
   req->setParameter(key,value);

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
           const drogon::WebSocketClientPtr & wsPtr) {
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