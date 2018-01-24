#include <memory>
#include "Message.hpp"

Message Message::makeMessageByAllocatingAndCopying(Message::TypeID type, const char *src, Message::SizeInBytes size) {
    return Message(type, size, src);
}

Message *Message::newMessageByAllocatingAndCopying(Message::TypeID type, const char *src, Message::SizeInBytes size) {
    return new Message(type, size, src); /*TODO: remove or change to smart ptr */
}

Message Message::makeMessageByOutsidePayload(Message::TypeID type, const std::shared_ptr<char> &payload, Message::SizeInBytes size) {
    return Message(type, size, payload);
}

Message *Message::newMessageByOutsidePayload(Message::TypeID type, const std::shared_ptr<char> &payload, Message::SizeInBytes size) {
    return new Message(type, size, payload); /*TODO: remove or change to smart ptr */
}

