#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#pragma once
#include <mutex>
#include <deque>
#include <condition_variable>

#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

enum traffic_light_phase
{
    red,
    green,
};

template <class T>
class MessageQueue
{
public:
    T receive();
    void send(T &&msg);

private:
    std::mutex mutex_;
    std::condition_variable cond_;
    std::deque<T> queue_;
};

class TrafficLight final : public TrafficObject
{
public:
    TrafficLight();
    traffic_light_phase get_current_phase() const;

    void wait_for_green() const;
    void simulate() override;

private:
    void cycle_through_phase();

    std::condition_variable _condition;
    std::mutex _mutex;

    std::shared_ptr<MessageQueue<traffic_light_phase>> msg_queue_;
    traffic_light_phase current_phase_;
};

#endif