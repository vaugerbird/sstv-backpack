# sstv-backpack
## A ESP32-CAM SSTV backpack for the Baofeng UV-5R and others!

<img align="center" src="./img/radio.jpg" width="300">
<img align="center" src="./img/sstv1.jpg" width="300">
<img align="center" src="./img/sstv2.jpg" width="300">

## Hardware Revisions
I currently have 2 hardware revisions available, with a 3rd planned: 
1. v1.0:
    * Mostly through-hole and completely hand-solderable
    * Simplest version with only a voltage regulator circuit, 2 switches, a buzzer, and the ESP32-CAM
    * Requires external board for flashing and a serial monitor
2. v1.1: 
    * Still mostly through-hole and hand-solderable, albeit with some fine-pitch components like the FTDI chip
    * Retains all the same functionality of v1.0, but adds an FTDI USB-to-Serial chip for flashing and a serial monitor.
    * The board is quite cramped and may be hard to work with
3. v1.2 (PLANNED):
    * Switch to an entirely top-mounted SMD design, while still being hand-solderable
    * Retain all features of the v1.1 board
    * Pogo pins for power will be mounted on the board
    * Switch to USB-C instead of USB Mini-B  

## Assembly Instructions
1. Upload your desired [Gerber](./hardware/gerbers/) file to your desired PCB manufacturer. I used OSHPark, but JLCPCB or PCBWay are good manufacturers too.

2. Order the required parts using the proper BOM for [v1.0](./hardware/v1.0/sstv-backpack.csv) or [v1.1](./hardware/v1.1/sstv-backpack.csv) from your favorite parts supplier.

3. Once all parts arrive, assemble the board. There isn't much of a technique to it, just start with the lowest profile parts first like the diodes, resistors, and chips, and work your way up.

4. Once the board is complete, print the case using one of the included [STLs](./hardware/case). Currently, the Baofeng UV-5R is the only supported radio, but the Inventor (2025) files are included and help with additional radio models is always appreciated!

    * I used the standard settings (0.2mm) on my print, with supports on, and the case interior facing the build plate.

5. After you have the hardware assembled, the next thing to do is upload the code!

## Programming Instructions
1. Ensure that you have this link added to the 3rd party boards section of the Arduino IDE: https://dl.espressif.com/dl/package_esp32_index.json

2. From the board manager, install version **2.0.17** of the ESP32 board definitions. Any newer version will throw compilation errors.

3. Change the text options on lines 24 and 25 to reflect your callsign and desired splash text.

4. All the board settings can be left alone except for selecting the correct COM port.

5. Upload the code!

## Licensing
* The [main program](./code/sstv-backpack.ino) and [supporting libraries](./code/) are licensed as CC BY-NC-SA 4.0 from [desafioinventor](https://www.instructables.com/SSTV-Capsule-V2-for-High-Altitude-Balloons/) on Instructables. 
* The case design is roughly based on [this design](https://www.thingiverse.com/thing:5270394) by HStakhiv on [Thingiverse](https://www.thingiverse.com/hstakhiv/designs). The original is licensed under CC BY. My case design falls under the GPL-2.0 license.
* The remaining contents of this repository are designed by myself, and are licensed under the GPL-2.0 license.
