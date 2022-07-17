#include "StateMachine.h"
#include <stdio.h>

void HAL::set_alarm_silence_button(bool state)
{
  alarm_silence_button = state;
}

void HAL::set_alarm_state(bool state)
{
  alarm = state;
}
bool HAL::is_alarm_silence_pushed()
{
  return alarm_silence_button;
}

void HAL::set_alarm_condition(bool state) {
  alarm_condition = state;
}

enum Trigger
{
  ALARM_SILENCE,
  ALARM_CONDITION,
  ALARM_RESOLVED
};

HAL hal;

// Alarm
void no_alarm_on_enter()
{
  hal.set_alarm_state(false);
}

void alarm_on_enter()
{
  hal.set_alarm_state(true);
}

void alarm_silenced()
{
  hal.set_alarm_state(false);
}

AlarmFSM::AlarmFSM()
{
  setup();
}

void AlarmFSM::setup()
{
  state_no_alarm = new FunctionState("no alarm", &no_alarm_on_enter, nullptr, nullptr);
  state_alarm = new FunctionState("alarm", &alarm_on_enter, nullptr, nullptr);
  state_silenced = new FunctionState("silenced", &alarm_silenced, nullptr, nullptr);
  fsm = new FunctionFsm(state_no_alarm);
  fsm->add_transition(state_no_alarm, state_alarm, ALARM_CONDITION, nullptr);
  fsm->add_transition(state_alarm, state_silenced, ALARM_SILENCE, nullptr);
  fsm->add_transition(state_silenced, state_no_alarm, ALARM_RESOLVED, nullptr);
  fsm->add_transition(state_alarm, state_no_alarm, ALARM_RESOLVED, nullptr);
}

void AlarmFSM::run()
{
  if (hal.alarm_condition)
  {
    fsm->trigger(ALARM_CONDITION);
  }
  else
  {
    fsm->trigger(ALARM_RESOLVED);
  }
  if (hal.alarm_silence_button == true)
  {
    fsm->trigger(ALARM_SILENCE);
  }
  fsm->run_machine();
}

AlarmFSM alarm_fsm;

HAL* getHal()
{
  return &hal;
}

AlarmFSM* getAlarmFsm()
{
  return &alarm_fsm;
}
