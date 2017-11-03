/*
  EventQueue.h - Library for tracking events (non-duplicate) and informing if any events are in the queue.
  Created by Tom Wallace.
*/
#ifndef EventQueue_h
#define EventQueue_h

#include "Arduino.h"

class EventQueue {
  private: 
	String _queue;
	String _queueName;
  
  public: 
	EventQueue(String queueName);
	void AddEvent(String event);
	void RemoveEvent(String event);
	bool IsPopulated();
};

#endif