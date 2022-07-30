#include "Fsm.h"

#define SOLAR_MAX_TEMP 190

class AlarmFSM
{
public:
  AlarmFSM();
  void setup();
  void run();
  Fsm *fsm;
  State *state_no_alarm;
  State *state_alarm;
  State *state_silenced;
};

AlarmFSM* getAlarmFsm();
