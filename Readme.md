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

TBD....