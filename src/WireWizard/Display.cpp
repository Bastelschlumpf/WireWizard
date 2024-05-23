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
  * @file Display.cpp
  * 
  * Implementation of the display class
  */

#include <SPI.h>
#include <TFT_eSPI.h>
#include "Display.h"

#define DISP_TASK_PRO   2
#define DISP_TASK_CORE  1
#define DISP_TASK_STACK 4096 * 2

TaskHandle_t lv_disp_tcb = NULL;

#define LCD_EN		  GPIO_NUM_5     
#define LCD_BLK_ON  digitalWrite(LCD_EN, LOW)
#define LCD_BLK_OFF digitalWrite(LCD_EN, HIGH)

TFT_eSPI tft = TFT_eSPI(TFT_HEIGHT, TFT_WIDTH); 

static lv_disp_buf_t disp_buf;
static lv_color_t    color_buf[TFT_WIDTH * 10];

Display *Display::display = NULL;

Display::Display()
{
  display = this;

  valueX  = "x";
  valueY  = "y";
  valueZ  = "z";
}

void Display::displayFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

bool Display::readTouch(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
  static uint16_t last_x = 0;
  static uint16_t last_y = 0;

  uint16_t touchX=0, touchY=0;

  boolean touched = tft.getTouch(&touchX, &touchY);

  if (touched != false) {
    last_x        = touchX;
    last_y        = touchY;
    data->point.x = last_x;
    data->point.y = last_y;
    data->state   = LV_INDEV_STATE_PR;
  } else {
    data->point.x = last_x;
    data->point.y = last_y;
    data->state   = LV_INDEV_STATE_REL;
  }
  return false;
}

void Display::buttonEvent(lv_obj_t *button, lv_event_t event) 
{
  if (event == LV_EVENT_RELEASED) {
    Event event;

         if (button == Display::display->buttonXLeft)  event.pushButton = Event::X_LEFT;
    else if (button == Display::display->buttonXRight) event.pushButton = Event::X_RIGHT;
    else if (button == Display::display->buttonYLeft)  event.pushButton = Event::Y_LEFT;
    else if (button == Display::display->buttonYRight) event.pushButton = Event::Y_RIGHT;
    else if (button == Display::display->buttonZLeft)  event.pushButton = Event::Z_LEFT;
    else if (button == Display::display->buttonZRight) event.pushButton = Event::Z_RIGHT;
    else if (button == Display::display->buttonStart)  event.pushButton = Event::START;
    else if (button == Display::display->buttonOnOff) {
      lv_obj_t *label = lv_obj_get_child(button, NULL);

      Display::display->enabled = !Display::display->enabled;

      if (Display::display->enabled) {
        event.pushButton = Event::ON;
        if (label) lv_label_set_text(label, "On");
      } else {
        event.pushButton = Event::OFF;
        if (label) lv_label_set_text(label, "Off");
      }
    }   
    eventQueue.send(event);
  }
}

void Display::sliderEvent(lv_obj_t *slider, lv_event_t event)
{
  if(event == LV_EVENT_VALUE_CHANGED) {
    Event event;

    uint32_t value = lv_slider_get_value(slider);

    if (slider == Display::display->sliderSpeed) {
      event.speed      = value;
      event.pushButton = Event::SPEED;
    } else if (slider == Display::display->sliderSteps) {
      event.steps      = value;
      event.pushButton = Event::STEPS;
    }

    lv_obj_t *label = lv_obj_get_child(slider, NULL);

    if (label) {
      lv_label_set_text_fmt(label, "%d %%", value);
    }

    eventQueue.send(event);
  }
}

void Display::initStyles()
{
  // Stil für den aktivierten Button
  lv_style_copy(&style_btn_enabled, &lv_style_plain);
  style_btn_enabled.body.main_color   = LV_COLOR_GREEN;    // Hintergrundfarbe
  style_btn_enabled.body.grad_color   = LV_COLOR_GREEN;    // Hintergrundfarbe (Gradient)
  style_btn_enabled.body.border.color = LV_COLOR_WHITE;    // Randfarbe
  style_btn_enabled.text.color        = LV_COLOR_WHITE;    // Textfarbe

  // Stil für den deaktivierten Button
  lv_style_copy(&style_btn_disabled, &lv_style_plain);
  style_btn_disabled.body.main_color   = LV_COLOR_GRAY;    // Hintergrundfarbe
  style_btn_disabled.body.grad_color   = LV_COLOR_GRAY;    // Hintergrundfarbe (Gradient)
  style_btn_disabled.body.border.color = LV_COLOR_WHITE;   // Randfarbe
  style_btn_disabled.text.color        = LV_COLOR_WHITE;   // Textfarbe
}

lv_obj_t *Display::createLabel(int16_t posX, int16_t posY, String text)
{
  lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);

  lv_label_set_text(label, text.c_str());
  lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, posX, posY);
  return label;
}

