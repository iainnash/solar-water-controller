#include "Fsm.h"

#pragma once

class HeaterFSM
{
public:
  HeaterFSM();
  void setup();
  void run();
  Fsm *fsm;
  State *state_idle;
  State *state_sensing;
  State *state_running;
  State *state_sanitizing;
  unsigned long last_sanitize_seconds = 0;

  void state_idle_run();
  void state_sensing_run();
  void state_sanitize_run();
  void state_running_run();
private:
  int sense_count = 0;
};

HeaterFSM* getHeaterFSM();
