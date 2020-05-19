#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  std::unique_lock<std::mutex> uLock(_mtx);
  _condition.wait(uLock, [this] { return !_queue.empty(); });

  T msg = std::move(_queue.front());
  _queue.pop_front();
  return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  std::lock_guard<std::mutex> lck(_mtx);
  _queue.push_back(std::move(msg));
  _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  while(true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1)); // slow down the endless loop a bit as in the exercises
    if (_msg_queue.receive() == TrafficLightPhase::green) return;
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    std::random_device device;
    std::mt19937_64 generator(device()); // PRNG as in https://diego.assencio.com/?index=6890b8c50169ef45b74db135063c227c
  
    std::uniform_int_distribution<int> distribution(4000,6000); 
    int duration = distribution(generator);  
    
    auto timestamp = std::chrono::system_clock::now(); // timestamp on enter
    
    while(true)
  {
   std::this_thread::sleep_for(std::chrono::milliseconds(1)); // slow down the endless loop a bit as in the exercises
      
   auto last_timestamp = std::chrono::system_clock::now(); // timestamp on enter  
   auto msecs_from_last_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>( last_timestamp - timestamp ).count(); // based on stackoverflow answers https://stackoverflow.com/a/6012671
   
   if (duration < msecs_from_last_timestamp) 
   {
    _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red; // toggle the current phase
    
    _msg_queue.send(std::move(_currentPhase));

    duration = distribution(generator);
    timestamp = std::chrono::system_clock::now();
   }  
  }
}
