#include "gtest/gtest.h"
#include "../Task.hpp"

class EventAlpha : public Event {
public:
    EventAlpha(int val) : val(val) {}

    static constexpr Message::ID getMessageID() { return 777; }

    static EventAlpha makeEvent(const Message &msg){
        int data = *(int *)msg.data;
        return EventAlpha(data);
    }
    static EventAlpha* makeEventNew(const Message &msg){
        int data = *(int *)msg.data;
        return new EventAlpha(data);
    }
    constexpr static int eventType(){ return 'a'; }

    int val;
};

class EventBeta : public Event {
public:
    EventBeta(int val) : val(val) {}

    static constexpr Message::ID getMessageID() { return 333; }

    static EventBeta makeEvent(const Message &msg){
        int data = *(int *)msg.data;
        return EventBeta(data);
    }
    static EventBeta* makeEventNew(const Message &msg){
        int data = *(int *)msg.data;
        return new EventBeta(data);
    }
    constexpr static int eventType(){ return 'b'; }

    int val;
};

class MultiEventHandlingTask : public Task {
public:
    MultiEventHandlingTask() : alphaDone(false), betaDone(false) {
        useDefaultHandler<EventAlpha>(this);
        useDefaultHandler<EventBeta>(this);
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

    void handle(Message *msg) override {
        Task::handle(msg);
        if(alphaDone && betaDone)
            quit();
    }

private:
    bool alphaDone;
    bool betaDone;
};

TEST(Task, testHandle) {
    MultiEventHandlingTask task;

    int alpha = 777432;
    Message msgAlpha(777, 4, (char *)&alpha);
    task.handle(&msgAlpha);

    int beta = 3333245;
    Message msgBeta(333, 4, (char *)&beta);
    task.handle(&msgBeta);

    EXPECT_EQ(true, task.hasQuit());
}