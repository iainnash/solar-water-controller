#include <FunctionFSM.h>

class HAL
{
public:
  bool alarm_silence_button;
  bool alarm;
  void set_alarm_silence_button(bool state);
  void set_alarm_condition(bool state);
  void set_alarm_state(bool state);
  bool is_alarm_silence_pushed();
  bool alarm_condition;
};

class AlarmFSM
{
public:
  AlarmFSM();
  void setup();
  void run();
  FunctionFsm *fsm;
  FunctionState *state_no_alarm;
  FunctionState *state_alarm;
  FunctionState *state_silenced;
};

HAL* getHal();
AlarmFSM* getAlarmFsm();
