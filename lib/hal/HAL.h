#pragma once

struct Temps {
  // Temps(int a, int b, int c);
  float tank_temp_f;
  float solar_temp_f;
  float water_shower_out_f;

  // New sensor
  float tank_in_temp_f;
};

class HAL
{
public:
  HAL();
  void set_alarm_silence_button(bool state);

  void set_alarm_state(bool state);
  bool get_alarm_state();

  bool get_heater();
  void set_heater(bool state);

  bool get_pump();
  void set_pump(bool state);

  // rtc
  unsigned int get_hour_of_day();
  // for testing
  void set_hour_of_day(unsigned int hour);

  bool is_alarm_silence_pushed();
  unsigned long get_seconds();
  void set_seconds(unsigned long seconds);
  Temps get_temps();
  void set_temps(float tank_temp_f, float solar_temp_f, float water_shower_out_f, float tank_in_temp_f);

  // testing only

  // current hour
  unsigned int hour_of_day;
  // for testing, stores these vars
  bool alarm_silence_button;
  // for real
  bool alarm;
  // for testing/real
  bool heater;
  bool pump;
  // for testing
  unsigned long seconds;
  // for testing
  Temps temps;
};

HAL *getHal(void);