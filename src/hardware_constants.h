#pragma once

#define ALARM_PIN 6
#define HEATER_PIN 7
#define PUMP_PIN 8
#define ALARM_SILENCE_BUTTON_PIN 11


#define ALARM_LOOP_SECONDS 5 
#define MAIN_HEATER_LOOP_SECONDS 30


// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS_LOCAL 4
#define ONE_WIRE_BUS_REMOTE 5

#define BUTTON_ENTER_PIN 24
#define BUTTON_UP_PIN 25
#define BUTTON_DOWN_PIN 23

#define EEPROM_UPDATE_INTERVAL_SECONDS 60*5 // 5 minutes
#define EEPROM_HEATER_ADDR 2