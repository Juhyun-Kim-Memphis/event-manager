#ifndef EVENT_MANAGER_MESSAGE_HPP
#define EVENT_MANAGER_MESSAGE_HPP

#include <ostream>
#include <cstring>

/* Raw Message class.
 * consists of Header(8byte) + Byte Array (
 * */
class Message {
public:
    using TypeID = uint32_t;
    using SizeInBytes = uint32_t;

    struct Header {
        Header() {}
        Header(TypeID type, SizeInBytes length) : type(type), length(length) {}
        TypeID type; /* TODO: enum?*/
        SizeInBytes length;

        void reset() {
            type = 0;
            length = 0;
        }
    };

    virtual ~Message() {}

    /* Message Factory */
    static Message makeMessage(TypeID type, const char *src, SizeInBytes size);
    static Message makeDummyMessage(TypeID id = 0) {
        return Message(id);
    }

    /* Accessor */
    Header *getHeader() { return &header; }
    TypeID getID() const { return header.type; }
    SizeInBytes getPayloadSize() const { return header.length; }
    const char *getPayload() const { return payload; } /*TODO: remove .get() change return type to shared_ptr*/

    /* only used in IO (pipe, socket, file).. */
    char *makeSerializedMessage() const {
        char *buf = new char[sizeof(Header) + header.length];
        memcpy(buf, &header, sizeof(Header));
        memcpy(buf + sizeof(Header), payload, header.length);
        return buf;
    }

    size_t getSerializedMessageSize() const {
        return sizeof(Header) + header.length;
    }

    /* Auxiliary Function */
    bool operator==(const Message &rhs) const {
        return header.type == rhs.header.type &&
               header.length == rhs.header.length &&
               memcmp(payload, rhs.payload, header.length) == 0;
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
    const char *payload;

    Message(TypeID type, const char *src, SizeInBytes size) : header(type, size), payload(src) {}
    /* header only message */
    Message(TypeID type) : header(type, 0), payload(nullptr) {}
};


class RAIIWrapperMessage : public Message {
public:
    /* Given payload(contiguous serialized data) as a form of shared_ptr(heap), make Message  */
    static RAIIWrapperMessage* newMessageByOutsidePayload(TypeID type, const std::shared_ptr<char> &payload, SizeInBytes size) {
        return new RAIIWrapperMessage(type, payload, size);
    }

    /**/
    static RAIIWrapperMessage* newMessageByAllocatingAndCopying(TypeID type, const char *src, SizeInBytes size) {
        char *dst = new char[size];
        memcpy(dst, src, size);
        return new RAIIWrapperMessage(type, dst, size);
    }

private:
    RAIIWrapperMessage(TypeID type, char src[], SizeInBytes length)
            : Message(Message::makeMessage(type, src, length)), payloadRAIIWrapper(src, std::default_delete<char[]>()) {}
    RAIIWrapperMessage(TypeID type, const std::shared_ptr<char> &payload, SizeInBytes length)
            : Message(Message::makeMessage(type, payload.get(), length)), payloadRAIIWrapper(payload) {}

    std::shared_ptr<char> payloadRAIIWrapper;
};

#endif //EVENT_MANAGER_MESSAGE_HPP
