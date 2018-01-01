#include "gtest/gtest.h"
#include "../Task.hpp"



class MultiEventHandlingTask : public Task {
public:
    MultiEventHandlingTask() : alphaDone(false), betaDone(false) {}

    class EventAlpha : public Event {
    public:
        EventAlpha(int val) : Event('a'), val(val) {}
        static EventAlpha makeEvent(const Message &msg){
            int data = *(int *)msg.data;
            return EventAlpha(data);
        }
        int val;
    };

    class EventBeta : public Event {
    public:
        EventBeta(int val) : Event('b'), val(val) {}
        static EventBeta makeEvent(const Message &msg){
            int data = *(int *)msg.data;
            return EventBeta(data);
        }
        int val;
    };

    void start() override {}

    void handle(Message *msg) override {
        if(msg->getType() == 'a'){
            EventAlpha event = EventAlpha::makeEvent(*msg);
            handleEvent(&event);
        }
        else if(msg->getType() == 'b'){
            EventBeta event = EventBeta::makeEvent(*msg);
            handleEvent(&event);
        }
        else
            throw std::string("no such event.");

        if(alphaDone && betaDone)
            quit();
    }

    void handleEvent(EventAlpha *event){
        std::cout<<"EventAlpha: "<<event->val<<"\n";
        alphaDone = true;
    }

    void handleEvent(EventBeta *event){
        std::cout<<"EventBeta: "<<event->val<<"\n";
        betaDone = true;
    }

private:
    bool alphaDone;
    bool betaDone;
};

TEST(TaskAndEvent, testHandle) {
    MultiEventHandlingTask task;

    int alpha = 777;
    Message msgAlpha('a', 4, (char *)&alpha);
    task.handle(&msgAlpha);

    int beta = 333;
    Message msgBeta('b', 4, (char *)&beta);
    task.handle(&msgBeta);

    EXPECT_EQ(true, task.hasQuit());
}
