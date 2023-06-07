#include "webServerMessageInterface.hpp"
#include <iostream>
#include "../util/macros.hpp"

WebServerMessageInterface::WebServerMessageInterface()
{ }

WebServerMessageInterface::~WebServerMessageInterface() {}

void WebServerMessageInterface::processMessage(broadcast_server* s, websocketpp::connection_hdl hdl, std::string msg)
{
    LOG_INFO("Processing ["<< msg <<"]");

    // Separate message header from message data
    auto parsedMsg = parseMessage(msg);
    
    // Process message
    if(!parsedMsg.empty())
    {
        auto it = messageProcessingFunctions.find(parsedMsg[0]);
        if(it != messageProcessingFunctions.end())
            messageProcessingFunctions[parsedMsg[0]](s, hdl, parsedMsg, status);
    }
    LOG_INFO("Message ["<<parsedMsg[0]<<"] processed");
}
