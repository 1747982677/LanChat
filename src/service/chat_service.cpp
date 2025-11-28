#include "chat_service.h"
#include "utils/logger.h"

void ChatService::sendMessage(const LanChat::Message& message)
{
    Logger::getInstance().log("Sending message from service layer");
    // TODO: Implement send message logic
    emit messageSent(message);
}

void ChatService::receiveMessage(const LanChat::Message& message)
{
    Logger::getInstance().log("Receiving message in service layer");
    // TODO: Implement receive message logic
    emit messageReceived(message);
}
