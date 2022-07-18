#include "HAL.h"
#include "AlarmMachine.h"
#include "HeaterMachine.h"
#include "hardware_constants.h"

void setup()
{
  getAlarmFsm()->setup();
  getHeaterFSM()->setup();

  pinMode(ALARM_PIN, OUTPUT);
  pinMode(ALARM_SILENCE_BUTTON_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
}

unsigned long last_millis;

void update_temperature_readings() {
  // TODO(iain): Read real temperature here
  getHal()->set_temps(100, 100, 100);
}

void loop()
{
  // every 30 seconds
  if (millis() - last_millis > MAIN_HEATER_LOOP_SECONDS * 1000)
  {
    update_temperature_readings();
    // 1 update HAL with machine state
    getHal()->set_heater(digitalRead(HEATER_PIN));
    getHeaterFSM()->run();
    // 2 flush HAL with new state

    digitalWrite(HEATER_PIN, getHal()->get_heater());
    digitalWrite(PUMP_PIN, getHal()->get_pump());

    last_millis = millis();
  }

  if (millis() - last_millis > ALARM_LOOP_SECONDS * 1000)
  {
    getHal()->set_alarm_silence_button(digitalRead(ALARM_SILENCE_BUTTON_PIN));
    getHal()->set_alarm_state(digitalRead(ALARM_PIN));
    getAlarmFsm()->run();
    digitalWrite(ALARM_PIN, getHal()->get_alarm_state());
  }

  // update display
}