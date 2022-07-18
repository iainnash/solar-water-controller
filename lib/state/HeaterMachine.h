#include <FunctionFSM.h>

#pragma once

class HeaterFSM
{
public:
  HeaterFSM();
  void setup();
  void run();
  FunctionFsm *fsm;
  FunctionState *state_idle;
  FunctionState *state_sensing;
  FunctionState *state_running;
  FunctionState *state_sanitizing;
  unsigned long last_sanitize_seconds = 0;

private:
  int sense_count = 0;
  void state_idle_run();
  void state_sensing_run();
  void state_sanitize_run();
  void state_running_run();
};

HeaterFSM* getHeaterFSM();
