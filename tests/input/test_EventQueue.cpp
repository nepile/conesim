#include <gtest/gtest.h>
#include <memory>
#include <limits>
#include "input/EventQueue.hpp"
#include "input/ExternalEvent.hpp"

using namespace input;


class DummyQueueEvent : public ExternalEvent {
public:
    explicit DummyQueueEvent(double time) : ExternalEvent(time) {}
    void processEvent() override {}
};

class MockEventQueue : public EventQueue {
private:
    bool isEmpty;

public:
    explicit MockEventQueue(bool isEmpty) : isEmpty(isEmpty) {}

    std::unique_ptr<ExternalEvent> nextEvent() override {
        if (isEmpty) {
            return std::make_unique<DummyQueueEvent>(std::numeric_limits<double>::max());
        }
        return std::make_unique<DummyQueueEvent>(10.0);
    }

    double nextEventsTime() const override {
        if (isEmpty) {
            return std::numeric_limits<double>::max();
        }
        return 10.0;
    }
};

class EventQueueTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
};

TEST_F(EventQueueTest, EmptyQueueReturnsMaxTime) {
    MockEventQueue emptyQueue(true);
    EXPECT_DOUBLE_EQ(emptyQueue.nextEventsTime(), std::numeric_limits<double>::max());
}

TEST_F(EventQueueTest, EmptyQueueNextEventHasMaxTime) {
    MockEventQueue emptyQueue(true);
    auto event = emptyQueue.nextEvent();
    
    ASSERT_NE(event, nullptr);
    EXPECT_DOUBLE_EQ(event->getTime(), std::numeric_limits<double>::max());
}

TEST_F(EventQueueTest, PopulatedQueueReturnsCorrectTime) {
    MockEventQueue populatedQueue(false);
    EXPECT_DOUBLE_EQ(populatedQueue.nextEventsTime(), 10.0);
}