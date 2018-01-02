#include "gtest/gtest.h"
#include "../Task.hpp"

template <typename E, typename T>
void makeEventAndCallHandle(Message *msg, T &task){
    E event = E::makeEvent(*msg);
    task.handleEvent(&event);
};

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
        constexpr static int eventType(){ return 'a'; }

        int val;
    };

    class EventBeta : public Event {
    public:
        EventBeta(int val) : Event('b'), val(val) {}
        static EventBeta makeEvent(const Message &msg){
            int data = *(int *)msg.data;
            return EventBeta(data);
        }
        constexpr static int eventType(){ return 'b'; }

        int val;
    };

    void start() override {}

    void handle(Message *msg) override {
        switch (msg->getType()){
            case (EventAlpha::eventType()):
            {
                makeEventAndCallHandle<EventAlpha>(msg, *this);
                break;
            }
            case (EventBeta::eventType()):
            {
                makeEventAndCallHandle<EventBeta>(msg, *this);
                break;
            }
            default:
                throw std::string("no such event.");
        }

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
