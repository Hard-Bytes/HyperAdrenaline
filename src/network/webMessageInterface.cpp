#include "webMessageInterface.hpp"
#include "../context/gameContext.hpp"

WebMessageInterface::WebMessageInterface()
{}

WebMessageInterface::~WebMessageInterface() {}

/*Queue Message*/
// Adds a message to the processing queue
void WebMessageInterface::queueMessage(std::string msg) noexcept
{
    // Add the new message
    messageQueue.push(msg);
}

/*Process All Messages*/
// Process every message in the queue
void WebMessageInterface::processAllMessages() noexcept
{
    while(!messageQueue.empty())
    {
        std::string next = messageQueue.front();
        messageQueue.pop();
        processMessage(next);
    }
}

/*Process Message*/
// Process a single message
void WebMessageInterface::processMessage(std::string msg) noexcept
{
    // Separate message name from message data
    auto parsedMsg = parseMessage(msg);

    //if(parsedMsg[0] == "EntityMoved") LOG_ONLINE("To process: [" << msg << "]");

    // Call the function
    if(!parsedMsg.empty())
    {
        auto it = messageProcessingFunctions.find(parsedMsg[0]);
        if(it != messageProcessingFunctions.end())
            messageProcessingFunctions[parsedMsg[0]](gctx, parsedMsg);
    }
}

void WebMessageInterface::setGameContext(GameContext* gc) noexcept
{
    gctx = gc;
}