void Display::setLabelText(lv_obj_t *label, String text)
{
  lv_label_set_text(label, text.c_str());
}

lv_obj_t *Display::createButton(int16_t posX, int16_t posY, int16_t sizeX, int16_t sizeY, String text)
{
  lv_obj_t *button = lv_btn_create(lv_scr_act(), NULL);

  lv_obj_set_size        (button, sizeX, sizeY);
  lv_obj_set_pos         (button,  posX,  posY);
  lv_obj_set_event_cb    (button, buttonEvent);
  lv_btn_set_style       (button, LV_BTN_STATE_REL, &lv_style_btn_rel);
  lv_btn_set_style       (button, LV_BTN_STATE_PR,  &lv_style_btn_pr);

  lv_obj_t *label = lv_label_create(button, NULL);

  lv_label_set_long_mode (label, LV_LABEL_LONG_EXPAND);
  lv_label_set_recolor   (label, true);
  lv_label_set_text      (label, text.c_str());
  lv_label_set_align     (label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align           (label, NULL, LV_ALIGN_CENTER, 0, 0);

  return button;
}

lv_obj_t *Display::createSlider(int16_t posX, int16_t posY, int16_t sizeX, int16_t sizeY, int min, int max, int standard)
{
  lv_obj_t *slider = lv_slider_create(lv_scr_act(), NULL);

  lv_obj_set_size(slider, sizeX, sizeY);
  lv_obj_align(slider, NULL, LV_ALIGN_IN_TOP_LEFT, posX, posY); // Zentrieren Sie den Slider auf dem Bildschirm

  lv_slider_set_range(slider, min, max);
  lv_slider_set_value(slider, standard, LV_ANIM_OFF); // Setzen Sie den Standardwert auf 50
  lv_obj_set_event_cb(slider, sliderEvent);

  lv_obj_t *label     = lv_label_create(slider, NULL);
  String    labelText = String(standard) + "%";

  lv_label_set_text(label, labelText.c_str());
  lv_obj_set_auto_realign(label, true);
  lv_obj_align(label, slider, LV_ALIGN_CENTER, 0, 0);

  return slider;
}

IRAM_ATTR void Display::displayTask(void *parg) 
{
  Display *display = (Display *) parg;

  if (display) {
    display->initStyles();

    display->createLabel( 60, 30, "In");
    display->createLabel(140, 30, "Cut");
    display->createLabel(230, 30, "Out");

    display->buttonXLeft  = display->createButton(  30,  60, 70, 50, "Left");
    display->buttonXRight = display->createButton(  30, 120, 70, 50, "Right");
    display->buttonYLeft  = display->createButton( 120,  60, 70, 50, "Left");
    display->buttonYRight = display->createButton( 120, 120, 70, 50, "Right");
    display->buttonZLeft  = display->createButton( 210,  60, 70, 50, "Left");
    display->buttonZRight = display->createButton( 210, 120, 70, 50, "Right");

    display->createLabel(320, 30, "Enabled");
    display->buttonOnOff = display->createButton( 315,  60, 70, 50, "On");

    display->createLabel (40, 200, "Steps");
    display->sliderSteps = display->createSlider(120, 190, 180, 40, 1, 100, 20);

    display->createLabel (40, 250, "Speed");
    display->sliderSpeed = display->createSlider(120, 240, 180, 40, 0, 100, 20);

    display->buttonStart = display->createButton( 320, 210, 70, 50, "Start");

    lv_obj_t *labelX = display->createLabel(320, 120, display->valueX);
    lv_obj_t *labelY = display->createLabel(320, 140, display->valueY);
    lv_obj_t *labelZ = display->createLabel(320, 160, display->valueZ);

    while (1) {
      display->setLabelText(labelX, display->valueX);
      display->setLabelText(labelY, display->valueY);
      display->setLabelText(labelZ, display->valueZ);

      lv_task_handler();
      vTaskDelay(5);
    }
  }
}

void Display::begin()
{
  pinMode(LCD_EN, OUTPUT);
    
  LCD_BLK_ON;

  tft.begin();
  tft.initDMA();
  tft.setRotation(1);
  delay(100);

  tft.fillScreen(TFT_WHITE);

  lv_init(); 
  lv_disp_buf_init(&disp_buf, color_buf, NULL, TFT_WIDTH * 10);

  // Initialize the display
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = TFT_WIDTH;
  disp_drv.ver_res  = TFT_HEIGHT;
  disp_drv.flush_cb = displayFlush;
  disp_drv.buffer   = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize the input device driver
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type    = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = readTouch;
  lv_indev_drv_register(&indev_drv);

  // Starting display task
  xTaskCreatePinnedToCore(displayTask, "displayTask", DISP_TASK_STACK, this, DISP_TASK_PRO, &lv_disp_tcb, DISP_TASK_CORE);
}
