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
  * @file Motors.cpp
  * 
  * Stepper Interface
  */

#include <SPI.h>
#include <I2S.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Motors.h"

#define ENABLE_BIT  0
#define X_STEP_BIT  1
#define X_DIR_BIT   2
#define Y_STEP_BIT  3
#define Y_DIR_BIT   4
#define Z_STEP_BIT  5
#define Z_DIR_BIT   6
#define BEEP_BIT    7

const int latchPin = GPIO_NUM_17;
const int clockPin = GPIO_NUM_16;
const int dataPin  = GPIO_NUM_21;

Motors::Motors()
{
  reset();
}

void Motors::reset()
{
  enabled  = false;
  currPosX = 0;
  currPosY = 0;
  currPosZ = 0;
  doStepsX = 0;
  doStepsY = 0;
  doStepsZ = 0;
  delayUs  = 0;
  beepMs   = 0;
}

void Motors::serialOut(byte value)
{
  digitalWrite(latchPin, LOW);                  // Schieberegister-Puffer sperren
  shiftOut(dataPin, clockPin, MSBFIRST, value); // Wert über SPI senden
  digitalWrite(latchPin, HIGH);                 // Schieberegister-Puffer freigeben
}

void Motors::setEnableBit(byte &value, short enableBit, bool enable)
{
  if (enable) {
    bitClear(value, enableBit);
  } else {
    bitSet(value, enableBit);
  }
}

void Motors::setBeepBit(byte &value, short beepBit, TickType_t startTickCount, long &beepMs)
{
  uint32_t elapsedTimeMs = (xTaskGetTickCount() - startTickCount) * portTICK_PERIOD_MS;

  if (elapsedTimeMs > beepMs) {
    beepMs = 0;
  }
  if (beepMs > 0) {
    bitSet(value, beepBit);
  } else {
    bitClear(value, beepBit);
  }
}

void Motors::setStepperBit(byte &value, short dirBit, short stepBit, long &doSteps, long &pos)
{
  bitClear(value, stepBit);
  if (doSteps != 0) {
    bitSet(value, stepBit);
    if (doSteps > 0) {
      bitSet(value, dirBit);
      doSteps--;
      pos++;
    } else {
      bitClear(value, dirBit);
      doSteps++;
      pos--;
    }
  }
}

void Motors::motorTask(void *parg) 
{
  Motors *motors = (Motors *) parg;

  if (motors) {
    TickType_t startTickCount = xTaskGetTickCount();

    motors->serialOut(0b00000001);
    while (true) {
      if ((!motors->enabled && motors->isIdleBeep()) || (motors->isIdleX() && motors->isIdleY() &&motors->isIdleZ())) {
        motors->serialOut(0b00000001);
        vTaskDelay(pdMS_TO_TICKS(1));
        startTickCount = xTaskGetTickCount();
      } else {
        byte value = 0b00000000;

        motors->setEnableBit(value, ENABLE_BIT, motors->enabled);
        if (motors->enabled) {
          motors->setStepperBit(value, X_DIR_BIT, X_STEP_BIT, motors->doStepsX, motors->currPosX);
          motors->setStepperBit(value, Y_DIR_BIT, Y_STEP_BIT, motors->doStepsY, motors->currPosY);
          motors->setStepperBit(value, Z_DIR_BIT, Z_STEP_BIT, motors->doStepsZ, motors->currPosZ);
        }
        motors->setBeepBit(value, BEEP_BIT, startTickCount, motors->beepMs);

        motors->serialOut(value);
        ets_delay_us(10);

        bitClear(value, X_STEP_BIT);
        bitClear(value, Y_STEP_BIT);
        bitClear(value, Z_STEP_BIT);

        motors->serialOut(value);
        ets_delay_us(100);

        if (motors->delayUs > 0) {
          ets_delay_us(motors->delayUs);
        }
      }
    }
  }
}

void Motors::init()
{
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  xTaskCreatePinnedToCore(motorTask, "motorTask", 10000, this, 1, NULL, 0);
}
