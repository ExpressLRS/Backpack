# ExpressLRS Backpack Protocol Specification v1.1

## Introduction
This protocol defines the message set to be used for bi-directional communications between an ExpressLRS “backpack” and a third party device, such as a video receiver module, or a lap timer, etc. The protocol has been built on the MSPv2 standard for compatibility with other open source firmware.

## Definitions

| Abbreviation | Definition                                                                                                       |
| ------------ | ---------------------------------------------------------------------------------------------------------------- |
| VRX          | Video receiver module. This is the module that plugs into the FPV goggles, and receives analog or digital video. |
| TX           | Transmitter module. This is the module that connects to the handset, and broadcasts the ExpressLRS OTA protocol. |
| ELRS         | ExpressLRS                                                                                                       |
| UART         | Universal asynchronous receiver-transmitter. The serial interface for communication between peers.               |
| Opcode       | Operation Code. Defines a command or ID that both peers understand. This is the message ID.                      |

## Revision Log

| Version | Changes                                                                            |
| ------- | ---------------------------------------------------------------------------------- |
| 1.0     | Initial protocol release for Backpack firmware v1.0                                |
| 1.1     | Adopted MSP\_DISPLAYPORT message payload format for the “Set OSD Element” message. |
|         |                                                                                    |
|         |                                                                                    |
|         |                                                                                    |

