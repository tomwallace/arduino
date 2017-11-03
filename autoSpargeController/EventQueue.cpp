/*
  EventQueue.cpp - Library for tracking events (non-duplicate) and informing if any events are in the queue.
  Created by Tom Wallace.
*/

#include "Arduino.h"
#include "EventQueue.h"

EventQueue::EventQueue(String queueName)
{
	_queue = "";
    _queueName = queueName;
};

void EventQueue::AddEvent(String event) 
{
    // If event is not already on queue, add it
    if (_queue.indexOf(event) == -1) {
      _queue = _queue + event;
    }
}

void EventQueue::RemoveEvent(String event) {
    _queue.replace(event, "");
}

bool EventQueue::IsPopulated() {
    return (_queue.length() > 0);
}