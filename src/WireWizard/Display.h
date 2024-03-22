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
  * @file Display.h
  * 
  * lvgl interface to the ST7796_DRIVER display
  */

#include <lvgl.h>
#include "EventQueue.h"


class Display
{
private:
  static Display *display;  

private:
  static IRAM_ATTR void displayTask(void *parg);

  static void displayFlush (lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  static bool readTouch    (lv_indev_drv_t *indev, lv_indev_data_t *data);
  static void buttonEvent  (lv_obj_t *obj,    lv_event_t event);
  static void sliderEvent  (lv_obj_t *slider, lv_event_t event);

private:
  bool enabled = true;

  lv_style_t style_btn_enabled;
  lv_style_t style_btn_disabled;

  lv_obj_t *buttonOnOff  = NULL;
  lv_obj_t *sliderSpeed  = NULL;
  lv_obj_t *sliderSteps  = NULL;
  lv_obj_t *buttonBeep   = NULL;
  lv_obj_t *buttonXLeft  = NULL;
  lv_obj_t *buttonXRight = NULL;
  lv_obj_t *buttonYLeft  = NULL;
  lv_obj_t *buttonYRight = NULL;
  lv_obj_t *buttonZLeft  = NULL;
  lv_obj_t *buttonZRight = NULL;

private:
  void      initStyles();
  lv_obj_t *createLabel(int16_t posX, int16_t posY, String text);
  lv_obj_t *createButton (int16_t posX, int16_t posY, int16_t sizeX, int16_t sizeY, String text);
  lv_obj_t *createSlider(int16_t posX, int16_t posY, int16_t sizeX, int16_t sizeY, int min, int max, int standard);

public: 
  Display();

  void begin();
};
