#include <Wire.h>
#include <TimeLib.h>
#include <U8g2lib.h>
#include <DS3232RTC.h>
#include <EEPROM.h>
#include "SensorLamp.h"

#define BUTTON_LEFT  2
#define BUTTON_RIGHT 3

#define BUTTON_NO_PRESS        0
#define BUTTON_LONG_PRESS    800   // ms
#define BUTTON_SHORT_PRESS   120   // ms

#define BUTTON_RESET       10000

U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C Display(U8G2_R0, U8X8_PIN_NONE);

bool doSetup = false;

volatile unsigned int left_pressed  = 0;
volatile unsigned int right_pressed = 0;

void buttonLeftInterrupt()
{
    unsigned long current_pressed = millis();
    static unsigned long last_pressed = current_pressed;

    if (digitalRead(BUTTON_LEFT) == LOW)
    {
        last_pressed = current_pressed;
    }
    else
    {
        if (current_pressed - last_pressed > BUTTON_LONG_PRESS)
        {
            left_pressed = BUTTON_LONG_PRESS;
        }
        else if (current_pressed - last_pressed > BUTTON_SHORT_PRESS)
        {
            left_pressed = BUTTON_SHORT_PRESS;
        }
        else
        {
            left_pressed = BUTTON_NO_PRESS;
        }
    }
}

void buttonRightInterrupt()
{
    unsigned long current_pressed = millis();
    static unsigned long last_pressed = current_pressed;

    if (digitalRead(BUTTON_RIGHT) == LOW)
    {
        last_pressed = current_pressed;
    }
    else
    {
        if (current_pressed - last_pressed > BUTTON_LONG_PRESS)
        {
            right_pressed = BUTTON_LONG_PRESS;
        }
        else if (current_pressed - last_pressed > BUTTON_SHORT_PRESS)
        {
            right_pressed = BUTTON_SHORT_PRESS;
        }
        else
        {
            right_pressed = BUTTON_NO_PRESS;
        }
    }
}

void updateDisplay(const uint32_t &cycleTime)
{
    uint32_t current_cycle = millis();
    static uint32_t last_cycle = current_cycle;

    if (current_cycle - last_cycle >= cycleTime)
    {
        static uint8_t is_next_page = 0;

        // Call first page
        if (is_next_page == 0) {
            Display.firstPage();
            is_next_page = 1;
        }

        // Draw functions
        Display.setDrawColor(1);
        char buffer[6];
        sprintf (buffer, "%02d:%02d", hour(), minute());
        Display.setFont(u8g2_font_fub30_tn);
        Display.drawStr((Display.getWidth() - Display.getStrWidth(buffer)) / 2, 31, buffer);

        // Call next page
        if (Display.nextPage() == 0) {
            is_next_page = 0;     // ensure, that first page is called
            last_cycle = current_cycle;
        }
    }
}

bool setupTime()
{
    TimeElements tm;
    breakTime(now(), tm);

    unsigned long current_pressed = millis();
    static unsigned long last_pressed = current_pressed;

    if (left_pressed == BUTTON_SHORT_PRESS)
    {
        tm.Hour = (tm.Hour < 23) ? (tm.Hour + 1 ) : (tm.Hour - 23);
        setTime(makeTime(tm));
        last_pressed = current_pressed;
        left_pressed = BUTTON_NO_PRESS;
    }

    if (right_pressed == BUTTON_SHORT_PRESS)
    {

        tm.Minute = (tm.Minute <  59) ? (tm.Minute + 1) : (tm.Minute - 59);
        setTime(makeTime(tm));
        last_pressed = current_pressed;
        right_pressed = BUTTON_NO_PRESS;
    }

    if (current_pressed - last_pressed > BUTTON_RESET)
    {
        return false;
    }

    return true;
}

void setup()
{
    Serial.begin(115200);
    Display.begin();
    pinMode(BUTTON_LEFT, INPUT_PULLUP);
    pinMode(BUTTON_RIGHT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_LEFT),  buttonLeftInterrupt,  CHANGE);
    attachInterrupt(digitalPinToInterrupt(BUTTON_RIGHT), buttonRightInterrupt, CHANGE);
}


void loop()
{
    if (left_pressed == BUTTON_LONG_PRESS)
    {
        Serial.println("Left pressed: LONG");
    }
    else if (left_pressed == BUTTON_SHORT_PRESS)
    {
        Serial.println("Left pressed: SHORT");
    }

    if (right_pressed == BUTTON_LONG_PRESS)
    {
        Serial.println("Right pressed: LONG");
    }
    else if (right_pressed == BUTTON_SHORT_PRESS)
    {
        Serial.println("Right pressed: SHORT");
    }

    if (doSetup || (left_pressed == BUTTON_LONG_PRESS && right_pressed == BUTTON_LONG_PRESS))
    {
        doSetup = setupTime();
    }

    updateDisplay(500);

    delay(100);
}