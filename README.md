# Solar Water Controller

This is a solar water controller deployed at Burning Man 2022 to manage a water heater and pump system for the Kostume Kult theme camp.

### Code Architecture

The controller code is divided into two halves:

1. A generic state machine implementation in c++
2. A arduino / wiring main file that integrates the generic c++ code into actual hardware.

### Physical System Design

The target hardware for part 2 is a https://industruino.com/shop/product/industruino-proto-d21g-8?category=2 connected to a generic 4-channel low side mosfet driver to switch 24v relays controlling pumps along with controlling a contactor connected to a hot water heater. https://shop.inpublic.space/products/esp32-c3-4-channel-dimmer (Mosfet board). Additionally, 3 digital temperature sensor modules were deployed in different parts of the water system to monitor various temperatures.


The overall physical system design was:

1. A typical hot water heater used as a storage tank and heater for when the sun wasn't up.
2. A solar hot water set of tubing with a pump system for that.
3. A pump system for a shower output and for a water-bottle filling station.

The industrino was chosen for the complete form factor, including an RTC, EEPROM, Display, and buttons along with sufficient proto board space and fits cleanly on a DIN rail.

We had no issues with the controller during the burn but were unable to test the software fully integrated (hence the two-part design). The pumps did turn on and off at the correct times and the readouts for the temperature were useful.

We plan to deploy this system in 2023 and property test all functionality of the software fully connected to the hardware.

### Software system design

The overall design for the c++ components of this project live in lib, and are tested in the test folder.

These are:

1. `alarmmachine` – an over-temperature alarm watch state machine. This alerts us through a buzzer/siren that the temperature is above a safety point and a covering needs to be placed on the solar panels to prevent pressure build up. Thankfully, this alarm did not need to be used at the burn.
2. `fsm` - a fork of arduino-fsm that removes the timing features and simplifies some implementation details.
3. `hal` – a custom class that takes in all input and outputs. Used to interface between the actual hardware implementation and the software testbench.
4. `heatermachine` – this is the main FSM that interfaces with the HAL to setup the time-based and temperature based pump and heating rules.

### Operating principles

This controller has 2 different modes: day and night.

In day mode, the circulator runs only when the solar temperature is greater than the tank tempearture.

A little before sunset, the tank is brought up to a SANITIZE cycle to kill any potential bacteria turning on the heating element of the tank to 125 deg f (STERI_CYCLE_END_TEMP).

At night, the tank is maintained to turn on at 85 deg f (NIGHT_HEATER_ON_THRESHOLD_DEG_F) and off at 90 deg f (NIGHT_HEATER_OFF_THRESHOLD_DEG_F).

### HAL interfaces

The industrino pinout used was:

* One wire sensor remote – monitored the solar temperature
* One wire sensor local – monitored the 1. tank temp, 2. shower water out, 3. solar panel in temp. Ideally, 3 and remote should be relatively similar when the solar pump is activated. These sensors are digital and 3 are wired to one pin. They are assigned by index defined in the constant files.
* Heater output – this controlled the mosfet that controlled a high current solid state relay controlling the hot water coil.
* Pump output – output powering the solar circulating pump. Largely temperature controlled – if the temperature in the solar module was higher than the tank the circulator would turn on to equalize the temperature.

### Getting started with development

1. To run tests `pio run -t test`
2. To upload program `pio run -t upload`

Once the program is flashed, the current date and time are included in the program code. When the controller is reset the current date and time is copied from the program into the RTC module (with a lithium battery backup in the industrino) and the EEPROM time counters are reset.