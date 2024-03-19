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
  * @file Motors.h
  * 
  * Stepper Interface
  */

class Motors
{
private:
  bool enabled;
  long currPosX;
  long currPosY;
  long currPosZ;
  long doStepsX;
  long doStepsY;
  long doStepsZ;
  long delayUs;
  long beepMs;

private:
  static void motorTask(void *parg);

private:
  void serialOut     (byte value);
  void setEnableBit  (byte &value, short enableBit, bool enable);
  void setBeepBit    (byte &value, short beepBit, TickType_t startTickCount, long &beepMs);
  void setStepperBit (byte &value, short dirBit, short stepBit, long &doSteps, long &pos);

public:
  Motors();

  void init();

  void reset();

  bool isIdleX()        { return doStepsX == 0; }
  bool isIdleY()        { return doStepsY == 0; }
  bool isIdleZ()        { return doStepsZ == 0; }
  bool isIdleBeep()     { return beepMs   == 0; }

  bool isEnabled()      { return enabled;       }
  void enable(bool en)  { enabled = en;         }

  long getPosX()        { return currPosX;      }
  long getPosY()        { return currPosY;      }
  long getPosZ()        { return currPosZ;      }
  void stepX(int steps) { if (enabled) doStepsX += steps; }
  void stepY(int steps) { if (enabled) doStepsY += steps; }
  void stepZ(int steps) { if (enabled) doStepsZ += steps; }

  void delay(int us)    { delayUs = us;         }

  void beep(int ms)     { beepMs = ms;          }
};
