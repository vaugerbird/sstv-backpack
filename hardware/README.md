# Hardware
## Currently there are 3 revisions:
* v1.0(.1)
    * Simplest version
    * The board only acts as a voltage regulator, PTT circuit, and carrier for the major parts
* v1.1
    * More complex
    * The board contains the voltage regulator circuit, USB flashing circuit, and PTT circuit
* v1.2
    * Same complexity as v1.1, contains mostly SMD parts

## HW Issues
* v1.0(.1):
    * This board is incapable of flashing the ESP32 or using serial monitor as there is no UART-to-USB chip present. Flashing has to be done with a dedicated programmer and debugging is very difficult. 
* v1.1:
    * No CC resistors are present (or really possible) for the USB-C port. This is due to physical restraints of through-hole components, and means the board will not receive power from a USB-C to USB-C cable.
    * Powering the board from the radio and connecting USB data is possible, and won't break anything as power is diode-protected, but is not recommended.
* v1.2:
    * No issues have been found; this revision of the board has not yet been tested.
