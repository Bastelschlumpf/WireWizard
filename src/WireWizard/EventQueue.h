/*
   Copyright (C) 2024 SFini

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

/**
  * @file EventQueue.h
  * 
  * Multithreading Event Queue
  */
  
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

class Event 
{
public:
  enum PushButton { 
    X_LEFT, X_RIGHT, Y_LEFT, Y_RIGHT, Z_LEFT, Z_RIGHT, 
    ON, OFF, STEPS, SPEED, START
  };

public:
  PushButton pushButton;
  int        speed;
  int        steps;

public:
  Event() 
    : speed(20)
    , steps(20)
  {
  }
};

class EventQueue
{
protected:
  QueueHandle_t eventQueue;

public: 
  EventQueue()
  {
    eventQueue = xQueueCreate(10, sizeof(Event));
  }  
  
  void send(Event &event)
  {
    xQueueSend(eventQueue, &event, portMAX_DELAY);
  }

  bool messageWaiting()
  {
    return uxQueueMessagesWaiting(eventQueue) > 0;
  }

  bool receive(Event &event) 
  {
    return pdPASS == xQueueReceive(eventQueue, &event, 0);
  }
};

extern EventQueue eventQueue;
