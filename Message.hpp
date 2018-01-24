#ifndef EVENT_MANAGER_MESSAGE_HPP
#define EVENT_MANAGER_MESSAGE_HPP

#include <ostream>
#include <cstring>

/* can be event or task */
class Message {
public:
    using TypeID = uint32_t;
    using SizeInBytes = uint32_t;

    struct Header {
        Header() {}
        Header(TypeID type, SizeInBytes length) : type(type), length(length) {}
        TypeID type; /* TODO: enum?*/
        SizeInBytes length;
    };

    Message(TypeID type, SizeInBytes length, const char *src) : header(type, length), payload(new char[length]) {
        memcpy(payload.get(), src, length); /* TODO: too many copy operation of raw data. reduce them.*/
    }

    Message(TypeID type, SizeInBytes length, const std::shared_ptr<char> &payload) : header(type, length), payload(payload) {}

    ~Message() {
        /* payload will be deleted automatically. */
    }

    static Message makeMessageByAllocatingAndCopying(TypeID type, const char *src, SizeInBytes size) {
        return Message(type, size, src);
    }

    static Message* newMessageByAllocatingAndCopying(TypeID type, const char *src, SizeInBytes size) {
        return new Message(type, size, src); /*TODO: remove or change to smart ptr */
    }

    static Message makeMessageByOutsidePayload(TypeID type, const std::shared_ptr<char> &payload, SizeInBytes size) {
        return Message(type, size, payload);
    }

    static Message* newMessageByOutsidePayload(TypeID type, const std::shared_ptr<char> &payload, SizeInBytes size) {
        return new Message(type, size, payload); /*TODO: remove or change to smart ptr */
    }

    TypeID getID() const { return header.type; }
    SizeInBytes getPayloadSize() const { return header.length; }
    char *getPayload() const { return payload.get(); } /*TODO: remove .get() change return type to shared_ptr*/

    static Message makeDummyMessage(TypeID id = 0) {
        return Message(id);
    }

    char *makeSerializedMessage() const {
        char *buf = new char[sizeof(Header) + header.length];
        memcpy(buf, &header, sizeof(Header));
        memcpy(buf + sizeof(Header), payload.get(), header.length);
        return buf;
    }

    size_t getSerializedMessageSize() const {
        return sizeof(Header) + header.length;
    }

    bool operator==(const Message &rhs) const {
        return header.type == rhs.header.type &&
               header.length == rhs.header.length &&
               memcmp(payload.get(), rhs.payload.get(), header.length) == 0;
        /* WARNING: memcmp can cause SEGV */
    }

    bool operator!=(const Message &rhs) const {
        return !(rhs == *this);
    }

    friend std::ostream &operator<<(std::ostream &os, const Message &message) {
        os << "type: " << message.header.type << " length: " << message.header.length << " data: " << message.payload;
        return os;
    }

private:
    Header header;
    std::shared_ptr<char> payload;

    /* header only message */
    Message(TypeID type) : header(type, 0), payload(nullptr) {}
};


#endif //EVENT_MANAGER_MESSAGE_HPP
