#ifndef EVENT_MANAGER_MESSAGE_HPP
#define EVENT_MANAGER_MESSAGE_HPP

#include <ostream>
#include <cstring>

/* can be event or task */
class Message {
public:
    using ID = uint32_t;

    struct Header {
        Header() {}
        Header(uint32_t type, uint32_t length) : type(type), length(length) {}
        uint32_t type; /* TODO: enum?*/
        uint32_t length;
    };

    Message(uint32_t type, uint32_t length, const char *src) : header(type, length), data(new char[length]) {
        memcpy(data, src, length); /* TODO: too many copy operation of raw data. reduce them.*/
    }

    ~Message() {
        delete[] data;
    }

    int getType(){ return header.type; }

    char *makeSerializedMessage() const {
        char *buf = new char[sizeof(Header) + header.length];
        memcpy(buf, &header, sizeof(Header));
        memcpy(buf + sizeof(Header), data, header.length);
        return buf;
    }
    size_t getSerializedMessageSize() const {
        return sizeof(Header) + header.length;
    }

    bool operator==(const Message &rhs) const {
        return header.type == rhs.header.type &&
               header.length == rhs.header.length &&
               memcmp(data, rhs.data, header.length) == 0;
        /* WARNING: memcmp can cause SEGV */
    }

    bool operator!=(const Message &rhs) const {
        return !(rhs == *this);
    }

    friend std::ostream &operator<<(std::ostream &os, const Message &message) {
        os << "type: " << message.header.type << " length: " << message.header.length << " data: " << message.data;
        return os;
    }

    static Message makeDummyMessage(ID id = 0) {
        return Message(id);
    }

public:
    Header header;
    char *data; /* TODO: "onwership?", "dtor delete?", "unique_ptr?" */

    /* header only message */
    Message(uint32_t type) : header(type, 0), data(nullptr) {}
};


#endif //EVENT_MANAGER_MESSAGE_HPP
