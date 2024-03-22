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

#include <SPI.h>
#include "EventQueue.h"
#include "Motors.h"
#include "Display.h"
#include "WebServer.h"

Motors      motors;
Display     display;
MyWebServer webServer;

void setup() 
{
  Serial.begin(115200);

  disableCore0WDT();
  disableCore1WDT();

  SPI.begin();

  display.begin();

  motors.begin();
  motors.enable(true);
  // motors.beep(100);

  webServer.begin();
}

#define MAX_DELAY 1000
#define MAX_STEPS 10 * 16 * 200

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
      }
    }
  }

  delay(100);
}