## Application Layer
### 4.1 Message Framing
The application layer messages between peers are framed using MSPv2 (see https://github.com/iNavFlight/inav/wiki/MSP-V2).

The MSPv2 packet structure is defined as:

| Offset | Usage        | Included in CRC | Comment                                                                                     |
| ------ | ------------ | --------------- | ------------------------------------------------------------------------------------------- |
| 0      | $            |                 | Starting char                                                                               |
| 1      | X            |                 | 'X' in place of v1 'M'                                                                      |
| 2      | type         |                 | '<' / '>' / '!' (see message types below)                                                   |
| 3      | flag         | yes             | uint8, flag, usage to be defined (set to zero)                                              |
| 4      | function     | yes             | uint16 (little endian). 0 - 255 is the same function as V1 for backwards compatibility      |
| 6      | Payload size | yes             | uint16 (little endian) payload size in bytes                                                |
| 8      | Payload      | yes             | n (up to 65535 bytes) payload. Data structures larger than one byte are sent little endian. |
| n+8    | checksum     |                 | uint8, (n= payload size), crc8\_dvb\_s2 checksum                                            |

Where the message type is one of:
| Identifier | Type     | Sent from     | Processed by  | Comment                                                                                                                          |
| ---------- | -------- | ------------- | ------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| ‘<’        | Request  | Master        | Slave         |                                                                                                                                  |
| ‘>’        | Response | Slave         | Master        | Only sent in response to a request                                                                                               |
| ‘!’        | Error    | Master, Slave | Master, Slave | Response to receipt of data that cannot be processed (corrupt checksum, unknown function, message type that cannot be processed) |

## Messages
### 5.1 Message Opcodes
Proposed VRx specific opcode range **0x0300-0x03FF**.
For detailed message descriptions, see **section 5.2** below.

| Message                | Opcode | Page Link                                                                                                                             |
| ---------------------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------- |
| Get band/channel index | 0x0300 | [Get band/channel index](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.ghas5bhbg7qv) |
| Set band/channel index | 0x0301 | [Set band/channel index](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.mwelg69z8gtu) |
| Get frequency          | 0x0302 | [Get frequency](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.sww9f2t9lvlc)          |
| Set frequency          | 0x0303 | [Set frequency](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.d9zo34q8rrxe)          |
| Get recording state    | 0x0304 | [Get recording state](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.n22kb05wbten)    |
| Set recording state    | 0x0305 | [Set recording state](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.5xq93verbyfb)    |
| Get VRx mode           | 0x0306 | [Get VRX mode](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.sptbfkucnghq)           |
| Set VRx mode           | 0x0307 | [Set VRX mode](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.5ec2cfivyfck)           |
| Get RSSI               | 0x0308 | [Get RSSI](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.treu6ha0qmwd)               |
| Get battery voltage    | 0x0309 | [Get battery voltage](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.uvmbnco4tm7l)    |
| Get firmware           | 0x030A | [Get firmware version](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.iu6n415w6xl9)   |
| Set buzzer             | 0x030B | [Set buzzer](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.cxkusoci25aa)             |
| Set OSD Element        | 0x00B6 | [Set OSD element](https://docs.google.com/document/d/1u3c7OTiO4sFL2snI-hIo-uRSLfgBK4h16UrbA08Pd6U/edit#heading=h.oyx2mkj2eyvw)        |

### 5.2 Messages
**Get band/channel index**
| Byte | Value  | Included in CRC | Comment                                 |
| ---- | ------ | --------------- | --------------------------------------- |
| 0    | $      |                 | Starting char                           |
| 1    | X      |                 | 'X' in place of v1 'M'                  |
| 2    | ‘<'    |                 | Request message from backpack to device |
| 3    | 0      | yes             | Flag. Set to zero                       |
| 4    | 0x0300 | yes             | Message opcode                          |
| 6    | 0      | yes             | Payload size                            |
| 8    | <crc>  |                 | checksum                                |

**Get band/channel index response**

| Byte | Value   | Included in CRC | Comment                                                          |
| ---- | ------- | --------------- | ---------------------------------------------------------------- |
| 0    | $       |                 | Starting char                                                    |
| 1    | X       |                 | 'X' in place of v1 'M'                                           |
| 2    | ‘>'     |                 | Response message from device to backpack                         |
| 3    | 0       | yes             | Flag. Set to zero                                                |
| 4    | 0x0300  | yes             | Message opcode                                                   |
| 6    | 1       | yes             | Payload size                                                     |
| 8    | \<index> | yes             | uint8. The current VRX index 0-47 (standard 48 frequency table)  |
| 9    | \<crc>   |                 | checksum                                                         |

**Set band/channel index**

| Byte | Value   | Included in CRC | Comment                                                      |
| ---- | ------- | --------------- | ------------------------------------------------------------ |
| 0    | $       |                 | Starting char                                                |
| 1    | X       |                 | 'X' in place of v1 'M'                                       |
| 2    | ‘<'     |                 | Command message from backpack to device                      |
| 3    | 0       | yes             | Flag. Set to zero                                            |
| 4    | 0x0301  | yes             | Message opcode                                               |
| 6    | 1       | yes             | Payload size                                                 |
| 8    | \<index> | yes             | uint8. The new VRX index 0-47(standard 48 frequency table) |
| 9    | \<crc>   |                 | checksum                                                     |

**Get frequency**

| Byte | Value  | Included in CRC | Comment                                 |
| ---- | ------ | --------------- | --------------------------------------- |
| 0    | $      |                 | Starting char                           |
| 1    | X      |                 | 'X' in place of v1 'M'                  |
| 2    | ‘<'    |                 | Request message from backpack to device |
| 3    | 0      | yes             | Flag. Set to zero                       |
| 4    | 0x0302 | yes             | Message opcode                          |
| 6    | 0      | yes             | Payload size                            |
| 8    | \<crc>  |                 | checksum                                |

**Get frequency response**

| Byte | Value       | Included in CRC | Comment                                  |
| ---- | ----------- | --------------- | ---------------------------------------- |
| 0    | $           |                 | Starting char                            |
| 1    | X           |                 | 'X' in place of v1 'M'                   |
| 2    | ‘>'         |                 | Response message from device to backpack |
| 3    | 0           | yes             | Flag. Set to zero                        |
| 4    | 0x0302      | yes             | Message opcode                           |
| 6    | 2           | yes             | Payload size                             |
| 8    | \<frequency> | yes             | uint16. The current VRX frequency in MHz |
| 10   | \<crc>       |                 | checksum                                 |

**Set frequency:**

| Byte | Value       | Included in CRC | Comment                                 |
| ---- | ----------- | --------------- | --------------------------------------- |
| 0    | $           |                 | Starting char                           |
| 1    | X           |                 | 'X' in place of v1 'M'                  |
| 2    | ‘<'         |                 | Command message from backpack to device |
| 3    | 0           | yes             | Flag. Set to zero                       |
| 4    | 0x0303      | yes             | Message opcode                          |
| 6    | 2           | yes             | Payload size                            |
| 8    | \<frequency> | yes             | uint16. The new VRX frequency in MHz    |
| 10   | \<crc>       |                 | checksum                                |

**Get recording state:**

| Byte | Value  | Included in CRC | Comment                                 |
| ---- | ------ | --------------- | --------------------------------------- |
| 0    | $      |                 | Starting char                           |
| 1    | X      |                 | 'X' in place of v1 'M'                  |
| 2    | ‘<'    |                 | Request message from backpack to device |
| 3    | 0      | yes             | Flag. Set to zero                       |
| 4    | 0x0304 | yes             | Message opcode                          |
| 6    | 0      | yes             | Payload size                            |
| 8    | \<crc>  |                 | checksum                                |

**Get recording state response:**

| Byte | Value   | Included in CRC | Comment                                  |
| ---- | ------- | --------------- | ---------------------------------------- |
| 0    | $       |                 | Starting char                            |
| 1    | X       |                 | 'X' in place of v1 'M'                   |
| 2    | ‘>'     |                 | Response message from device to backpack |
| 3    | 0       | yes             | Flag. Set to zero                        |
| 4    | 0x0304  | yes             | Message opcode                           |
| 6    | 1       | yes             | Payload size                             |
| 8    | \<state> | yes             | uint8. 0=off, 1=on                       |
| 9    | \<crc>   |                 | checksum                                 |

**Set recording state:**

| Byte | Value   | Included in CRC | Comment                                                          |
| ---- | ------- | --------------- | ---------------------------------------------------------------- |
| 0    | $       |                 | Starting char                                                    |
| 1    | X       |                 | 'X' in place of v1 'M'                                           |
| 2    | ‘<'     |                 | Command message from backpack to device                          |
| 3    | 0       | yes             | Flag. Set to zero                                                |
| 4    | 0x0305  | yes             | Message opcode                                                   |
| 6    | 3       | yes             | Payload size                                                     |
| 8    | \<state> | yes             | uint8. The recording state to set, where 0=stop, 1=start         |
| 9    | \<delay> | yes             | uint16. The delay before stopping/starting recording in seconds. |
| 11   | \<crc>   |                 | checksum                                                         |

**Get VRX mode:**

| Byte | Value  | Included in CRC | Comment                                 |
| ---- | ------ | --------------- | --------------------------------------- |
| 0    | $      |                 | Starting char                           |
| 1    | X      |                 | 'X' in place of v1 'M'                  |
| 2    | ‘<'    |                 | Request message from backpack to device |
| 3    | 0      | yes             | Flag. Set to zero                       |
| 4    | 0x0306 | yes             | Message opcode                          |
| 6    | 0      | yes             | Payload size                            |
| 8    | \<crc>  |                 | checksum                                |

**Get VRX mode response:**

| Byte | Value  | Included in CRC | Comment                                                                             |
| ---- | ------ | --------------- | ----------------------------------------------------------------------------------- |
| 0    | $      |                 | Starting char                                                                       |
| 1    | X      |                 | 'X' in place of v1 'M'                                                              |
| 2    | ‘>'    |                 | Response message from device to backpack                                            |
| 3    | 0      | yes             | Flag. Set to zero                                                                   |
| 4    | 0x0306 | yes             | Message opcode                                                                      |
| 6    | 1      | yes             | Payload size                                                                        |
| 8    | \<mode> | yes             | uint8. The current VRX mode as an emun. To be determined based on VRX manufacturer. |
| 9    | \<crc>  |                 | checksum                                                                            |

**Set VRX mode:**

| Byte | Value  | Included in CRC | Comment                                                                         |
| ---- | ------ | --------------- | ------------------------------------------------------------------------------- |
| 0    | $      |                 | Starting char                                                                   |
| 1    | X      |                 | 'X' in place of v1 'M'                                                          |
| 2    | ‘<'    |                 | Command message from backpack to device                                         |
| 3    | 0      | yes             | Flag. Set to zero                                                               |
| 4    | 0x0307 | yes             | Message opcode                                                                  |
| 6    | 1      | yes             | Payload size                                                                    |
| 8    | \<mode> | yes             | uint8. The new VRX mode as an emun. To be determined based on VRX manufacturer. |
| 9    | \<crc>  |                 | checksum                                                                        |

**Get RSSI:**

| Byte | Value  | Included in CRC | Comment                                 |
| ---- | ------ | --------------- | --------------------------------------- |
| 0    | $      |                 | Starting char                           |
| 1    | X      |                 | 'X' in place of v1 'M'                  |
| 2    | ‘<'    |                 | Request message from backpack to device |
| 3    | 0      | yes             | Flag. Set to zero                       |
| 4    | 0x0308 | yes             | Message opcode                          |
| 6    | 0      | yes             | Payload size                            |
| 8    | \<crc>  |                 | checksum                                |

**Get RSSI response:**

| 0    | $              |     | Starting char                                                                                                                                                                   |
| ---- | -------------- | --- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1    | X              |     | 'X' in place of v1 'M'                                                                                                                                                          |
| 2    | ‘>'            |     | Response message from device to backpack                                                                                                                                        |
| 3    | 0              | yes | Flag. Set to zero                                                                                                                                                               |
| 4    | 0x0308         | yes | Message opcode                                                                                                                                                                  |
| 6    | \<size>         | yes | Payload size                                                                                                                                                                    |
| 8    | \<num antennas> | yes | uint8. The number of RSSI bytes to follow, one for each RSSI value. For example, if there are 2 antennas, the following bytes may be \[30, 45\] if they have RSSI of 30 and 45. |
| 9+n  | \<RSSI>         | yes | uint8. RSSI for each antenna. One byte for each.                                                                                                                                |
| 10+n | \<crc>          |     | checksum                                                                                                                                                                        |

**Get battery voltage:**

| Byte | Value  | Included in CRC | Comment                                 |
| ---- | ------ | --------------- | --------------------------------------- |
| 0    | $      |                 | Starting char                           |
| 1    | X      |                 | 'X' in place of v1 'M'                  |
| 2    | ‘<'    |                 | Request message from backpack to device |
| 3    | 0      | yes             | Flag. Set to zero                       |
| 4    | 0x0309 | yes             | Message opcode                          |
| 6    | 0      | yes             | Payload size                            |
| 8    | \<crc>  |                 | checksum                                |

**Get battery voltage response:**

| Byte | Value     | Included in CRC | Comment                                                 |
| ---- | --------- | --------------- | ------------------------------------------------------- |
| 0    | $         |                 | Starting char                                           |
| 1    | X         |                 | 'X' in place of v1 'M'                                  |
| 2    | ‘>'       |                 | Response message from device to backpack                |
| 3    | 0         | yes             | Flag. Set to zero                                       |
| 4    | 0x0309    | yes             | Message opcode                                          |
| 6    | 2         | yes             | Payload size                                            |
| 8    | \<voltage> | yes             | uint16 Battery voltage in mV. 12.35V is sent as 12350. |
| 10   | \<crc>     |                 | checksum                                                |

**Get firmware version:**

| Byte | Value  | Included in CRC | Comment                                 |
| ---- | ------ | --------------- | --------------------------------------- |
| 0    | $      |                 | Starting char                           |
| 1    | X      |                 | 'X' in place of v1 'M'                  |
| 2    | ‘<'    |                 | Request message from backpack to device |
| 3    | 0      | yes             | Flag. Set to zero                       |
| 4    | 0x030A | yes             | Message opcode                          |
| 6    | 0      | yes             | Payload size                            |
| 8    | <crc>  |                 | checksum                                |

**Get firmware version response:**

| Byte | Value                 | Included in CRC | Comment                                                                                                                                                                     |
| ---- | --------------------- | --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0    | $                     |                 | Starting char                                                                                                                                                               |
| 1    | X                     |                 | 'X' in place of v1 'M'                                                                                                                                                      |
| 2    | ‘>'                   |                 | Response message from device to backpack                                                                                                                                    |
| 3    | 0                     | yes             | Flag. Set to zero                                                                                                                                                           |
| 4    | 0x030A                | yes             | Message opcode                                                                                                                                                              |
| 6    | \<size>                | yes             | Payload size                                                                                                                                                                |
| 8    | \<num firmware fields> | yes             | uint8. The number of firmware bytes to follow, one for each major minor / fix value. For example, if firmware version is 1.2.3, the following bytes would be \[1, 2, 3\]. |
| 9+n  | \<fw field>            | yes             | uint8. Firmware field. One byte for each.                                                                                                                                   |
| 10+n | \<crc>                 |                 | checksum                                                                                                                                                                    |

**Set buzzer:**

| Byte | Value      | Included in CRC | Comment                                      |
| ---- | ---------- | --------------- | -------------------------------------------- |
| 0    | $          |                 | Starting char                                |
| 1    | X          |                 | 'X' in place of v1 'M'                       |
| 2    | ‘<'        |                 | Command message from backpack to device      |
| 3    | 0          | yes             | Flag. Set to zero                            |
| 4    | 0x030B     | yes             | Message opcode                               |
| 6    | 2          | yes             | Payload size                                 |
| 8    | \<duration> | yes             | uint16. Duration in ms. 0xFFFF=on, 0x0=off. |
| 10   | \<crc>      |                 | checksum                                     |

**Set OSD element:**

| Byte | Value              | Included in CRC | Comment                                                                                                |
| ---- | ------------------ | --------------- | ------------------------------------------------------------------------------------------------------ |
| 0    | $                  |                 | Starting char                                                                                          |
| 1    | X                  |                 | 'X' in place of v1 'M'                                                                                 |
| 2    | ‘>'                |                 | Response message from backpack to device                                                               |
| 3    | 0                  | yes             | Flag. Set to zero                                                                                      |
| 4    | 0x00B6             | yes             | Message opcode                                                                                         |
| 6    | <size>             | yes             | Payload size                                                                                           |
| 8    | \<sub cmd>          | yes             | uint8. The displayport command type in this message*: <br>0x00 = heartbeat <br>0x01 = release the port <br>0x02 = clear screen <br>0x03 = write string <br>0x04 = draw screen |
| 9    | \<row>              | yes             | uint8. Row number in the character array.                                                              |
| 10   | \<column>           | yes             | uint8. Column number in the character array.                                                           |
| 11   | \<attribute>        | yes             | uint8. A modifier code that changes the presentation of the text: <br>0x00 = DISPLAYPORT_ATTR_NONE <br>0x01 = DISPLAYPORT_ATTR_INFO <br>0x02 = DISPLAYPORT_ATTR_WARNING <br>0x03 = DISPLAYPORT_ATTR_CRITICAL <br>0x08 = DISPLAYPORT_ATTR_BLINK |
| 12+n | \<ASCII characters> | yes             | uint8. ASCII character codes. One byte per character. <br> For example “ELRS” = \[0x45, 0x4c, 0x52, 0x53\]. |
| 13+n | \<crc>              |                 | checksum                                                                                               |

*As of version 1.1 of this protocol, only sub cmd 0x03 is used for communication between ELRS and the VRX.
**As of version 1.1 of this protocol, only attribute 0x08 is used for communication between ELRS and the VRX.