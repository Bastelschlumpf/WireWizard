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

#include <Arduino.h>
#include "Sensors.h"
#include "Motors.h"

/**
  * @file Controller.h
  * 
  * Implementation of the cutter controller class
  */

#define STEPS_PER_MM ((double) 10000 / 260)


class Controller
{
private:
  Motors  &motors;
  Sensors &sensors;

public:
  Controller(Motors &m, Sensors &s)
    : motors(m)
    , sensors(s)
  {
  }

  void begin();

  void move(double mm);
  void eject(double mm);
  void cut();
};
