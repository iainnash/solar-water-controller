#include "HeaterMachine.h"
#include "HAL.h"
#include "constants.h"

enum HeaterEvents
{
  START_SENSE,
  SENSE_FINISHED,
  SANITIZE_SKIPPED,
  STEAM_HOTTER_TANK,
  START_SANITIZE,
  DAY_TO_NIGHT_TRANSITION,
  SANITIZE_FINISHED,
  DAY_TIME_STARTED_RUN
};

HeaterFSM::HeaterFSM()
{
  setup();
}

bool is_day()
{
  unsigned int hour_of_day = getHal()->get_hour_of_day();
  return hour_of_day >= HOUR_DAY_START && hour_of_day < HOUR_DAY_END;
}

void HeaterFSM::state_sensing_run()
{
  // 1. turn on pump
  getHal()->set_pump(true);
  if (sense_count < 2)
  {
    sense_count += 1;
  }
  else
  {
    sense_count = 0;
    fsm->trigger(SENSE_FINISHED);
  }
}

void HeaterFSM::state_sanitize_run()
{
  // if less than 20 hours since last sanitize don't run cycle
  if (getHal()->get_seconds() - last_sanitize_seconds < (21L * 60L * 60L))
  {
    fsm->trigger(SANITIZE_SKIPPED);
    return;
  }
  if (getHal()->get_temps().tank_temp_f < STERI_CYCLE_END_TEMP)
  {
    getHal()->set_heater(true);
  }
  else
  {
    last_sanitize_seconds = getHal()->get_seconds();
    fsm->trigger(SANITIZE_FINISHED);
  }
}

void HeaterFSM::state_idle_run()
{
  // idle state only exists during the night
  // running state exists during the day
  if (is_day())
  {
    // go to day
    fsm->trigger(DAY_TIME_STARTED_RUN);
  }
  else
  {
    if (getHal()->get_heater())
    {
      // heater on
      if (getHal()->get_temps().tank_temp_f > NIGHT_HEATER_OFF_THRESHOLD_DEG_F)
      {
        // turn off heater
        getHal()->set_heater(false);
      }
    }
    else
    {
      // heater off and it should be on
      if (getHal()->get_temps().tank_temp_f < NIGHT_HEATER_ON_THRESHOLD_DEG_F)
      {
        getHal()->set_heater(true);
      }
    }
  }
}

void HeaterFSM::state_running_run()
{
  getHal()->set_heater(false);
  if (is_day())
  {
    Temps temps = getHal()->get_temps();
    if (temps.solar_temp_f > temps.tank_temp_f)
    {
      fsm->trigger(STEAM_HOTTER_TANK);
    }
    else
    {
      // if we don't have a situation where the steam is less than the tank
      // (turns off pump)
      if (getHal()->get_pump())
      {
        getHal()->set_pump(false);
      }
    }
  }
  else
  {
    // switching from day to night -> check sanitize if needed otherwise go to night
    fsm->trigger(DAY_TO_NIGHT_TRANSITION);
  }
}
// TODO(iain): set state pump running and another without the pump running

void state_idle_run_entrypoint() {
  getHeaterFSM()->state_idle_run();
}
void state_sensing_run_entrypoint() {
  getHeaterFSM()->state_sensing_run();
}
void state_running_run_entrypoint() {
  getHeaterFSM()->state_running_run();
}
void state_sanitize_run_entrypoint() {
  getHeaterFSM()->state_sanitize_run();
}

void HeaterFSM::setup()
{
  state_idle = new State(
      "night-n", nullptr, &state_idle_run_entrypoint,
      nullptr);
  state_sensing = new State(
      "sensing", nullptr, &state_sensing_run_entrypoint,
      nullptr);
  state_running = new State(
      "day-n", nullptr, &state_running_run_entrypoint,
      nullptr);
  state_sanitizing = new State(
      "sanitizing", nullptr, &state_sanitize_run_entrypoint,
      nullptr);
  // start in night mode
  fsm = new Fsm(state_idle);
  fsm->add_transition(state_idle, state_running, DAY_TIME_STARTED_RUN, nullptr);
  fsm->add_transition(state_idle, state_sensing, START_SENSE, nullptr);
  fsm->add_transition(state_idle, state_sanitizing, START_SANITIZE, nullptr);
  fsm->add_transition(state_running, state_sanitizing, START_SANITIZE, nullptr);
  fsm->add_transition(state_sanitizing, state_idle, SANITIZE_FINISHED, nullptr);
  fsm->add_transition(state_sensing, state_running, SENSE_FINISHED, nullptr);
  fsm->add_transition(state_running, state_sanitizing, DAY_TO_NIGHT_TRANSITION, nullptr);
  fsm->add_transition(state_running, state_sensing, STEAM_HOTTER_TANK, nullptr);
  fsm->add_transition(state_sanitizing, state_idle, SANITIZE_SKIPPED, nullptr);
}
void HeaterFSM::run()
{
  fsm->run_machine();
  if (getHal()->get_temps().tank_temp_f >= STERI_CYCLE_END_TEMP) {
    // Update last sanitize seconds
    last_sanitize_seconds = getHal()->get_seconds();
  }
}

HeaterFSM heaterFSM;
HeaterFSM *getHeaterFSM()
{
  return &heaterFSM;
}
