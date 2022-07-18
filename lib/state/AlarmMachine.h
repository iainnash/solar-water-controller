#include <FunctionFSM.h>

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

AlarmFSM* getAlarmFsm();
