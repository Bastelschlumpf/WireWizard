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
  * @file Controller.cpp
  * 
  * Implementation of cutter controller class
  */

#include "Controller.h"  


void Controller::begin()
{
}

void Controller::move(double mm)
{
  motors.beep(100);
  
  motors.stepX(-mm * STEPS_PER_MM);
  motors.stepZ(-mm * STEPS_PER_MM);

  while (!motors.isIdle()) {
    delay(10);
  }
  motors.beep(100);
}

void Controller::eject(double mm)
{
  motors.beep(100);
  
  motors.stepZ(-mm * STEPS_PER_MM);

  while (!motors.isIdle()) {
    delay(10);
  }
  motors.beep(100);
}

void Controller::cut()
{
  motors.beep(100);

  motors.stepY(4000);
  while (!motors.isIdle()) {
    delay(10);
  }
  motors.stepY(-4000);
  while (!motors.isIdle()) {
    delay(10);
  }
  motors.beep(100);
}

