# OneShot Relais Timer

This is a relais timer module that switches a AC mains load ON for a defined period of time.

![Assembly1](https://github.com/M1S2/OneShot_Relais_Timer/raw/master/Images/Assembly1.jpg)

## PCB
The schematics and PCBs were created using EAGLE. They were designed to be manufactured using toner transfer method so all traces are as big as possible.

There are two PCBs:
- The Main_PCB contains the microcontroller, the relay and the transformer and regulators used to generate all needed voltages.

	<img src="https://github.com/M1S2/OneShot_Relais_Timer/raw/master/Images/Main_PCB_Top.jpg" width="45%"/>
	<img src="https://github.com/M1S2/OneShot_Relais_Timer/raw/master/Images/Main_PCB_Bottom.jpg" width="45%"/>
- The UI_PCB contains the buttons, the 7-segment display and the needed BCD-7-segment driver IC.

	<img src="https://github.com/M1S2/OneShot_Relais_Timer/raw/master/Images/UI_PCB_Top.jpg" width="45%"/>
	<img src="https://github.com/M1S2/OneShot_Relais_Timer/raw/master/Images/UI_PCB_Bottom.jpg" width="45%"/>

## Housing
The housing of a mechanical timer was used to containg the PCBs.
The following parts are needed additionally:
- 1x Grundplatte.stl
- 1x Abdeckung_Symbole.stl (should be printed with 2 colors)
- 3x Spacer.stl
- 3x Screw M3x30
- 1x Screw M3x12
- 4x Sheet Metal Screw 15mm long

<img src="https://github.com/M1S2/OneShot_Relais_Timer/raw/master/Images/Parts_overview.jpg" width="45%"/>
<img src="https://github.com/M1S2/OneShot_Relais_Timer/raw/master/Images/Screws.jpg" width="45%"/>

## Software
The software was written for the ATTiny204 microcontroller.

A statemachine is running in the main loop with the following two states:
- SET: The hours can be modified using the set button. The allowed range is 0 to 9. If the start/stop button is pressed, the relay is toggled and the state is changed to COUNTDOWN.
- COUNTDOWN: If the remaining seconds are equal 0 (counted by the PIT interrupt) or the start/stop button is pressed, the relay is toggled and the state is changed to SET.

It uses the following peripherals:
- TCA0: Timer used to query the buttons. It calls the button debounce routine of Peter Dannegger (http://www.mikrocontroller.net/articles/Entprellung).
- PIT: Periodic Interrupt Timer used to count the remaining seconds in COUNTDOWN mode. It also toggles the 7-segment dot in COUNTDOWN mode.

### 0-hours
If the hours are set to 0, the start/stop button can be used to directly toggle the relay. By toggling the relay, an inverted mode is activated, because the relay is toggled when the (normal) countdown is started.
E.g.:
- The set button is used to set the hours to 0
- The start/stop button is used to toggle the relay on.
- The set button is used to set the hours to e.g. 1
- The start/stop button is used to start the countdown > the relay is switched off and on after 1 hour (inverted mode)

### Last set hour
The last set hours are saved when the Countdown is started and are reload on startup.