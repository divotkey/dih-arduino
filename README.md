# Device Interaction Hub (DIH)

The Device Interaction Hub (DIH) is a framework that connects various devices, such as Arduinos, to a central application on a PC or Raspberry Pi. It provides a structured basis for data exchange via the serial interface and supports the simultaneous connection of multiple devices. DIH includes a protocol for data exchange, but is designed so that other protocols, such as MQTT, can also be integrated.

## DIH Arduino Library
This Arduino library implements the serial communication structure of DIH. It enables Arduinos to connect to the Device Interaction Hub and provides all the necessary functions to reliably exchange data between the Arduino and the central application. The library supports several Arduinos in parallel and facilitates integration into the DIH framework.