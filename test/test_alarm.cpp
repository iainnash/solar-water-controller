#include <unity.h>
#include <AlarmMachine.h>
#include <HeaterMachine.h>
#include <HAL.h>
#include <assert.h>

void setUp(void)
{
    // set stuff up here
    getAlarmFsm()->setup();
    getAlarmFsm()->run();

    // set stuff up here
    getHeaterFSM()->setup();
    getHeaterFSM()->run();
}

void tearDown(void)
{
    // clean stuff up here
}

void test_night_mode_initial_keeps_temp(void)
{
    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_idle), "default idle state");
    // 8p
    // tank_temp_f, water_shower_out_f, tank_in_temp_f, solar_temp_f
    getHal()->set_temps(50, 50, 50, 50);
    getHal()->set_hour_of_day(20);
    getHeaterFSM()->run();
    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_idle), "still in idle");
    TEST_ASSERT_EQUAL_MESSAGE(getHal()->get_heater(), true, "Heater is not on, should be on");
    // tank_temp_f, water_shower_out_f, tank_in_temp_f, solar_temp_f
    getHal()->set_temps(100, 100, 100, 100);
    getHeaterFSM()->run();
    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_idle), "still in idle");
    TEST_ASSERT_EQUAL_MESSAGE(getHal()->get_heater(), false, "Heater is not off, should be off");
}

void test_day_sense_mode(void)
{
    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_idle), "default idle state");
    // 2p
    // tank_temp_f, water_shower_out_f, tank_in_temp_f, solar_temp_f
    getHal()->set_temps(50, 50, 50, 50);
    getHal()->set_hour_of_day(14);
    getHeaterFSM()->run();
    getHeaterFSM()->run();

    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_running), "not in running state");
    TEST_ASSERT_EQUAL_MESSAGE(getHal()->get_heater(), false, "heater should not be on");
    TEST_ASSERT_EQUAL_MESSAGE(getHal()->get_pump(), false, "pump should not be on");
    // tank_temp_f, water_shower_out_f, tank_in_temp_f, solar_temp_f
    getHal()->set_temps(70, 90, 90, 90);
    getHeaterFSM()->run();
    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_sensing), "still in idle");
    getHeaterFSM()->run();
    TEST_ASSERT_EQUAL_MESSAGE(getHal()->get_heater(), false, "Heater is not off, should be off");
    TEST_ASSERT_EQUAL_MESSAGE(getHal()->get_pump(), true, "Heater is not off, should be off");
    getHeaterFSM()->run();
    // temp remains the same
    TEST_ASSERT_EQUAL_MESSAGE(getHal()->get_pump(), true, "pump is on");
}

void test_day_night_transition_sanitize_run(void)
{
    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_idle), "Default state is not idle");
    
    getHal()->set_temps(50, 50, 50, 50);
    getHal()->set_hour_of_day(8);
    getHeaterFSM()->run();
    TEST_ASSERT_EQUAL_MESSAGE(getHeaterFSM()->last_sanitize_seconds, 0, "Sanitize not 0");
    getHal()->set_seconds(10000);
    getHal()->set_temps(200, 200, 200, 200);
    getHeaterFSM()->run();
    // this will happen in any state
    TEST_ASSERT_EQUAL_MESSAGE(getHeaterFSM()->last_sanitize_seconds, 10000, "Sanitize not 10000");

    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_running), "State is not running");

    // 9p
    getHal()->set_hour_of_day(21);
    getHal()->set_temps(80, 80, 80, 80);
    getHeaterFSM()->run();

    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_sanitizing), "State is not sanitize");

    getHeaterFSM()->run();
    TEST_ASSERT_MESSAGE(getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_idle), "state is not idle");

    // sanitize is recent so should not be updated
    TEST_ASSERT_EQUAL_MESSAGE(getHeaterFSM()->last_sanitize_seconds, 10000, "Sanitize not 10000");
    
}

void test_alarm_setup(void)
{
    TEST_ASSERT_MESSAGE(getHal()->alarm == false, "precondition check");
    TEST_ASSERT_MESSAGE(getHal()->alarm_silence_button == false, "precondition check");
    TEST_ASSERT_MESSAGE(getAlarmFsm()->fsm->is_in_state(getAlarmFsm()->state_no_alarm), "alarm state no alarm");
    // tank_temp_f, water_shower_out_f, tank_in_temp_f, solar_temp_f
    getHal()->set_temps(200, 200, 200, 200);
    getAlarmFsm()->run();
    TEST_ASSERT_MESSAGE(getAlarmFsm()->fsm->is_in_state(getAlarmFsm()->state_alarm), "state is not alarm");
    getHal()->set_alarm_silence_button(true);
    getAlarmFsm()->run();
    TEST_ASSERT_MESSAGE(getAlarmFsm()->fsm->is_in_state(getAlarmFsm()->state_silenced), "fsm state is not silenced");
    getHal()->set_alarm_silence_button(false);
    // tank_temp_f, water_shower_out_f, tank_in_temp_f, solar_temp_f
    getHal()->set_temps(10, 10, 10, 10);
    getAlarmFsm()->run();
    TEST_ASSERT_MESSAGE(getAlarmFsm()->fsm->is_in_state(getAlarmFsm()->state_no_alarm), "fsm state is silenced");
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_alarm_setup);
    RUN_TEST(test_night_mode_initial_keeps_temp);
    RUN_TEST(test_day_night_transition_sanitize_run);
    RUN_TEST(test_day_sense_mode);
    UNITY_END();
}
