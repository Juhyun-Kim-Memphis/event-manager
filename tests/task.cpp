#include <Lock.hpp>
#include "gtest/gtest.h"
#include "../Task.hpp"

class EventAlpha : public Event {
public:
    EventAlpha(int val) : val(val) {}

    static constexpr Message::TypeID getMessageID() { return 777; }

    static EventAlpha* newEvent(const Message &msg){
        int data = *(int *)msg.getPayload();
        return new EventAlpha(data);
    }
    constexpr static int eventType(){ return 'a'; }

    int val;
};

class EventBeta : public Event {
public:
    EventBeta(int val) : val(val) {}

    static constexpr Message::TypeID getMessageID() { return 333; }

    static EventBeta* newEvent(const Message &msg){
        int data = *(int *)msg.getPayload();
        return new EventBeta(data);
    }

    int val;
};

class MultiEventHandlingTask : public Task {
public:
    MultiEventHandlingTask() : alphaDone(false), betaDone(false), lockDone(false) {
        useDefaultHandler<EventAlpha>(this);
        useDefaultHandler<EventBeta>(this);
        useDefaultHandler<LockOwnershipChange>(this);
    }

    void start() override {}

    void handleEvent(EventAlpha *event){
        std::cout<<"EventAlpha: "<<event->val<<".\n";
        alphaDone = true;
    }

    void handleEvent(EventBeta *event){
        std::cout<<"EventBeta: "<<event->val<<".\n";
        betaDone = true;
    }

    void handleEvent(LockOwnershipChange *event){
        std::cout<<"LockOwnershipChange .\n";
        lockDone = true;
    }

    void handle(Message *msg) override {
        Task::handle(msg);
        if(alphaDone && betaDone && lockDone)
            quit();
    }

private:
    bool alphaDone;
    bool betaDone;
    bool lockDone;
};

TEST(Task, testHandle) {
    MultiEventHandlingTask task;

    int alpha = 777432;
    Message msgAlpha = Message::makeMessageByAllocatingAndCopying(777, reinterpret_cast<char *>(&alpha), sizeof(int));
    task.handle(&msgAlpha);

    int beta = 3333245;
    Message msgBeta = Message::makeMessageByAllocatingAndCopying(333, reinterpret_cast<char *>(&beta), sizeof(int));
    task.handle(&msgBeta);

    LockOwnershipChange event;
    Message ownershipChangeMsg = event.makeMessage();
    task.handle(&ownershipChangeMsg);

    EXPECT_EQ(true, task.hasQuit());
}