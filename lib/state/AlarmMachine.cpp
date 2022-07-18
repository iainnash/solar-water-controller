#include "AlarmMachine.h"
#include "HAL.h"
#include "constants.h"

enum Trigger
{
  ALARM_SILENCE,
  ALARM_CONDITION,
  ALARM_RESOLVED
};

// Alarm
void no_alarm_on_enter()
{
  getHal()->set_alarm_state(false);
}

void alarm_on_enter()
{
  getHal()->set_alarm_state(true);
}

void alarm_silenced()
{
  getHal()->set_alarm_state(false);
}

AlarmFSM::AlarmFSM()
{
  setup();
}

void AlarmFSM::setup()
{
  state_no_alarm = new State("no alarm", &no_alarm_on_enter, nullptr, nullptr);
  state_alarm = new State("alarm", &alarm_on_enter, nullptr, nullptr);
  state_silenced = new State("silenced", &alarm_silenced, nullptr, nullptr);
  fsm = new Fsm(state_no_alarm);
  fsm->add_transition(state_no_alarm, state_alarm, ALARM_CONDITION, nullptr);
  fsm->add_transition(state_alarm, state_silenced, ALARM_SILENCE, nullptr);
  fsm->add_transition(state_silenced, state_no_alarm, ALARM_RESOLVED, nullptr);
  fsm->add_transition(state_alarm, state_no_alarm, ALARM_RESOLVED, nullptr);
}

void AlarmFSM::run()
{
  if (getHal()->get_temps().solar_temp_f > SOLAR_MAX_TEMP)
  {
    fsm->trigger(ALARM_CONDITION);
  }
  else
  {
    fsm->trigger(ALARM_RESOLVED);
  }

  if (getHal()->alarm_silence_button == true)
  {
    fsm->trigger(ALARM_SILENCE);
  }
  fsm->run_machine();
}

AlarmFSM alarm_fsm;
AlarmFSM *getAlarmFsm()
{
  return &alarm_fsm;
}
