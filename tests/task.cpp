#include "gtest/gtest.h"
#include "../Task.hpp"
#include "../switch.hpp"

template <typename E, typename T>
void makeEventAndCallHandle(const Message *msg, T &task){
    E event = E::makeEvent(*msg);
    task.handleEvent(&event);
};

class MultiEventHandlingTask : public Task {
public:
    MultiEventHandlingTask() : alphaDone(false), betaDone(false) {}

    class EventAlpha : public Event {
    public:
        constexpr static int eventType(){ return 0; }
        EventAlpha(int val) : Event(eventType()), val(val) {}
        static EventAlpha makeEvent(const Message &msg){
            int data = *(int *)msg.data;
            return EventAlpha(data);
        }

        int val;
    };

    class EventBeta : public Event {
    public:
        constexpr static int eventType(){ return 1; }
        EventBeta(int val) : Event(eventType()), val(val) {}
        static EventBeta makeEvent(const Message &msg){
            int data = *(int *)msg.data;
            return EventBeta(data);
        }

        int val;
    };

    void start() override {}

    void handle(Message *msg) override;

    void handleEvent(EventAlpha *event){
        std::cout<<"EventAlpha: "<<event->val<<"~\n";
        alphaDone = true;
    }

    void handleEvent(EventBeta *event){
        std::cout<<"EventBeta: "<<event->val<<"~\n";
        betaDone = true;
    }

private:
    bool alphaDone;
    bool betaDone;
};

// Default option shall be one higher than the last
enum { ADD = 0, SUB = 1, DEFAULT = 2 };

template <int Operation>
struct interpreter {
    template<typename T>
    static inline int run(const Message *msg, T *t);
};

template <> struct interpreter<DEFAULT> {
    static const char *name() { return "Default"; }
    template<typename T>
    static inline int run(const Message *, T *t) {
        throw std::string("no such event.");
    }
};

template <> struct interpreter<MultiEventHandlingTask::EventAlpha::eventType()> {
    static const char *name() { return "Alpha"; }
    template<typename T>
    static inline int run(const Message *msg, T *t) {
        MultiEventHandlingTask::EventAlpha event = MultiEventHandlingTask::EventAlpha::makeEvent(*msg);
        t->handleEvent(&event);
        return 0;
    }
};

template <> struct interpreter<MultiEventHandlingTask::EventBeta::eventType()> {
    static const char *name() { return "Beta"; }
    template<typename T>
    static inline int run(const Message *msg, T *t) {
        MultiEventHandlingTask::EventBeta event = MultiEventHandlingTask::EventBeta::makeEvent(*msg);
        t->handleEvent(&event);
        return 0;
    }
};

template <int Operation>
struct operation_table {
    template<typename T>
    static always_inline int value(const Message *msg, T &t) {
        return interpreter<Operation>::run(msg, t);
    }
};


TEST(TaskAndEvent, testHandle) {
    MultiEventHandlingTask task;

    int alpha = 777;
    Message msgAlpha(MultiEventHandlingTask::EventAlpha::eventType(), 4, (char *)&alpha);
    task.handle(&msgAlpha);

    int beta = 333;
    Message msgBeta(MultiEventHandlingTask::EventBeta::eventType(), 4, (char *)&beta);
    task.handle(&msgBeta);

    EXPECT_EQ(true, task.hasQuit());
}

TEST(TaskAndEvent, testHandleThrowNoSuchEvent) {
    MultiEventHandlingTask task;

    int alpha = 777;
    Message unpromisedMsg(854, 4, (char *)&alpha);
    EXPECT_ANY_THROW(task.handle(&unpromisedMsg));
}

void MultiEventHandlingTask::handle(Message *msg) {
    metalevel::switch_table<0, 1, operation_table>::run<int>(msg->getType(), msg, this);
//        switch (msg->getType()){
//            case (EventAlpha::eventType()):
//            {
//                makeEventAndCallHandle<EventAlpha>(msg, *this);
//                break;
//            }
//            case (EventBeta::eventType()):
//            {
//                makeEventAndCallHandle<EventBeta>(msg, *this);
//                break;
//            }
//            default:
//                throw std::string("no such event.");
//        }

    std::cout<<"alpha: "<<alphaDone<<", "<<"beta: "<<betaDone<<"\n";

    if(alphaDone && betaDone)
        quit();
}
