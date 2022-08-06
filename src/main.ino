#include "HAL.h"
#include "AlarmMachine.h"
#include "HeaterMachine.h"
#include "hardware_constants.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#include <UC1701.h>
#include <MCP7940.h>

#include <Wire.h>
#include <I2C_eeprom.h>

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWireLocal(ONE_WIRE_BUS_LOCAL);
OneWire oneWireRemote(ONE_WIRE_BUS_REMOTE);

#define EEPROM_SIZE 255

I2C_eeprom eeprom50(0x50, EEPROM_SIZE);

// RTC
MCP7940_Class MCP7940;

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensorsLocal(&oneWireLocal);
DallasTemperature sensorsRemote(&oneWireRemote);

// A custom glyph (a smiley)...
static const byte glyph[] = {B00010000, B00110100, B00110000, B00110100, B00010000};

static UC1701 lcd;

struct StoredHeaterTimes
{
  unsigned long sanitizeTimes;
  unsigned long nightHeatTimes;
};

StoredHeaterTimes storedHeaterTimes;

void setupRTC(bool runReset)
{
  while (!MCP7940.begin())
  {                                                                         // Initialize RTC communications
    SerialUSB.println(F("Unable to find MCP7940M. Checking again in 3s.")); // Show error text
    delay(3000);                                                            // wait a second
  }                                                                         // of loop until device is located
  SerialUSB.println(F("MCP7940 initialized."));
  while (!MCP7940.deviceStatus())
  { // Turn oscillator on if necessary
    SerialUSB.println(F("Oscillator is off, turning it on."));
    bool deviceStatus = MCP7940.deviceStart(); // Start oscillator and return state
    if (!deviceStatus)
    {                                                                  // If it didn't start
      SerialUSB.println(F("Oscillator did not start, trying again.")); // Show error and
      delay(1000);                                                     // wait for a second
    }                                                                  // of if-then oscillator didn't start
  }                                                                    // of while the oscillator is off

  if (runReset)
  {
    MCP7940.adjust(); // Set to library compile Date/Time
  }
}

void setupEEPROM(bool runReset)
{
  if (runReset)
  {
    // set to zero
    memset((uint8_t *)&storedHeaterTimes, 0, sizeof(storedHeaterTimes));
    // write
    eeprom50.writeBlock(EEPROM_HEATER_ADDR, (uint8_t *)&storedHeaterTimes, sizeof(storedHeaterTimes));
  }
  eeprom50.readBlock(EEPROM_HEATER_ADDR, (uint8_t *)&storedHeaterTimes, sizeof(storedHeaterTimes));
}

void setupLibrary()
{
  getAlarmFsm()->setup();
  getHeaterFSM()->setup();

  pinMode(ALARM_PIN, OUTPUT);
  pinMode(ALARM_SILENCE_BUTTON_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
}

void clear_lcd()
{
  for (int y = 0; y <= 7; y++)
  {
    for (int x = 0; x <= 128; x++)
    {
      lcd.setCursor(x, y);
      lcd.print(" ");
    }
  }
}

void setup()
{
  // PCD8544-compatible displays may have a different resolution...
  lcd.begin();
  // Add the smiley to position "0" of the ASCII table...
  lcd.createChar(0, glyph);
  clear_lcd();

  eeprom50.begin();

  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_ENTER_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);

  SerialUSB.begin(9600);
  SerialUSB.println("HI");
  sensorsLocal.begin();
  sensorsRemote.begin();

  lcd.setCursor(0, 0);
  lcd.print("PRESS ENTER TO RESET SYSTEM (in 1 sec)");
  delay(1000);
  bool runReset = false;
  if (digitalRead(BUTTON_ENTER_PIN) == LOW)
  {
    runReset = true;
    lcd.setCursor(0, 2);
    lcd.print("...RESETTING...");
  } else {
    lcd.setCursor(0, 2);
    lcd.print("...STARTING UP...");
  }
  delay(2000);
  clear_lcd();

  setupRTC(runReset);
  setupEEPROM(runReset);

  setupLibrary();
}

void update_temperature_readings()
{
  // Send the command to get temperatures
  sensorsLocal.requestTemperatures();
  sensorsRemote.requestTemperatures();

  getHal()->set_hour_of_day(MCP7940.now().hour());

  float tank_temp_f = sensorsLocal.getTempFByIndex(TANK_TEMP_SENSOR_INDEX);
  float water_shower_out_f = sensorsLocal.getTempFByIndex(WATER_SHOWER_SENSOR_INDEX);
  float tank_in_temp_f = sensorsLocal.getTempFByIndex(TANK_IN_TEMP_INDEX);

  float solar_temp_f = sensorsRemote.getTempFByIndex(0);

  getHal()->set_temps(tank_temp_f, water_shower_out_f, tank_in_temp_f, solar_temp_f);
}

