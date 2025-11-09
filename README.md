# sstv-backpack
## A ESP32-CAM SSTV backpack for the Baofeng UV-5R and others!

<img align="left" src="./img/radio.jpg" width="300">
<img align="center" src="./img/sstv1.jpg" width="300">
<img align="center" src="./img/sstv2.jpg" width="300">

## Project Description
### Turn your HT into a point-and-shoot camera!
This project relies on using an ESP32 connected to a handheld transceiver to send SSTV images. Any combination of HT and ESP32-CAM can send a picture, but the hardware in this repository is specifically designed to work!

To decode the images, another radio must be used and connected to any device that can run a decoder program. I personally use MMSSTV on my PC with a USB soundcard, or my phone running Robot36 and listening to the radio with the microphone. 

Here's some questions I've been asked in the past:
* **Why?**
    * This is a term project that I created for my college ARC that I'm the president of! We are currently assembling v1.1 and we plan on using the completed projects to do a SSTV scavenger hunt. This consists of a list of things to take pictures of around campus, where the hunter will take a picture of the required object, send the image back to the control room (clubroom), and the point will be awarded to the hunter/team!
* **How does this work?**
    * In short, the ESP32-CAM captures a frame, converts the image into the appropriate encoded audio for SSTV, keys the PTT on the HT, and plays the audio through the microphone port. The specifics of the encoding process are somewhat black magic to me as I didn't create the underlying code, but as far as I understand, the encoding is done by applying a few math functions to the pixels stored in the framebuffer, and converted into audio.

## Hardware Revisions
I currently have 2 hardware revisions available, with a 3rd planned: 
1. v1.0:
    * Mostly through-hole and completely hand-solderable
    * Simplest version with only a voltage regulator circuit, PTT circuit, 2 switches, a buzzer, and the ESP32-CAM
    * Requires external board for flashing and a serial monitor
2. v1.1: 
    * Still mostly through-hole and hand-solderable, albeit with some fine-pitch components like the FTDI chip
    * Retains all the same functionality of v1.0, but adds an FTDI USB-to-Serial chip for flashing and a serial monitor.
    * The board is quite cramped and may be hard to work with
3. v1.2 (WIP):
    * Switch to an mostly top-mounted SMD design, while still being hand-solderable
    * Retain all features of the v1.1 board
    * Maybe add battery charging circuitry?

## Assembly Instructions
### Better (printable) instructions coming soon!
1. Upload your desired [Gerber](./hardware/gerbers/) file to your favorite PCB manufacturer. I recommend OSHPark for US-made, decent priced small-batch PCBs. JLCPCB or PCBWay are both great for cheap, high quality PCBs, if you don't mind waiting.

2. Order the required parts using the proper BOM for [v1.0](./hardware/v1.0/sstv-backpack-v1.0.1.csv) or [v1.1](./hardware/v1.1/sstv-backpack-v1.1.csv) from your favorite parts supplier.

3. Once all parts arrive, assemble the board. There isn't much of a technique to it, just start with the lowest profile parts first like the diodes, resistors, and chips, and work your way up.

4. Once the board is complete, print the case using one of the included [STLs](./hardware/case). Currently, the Baofeng UV-5R is the only supported radio, but the Inventor 2025 and STEP files are included, and help with additional radio models is always appreciated!

    * I used the standard settings (0.2mm) on my print, with supports on, and the case interior facing the build plate.

    * The case requires these additional parts:
        * 4 - M3 brass het set inserts
        * 4 - M3 x 6mm screws (preferably hex head)
        * 4 - M2 x 4mm screws
        * 1 - Radio cable (I just cut the end off of a cheap Baofeng speaker-mic, it was cheaper than using an unterminated cable.)

5. After you have the hardware assembled, the next thing to do is upload the code!

## Programming Instructions
1. Ensure that you have this link added to the 3rd party boards section of the Arduino IDE (Found under File -> Preferences -> Additional board manager URLs):
```
https://dl.espressif.com/dl/package_esp32_index.json
```

2. From the board manager, install the newest version of the ESP32 board definitions.

3. Set the board model to "AI Thinker ESP32-CAM" and select the appropriate COM port.

4. Change the text options on lines 37 and 38 of [sstv-backpack.ino](./code/sstv-backpack.ino) to reflect your callsign and desired splash text.
    * The color of the top and bottom text can be changed with the definitions in lines 24-31. 

5. Uncomment line 21 ```#define USE_FLASH``` if you want to use the flash LED (Turns on a high intensity white LED on when framegrabbing).

6. At line 447, set `1 = HIGH` or `0 = LOW` properly reflect your CAPT_BTN wiring (GND for LOW, or +3.3V for HIGH)

7. Between line 452-456, choose the correct button pull-up/pull-down configuration. The button is set to pull-up by default to mirror the PCBs. (Button connects to GND when pressed)

8. Inside "camera.h" file you can choose between two profiles, DAYLIGHT and HOME. Feel free to modify settings to best suit your case. 
	* Notes from IU5HKU:
		* Keep in mind that this cheap sensor struggles a lot in poor light scenarios, so don't expect a picture like your Fujifilm camera.
		* A quick search online can lead you towards the proper settings values you need.

9. All the board settings can be left alone except for selecting the correct COM port.

10. Upload the code!

## Contributors
 A huge thank you to contributors to this project! I'm pretty bad with code so all help is greatly appreciated!

 * [IU5HKU / Marco](https://github.com/IU5HKU) - Updated the code to work with the newest version of the ESP32 board definition and improved the readability/useability of the code!
 * [IU5HKU / Marco](https://github.com/IU5HKU) - Updated the code to work with deepsleep/wakeup ESP32 functions (maximize battery life).

## Licensing
* The [main program](./code/sstv-backpack.ino) and [supporting libraries](./code/) are licensed as CC BY-NC-SA 4.0 and are based on the program/libraries from [desafioinventor](https://www.instructables.com/SSTV-Capsule-V2-for-High-Altitude-Balloons/) on Instructables. 
* The case design is roughly based on [this design](https://www.thingiverse.com/thing:5270394) by HStakhiv on [Thingiverse](https://www.thingiverse.com/hstakhiv/designs). The original is licensed under CC BY. My case design falls under the GPL-2.0 license.
* The remaining contents of this repository are designed by myself (and contributors!), and are licensed under the GPL-2.0 license.

