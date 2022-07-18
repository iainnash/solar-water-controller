#include "HAL.h"

// for unit testing only
void HAL::set_alarm_silence_button(bool state)
{
  alarm_silence_button = state;
}

// for real, sets pin state for alarm readout
void HAL::set_alarm_state(bool state)
{
  alarm = state;
}

// for real,
bool HAL::is_alarm_silence_pushed()
{
  return alarm_silence_button;
}

void HAL::set_heater(bool state)
{
  heater = state;
}
bool HAL::get_heater()
{
  return heater;
}
unsigned long HAL::get_seconds()
{
  return seconds;
}
void HAL::set_seconds(unsigned long newSeconds)
{
  seconds = newSeconds;
}
Temps HAL::get_temps()
{
  return temps;
}

void HAL::set_temps(int tank_temp_f, int solar_temp_f, int water_shower_out_f) {
  temps.tank_temp_f = tank_temp_f;
  temps.solar_temp_f = solar_temp_f;
  temps.water_shower_out_f = water_shower_out_f;
}

unsigned int HAL::get_hour_of_day()
{
  return hour_of_day;
}

void HAL::set_hour_of_day(unsigned int new_hour_of_day)
{
  hour_of_day = new_hour_of_day;
}

bool HAL::get_pump()  {
  return pump;
}

void HAL::set_pump(bool new_pump) {
  pump = new_pump;
}

// singleton getter
HAL hal;
HAL *getHal(void)
{
  return &hal;
}