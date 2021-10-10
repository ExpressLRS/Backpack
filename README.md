## What is a "TX Backpack"?
Some of the ExpressLRS TX modules include an additional ESP8285 chip, which lets us communicate wirelessly with other ESP8285 enabled devices using a protocol called espnow. We call this chip the "TX-Backpack". The aim of the TX-Backpack is to allow wireless communication between ExpressLRS, and other FPV related devices for command and control, or for querying config.

## Sounds interesting... What type of FPV devices can it talk to?
 A prime use case is your video receiver module (or VRX). Currently there aren't many VRX modules that have an ESP8285 built in to allow them to communicate with ExpressLRS, so you need to add your own. A small ESP01F can be "piggybacked" onto your VRX module, which allows ExpressLRS to control the band and channel that your goggles are set to. We call this device the "VRX-Backpack". 

## Wow cool, so I'll be able to control the module via ELRS!? Which VRX modules does it work with?
At the moment, only the ImmersionRC Rapidfire module is supported.
(Note: there is also support for espnow comms to the FENIX VRX module, which is another open source project).

## Great! I use the Rapidfire module. How do I get a VRX-Backpack?
Currently you need to build your own, but we're hoping this will change in the not-too-distant future.

## Ok, I'm up for a challenge... How do I build a VRX-Backpack for my Rapidfire?
It's actually pretty simple...

https://github.com/ExpressLRS/Backpack/wiki/Building-a-VRX-Backpack