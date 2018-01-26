#include <memory>
#include "Message.hpp"

Message Message::makeMessage(Message::TypeID type, const char *src, Message::SizeInBytes size) {
    return Message(type, src, size);
}
