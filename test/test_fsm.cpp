#include <unity.h>
#include <StateMachine.h>
#include <assert.h>

void setUp(void)
{
    // set stuff up here
    getAlarmFsm()->setup();
    getAlarmFsm()->fsm->run_machine();
}

void tearDown(void)
{
    // clean stuff up here
}

void test_alarm_setup(void)
{
    TEST_ASSERT_MESSAGE(getHal()->alarm == false, "precondition check");
    TEST_ASSERT_MESSAGE(getHal()->alarm_silence_button == false, "precondition check");
    TEST_ASSERT_MESSAGE(getAlarmFsm()->fsm->is_in_state(*getAlarmFsm()->state_no_alarm), "alarm state no alarm");
    getHal()->set_alarm_condition(true);
    getAlarmFsm()->run();
    TEST_ASSERT_MESSAGE(getAlarmFsm()->fsm->is_in_state(*getAlarmFsm()->state_alarm), "state is not alarm");
    getHal()->set_alarm_silence_button(true);
    getAlarmFsm()->run();
    TEST_ASSERT_MESSAGE(getAlarmFsm()->fsm->is_in_state(*getAlarmFsm()->state_silenced), "fsm state is not silenced");
    getHal()->set_alarm_silence_button(false);
    getHal()->set_alarm_condition(false);
    getAlarmFsm()->run();
    TEST_ASSERT_MESSAGE(getAlarmFsm()->fsm->is_in_state(*getAlarmFsm()->state_no_alarm), "fsm state is silenced");
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_alarm_setup);
    UNITY_END();
}
