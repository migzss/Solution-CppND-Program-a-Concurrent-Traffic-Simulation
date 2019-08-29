#include <iostream>
#include <random>
#include "TrafficLight.h"

#include <queue>
#include <future>

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> u_lock(mutex_);
    cond_.wait(u_lock, [this] { return !queue_.empty(); });

    T msg = std::move(queue_.back());
    queue_.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> u_lock(mutex_);

    queue_.push_back(std::move(msg));
    cond_.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    current_phase_ = red;
    msg_queue_ = std::make_shared<MessageQueue<traffic_light_phase>>();
}

void TrafficLight::wait_for_green() const
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        auto curr_phase = msg_queue_->receive();
        if (curr_phase == green)
        {
            return;
        }
    }
}

traffic_light_phase TrafficLight::get_current_phase() const
{
    return current_phase_;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycle_through_phase, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycle_through_phase()
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4, 6);

    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "Traffic Light #" << _id << "::Cycle Through Phases: thread id = " << std::this_thread::get_id() << std::endl;
    lck.unlock();

    int cycle_duration = distr(eng);

    auto last_update = std::chrono::system_clock::now();
    while (true)
    {
        long time_since_last_update = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_update).count();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (time_since_last_update >= cycle_duration)
        {

            if (current_phase_ == red)
            {
                current_phase_ = green;
            }
            else
            {
                current_phase_ = red;
            }

            auto msg = current_phase_;
            auto is_sent = std::async(std::launch::async, &MessageQueue<traffic_light_phase>::send, msg_queue_, std::move(msg));
            is_sent.wait();

            last_update = std::chrono::system_clock::now();
            cycle_duration = distr(eng);
        }
    }
}
