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
/**
  * @file WireWizard.ino
  * 
  * Main WireWizard file
  */

#include "Config.h"
#define USE_CONFIG_OVERRIDE //!< Switch to use ConfigOverride
#ifdef USE_CONFIG_OVERRIDE
  #include "ConfigOverride.h"
#endif

#include <Arduino.h>
#include <SPI.h>
#include "EventQueue.h"
#include "Sensors.h"
#include "Motors.h"
#include "Controller.h"
#include "Display.h"
#include "WebServer.h"

Motors      motors;
Sensors     sensors;
Controller  controller(motors, sensors);
Display     display;
MyWebServer webServer;


void setup() 
{
  Serial.begin(115200);

  disableCore0WDT();
  disableCore1WDT();

  SPI.begin();

  motors.begin();
  motors.delay(0);
  motors.enable(true);
  motors.enableOnIdle(false);
  sensors.begin();
  controller.begin();
  display.begin();

  // webServer.begin();
}

#define MAX_DELAY 1000
#define MAX_STEPS 10 * 16 * 200

void testCut()
{
  int ticks = millis();

  motors.beep(100);
  motors.delay(1000);

  Serial.println("Fill in");
  ticks = millis();
  motors.stepX(-100000);
  Serial.println("Wait for cutter sensor");
  while (sensors.getY() < 3000) {
    delay(100);
    if (millis() - ticks > 5000) {
      break;
      // return;
    }
  };
  motors.resetSteps();

  Serial.println("Move forward");
  ticks = millis();
  motors.stepX(-100000);
  motors.stepZ(-100000);
  Serial.println("Wait for out sensor");
  while (sensors.getZ() < 3000) {
    delay(100);
    if (millis() - ticks > 5000) {
      break;
      // return;
    }
  };
  motors.resetSteps();

  Serial.println("Move a bit forward");
  ticks = millis();
  motors.stepX(-3000);
  motors.stepZ(-3000);
  while (!motors.isIdle()) {
    delay(100);
    if (millis() - ticks > 5000) {
      break;
      // return;
    }
  }
  motors.resetSteps();

  Serial.println("Cut");
  ticks = millis();
  motors.stepY(4000);
  while (!motors.isIdle()) {
    delay(100);
    if (millis() - ticks > 5000) {
      break;
      // return;
    }
  }
  ticks = millis();
  motors.stepY(-4000);
  while (!motors.isIdle()) {
    delay(100);
    if (millis() - ticks > 5000) {
      break;
      // return;
    }
  }

  Serial.println("Move out");
  ticks = millis();
  motors.stepZ(-200000);
  Serial.println("Wait for out");
  while (sensors.getZ() > 3000) {
    delay(100);
    if (millis() - ticks > 5000) {
      break;
      // return;
    }
  };

  motors.beep(100);
  motors.resetSteps();

  Serial.println("Finish");
}

void loop() 
{
  static int speed = 100;
  static int steps = 16 * 200;

  if (eventQueue.messageWaiting()) {
    Event event;

    if (eventQueue.receive(event)) {
      switch (event.pushButton) {
      case Event::X_LEFT:  motors.stepX(-steps); break;
      case Event::X_RIGHT: motors.stepX( steps); break;
      case Event::Y_LEFT:  motors.stepY(-steps); break;
      case Event::Y_RIGHT: motors.stepY( steps); break;
      case Event::Z_LEFT:  motors.stepZ(-steps); break;
      case Event::Z_RIGHT: motors.stepZ( steps); break;
      case Event::ON:      motors.enable(true);  break;
      case Event::OFF:     motors.enable(false); break;
      case Event::STEPS:  
        steps = event.steps * MAX_STEPS / 100; 
        break;
      case Event::SPEED:  
        motors.delay(MAX_DELAY * (100.0 - event.speed) / 100.0); 
        break;
      case Event::START:  
        controller.move(100);
        controller.cut();
        controller.eject(100);
        break;
      }
    }
  }

  int valueX = sensors.getX();
  int valueY = sensors.getY();
  int valueZ = sensors.getZ();

  display.setValueX("X: " + String(valueX));
  display.setValueY("Y: " + String(valueY));
  display.setValueZ("Z: " + String(valueZ));

  // Serial.println("Lichtschranke (X, Y, Z): " + String(valueX) + ", " + String(valueY) + ", " + String(valueZ));

/*
  if (valueX > 3000) {
    Serial.println("In sensor");
    testCut();
  }
  */

  delay(100);
}
