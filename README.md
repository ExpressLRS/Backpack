# What is a "TX Backpack"?
Some of the ExpressLRS TX modules include an additional ESP8285 chip, which lets us communicate wirelessly with other ESP8285 enabled devices using a protocol called espnow. We call this chip the "TX-Backpack". The aim of the TX-Backpack is to allow wireless communication between ExpressLRS, and other FPV related devices for command and control, or for querying config.

# Sounds interesting... What type of FPV devices can it talk to?
 A prime use case is your video receiver module (or VRX). Currently there aren't many VRX modules that have an ESP8285 built in to allow them to communicate with ExpressLRS, so you need to add your own. A small ESP01F can be "piggybacked" onto your VRX module, which allows ExpressLRS to control the band and channel that your goggles are set to. We call this device the "VRX-Backpack". 

# Wow cool, so I'll be able to control the module via ELRS!? Which VRX modules does it work with?
At the moment, only the ImmersionRC Rapidfire module is supported.
(Note: there is also support for espnow comms to the FENIX VRX module, which is another open source project).

# Great! I use the Rapidfire module. How do I get a VRX-Backpack
Currently you need to build your own, but we're hoping this will change in the not-too-distant future.

# Ok, I'm up for a challenge... How do I build a VRX-Backpack for my Rapidfire?
It's actually pretty simple...

BOM:
- 1x ESP01F https://www.aliexpress.com/item/32957577407.html
- 1x AMS1117 3.3V Regulator https://www.aliexpress.com/item/32910803907.html
- Silicon wire

1. Using the pinout for the AMS1117, solder 2x black silicon wires to GND pin, and 1x red silicon wires to each of the other pins
<add AMS1117 pinout>
<add AMS1117 pic>

Using the silkscreen on the back of the ESP01F:
2. Solder one of the GND wires from the reg to the top left GND pad on the ESP01F
3. Solder the output from the reg to the 3V3 pad on the ESP01F
4. Solder the rest of the silicon wires as per the diagram below:
<add ESP01F diagram>
<add ESP01F pic>

5. Stick a peice of kapton tape over the sheild on the ESP01F, then heat shrink the reg to the ESP01F:
<add ESP01F heat shrink pic>

6. Connect TX, RX, GND, and 5V (the input pin on the reg) to a FTDI
7. Bridge the IO0 wire to GND as you power up your FTDI
8. Under the `Rapidfire_Backpack` target in platformio, select Upload
9. Once flashed, you can remove the wires from the following pads: TX, RX, IO0
10. Solder 5V, GND, CLK, DATA and CS to the rapidfire pin header, as per the diagram below:
<add rapidfire diagram>
<add rapidfire pic>

11. Install the Rapidfire back into the module, and tuck the VRX-Backpack into the space above the Rapidfire:
<add install pic>