void store_eeprom_values()
{
  eeprom50.updateBlock(EEPROM_HEATER_ADDR, (uint8_t *)&storedHeaterTimes, sizeof(storedHeaterTimes));
}

// buffer for displaying info on display
char inputBuffer[256];

unsigned long last_millis_loop = 0;
unsigned long last_millis_alarm = 0;
unsigned long last_update_heaters_millis = 0;
unsigned long last_flush_eeprom_millis = 0;

bool run_program()
{
  unsigned long curMillis = millis();

  // every 30 seconds
  if (curMillis - last_millis_loop > MAIN_HEATER_LOOP_SECONDS * 1000)
  {
    update_temperature_readings();
    // 1 update HAL with machine state
    getHal()->set_heater(digitalRead(HEATER_PIN));
    getHal()->set_pump(digitalRead(PUMP_PIN));
    getHal()->set_seconds(curMillis / 1000);

    getHeaterFSM()->run();

    // 2 flush HAL with new state
    digitalWrite(HEATER_PIN, getHal()->get_heater());
    digitalWrite(PUMP_PIN, getHal()->get_pump());

    last_millis_loop = curMillis;
  }

  if (curMillis - last_millis_alarm > ALARM_LOOP_SECONDS * 1000)
  {
    update_temperature_readings();
    getHal()->set_alarm_silence_button(digitalRead(ALARM_SILENCE_BUTTON_PIN));
    getHal()->set_alarm_state(digitalRead(ALARM_PIN));
    getHal()->set_seconds(curMillis / 1000);
    getAlarmFsm()->run();
    digitalWrite(ALARM_PIN, getHal()->get_alarm_state());

    last_millis_alarm = curMillis;
    return true;
  }

  if (curMillis - last_update_heaters_millis > 1000)
  {
    if (getHal()->heater)
    {
      if (getHeaterFSM()->fsm->is_in_state(getHeaterFSM()->state_sanitizing))
      {
        storedHeaterTimes.sanitizeTimes += 1;
      }
      else
      {
        storedHeaterTimes.nightHeatTimes += 1;
      }
    }

    last_update_heaters_millis = millis();
  }

  // flush eeprom every 5 mins
  if (curMillis - last_flush_eeprom_millis > 5 * 60 * 1000)
  {
    store_eeprom_values();
    last_flush_eeprom_millis = millis();
  }

  return false;
}

void update_display()
{
  const char *current_alarm_state = getAlarmFsm()->fsm->current_state()->name;
  const char *current_heater_state = getHeaterFSM()->fsm->current_state()->name;
  Temps temps = getHal()->temps;

  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  lcd.print(current_heater_state);
  lcd.print(' ');
  lcd.print(current_alarm_state);
  lcd.print("      ");

  // Write the counter on the second line...
  lcd.setCursor(0, 1);
  lcd.write(' ');
  lcd.write(0); // write the smiley

  lcd.write(' ');
  DateTime now = MCP7940.now();
  sprintf(inputBuffer, "%02d:%02d", now.hour(), now.minute());
  lcd.print(inputBuffer);

  SerialUSB.print("Requesting temperatures...");

  SerialUSB.println("DONE");

  // pull up, GND is activated (on = 0, off = 1)
  bool cancelPushed = !digitalRead(ALARM_SILENCE_BUTTON_PIN);
  lcd.setCursor(0, 2);
  lcd.print(cancelPushed ? "Cancel pushed" : "Normal");

  lcd.setCursor(0, 3);
  lcd.print("Tank: ");
  lcd.print(int(temps.tank_temp_f));
  lcd.print("  ");
  lcd.print("Shower out: ");
  lcd.print(int(temps.water_shower_out_f));
  lcd.setCursor(0, 4);
  lcd.print("Solar: ");
  lcd.print(int(temps.solar_temp_f));
  lcd.print(" ");
  lcd.print("Tank in solar: ");
  lcd.print(int(temps.tank_in_temp_f));

  lcd.setCursor(0, 5);
  lcd.print("heater: ");
  lcd.print(storedHeaterTimes.sanitizeTimes / 60);
  lcd.print(" / ");
  lcd.print(storedHeaterTimes.nightHeatTimes / 60);
}

void loop()
{
  bool should_update_display = run_program();
  if (should_update_display)
  {
    update_display();
  }
}
