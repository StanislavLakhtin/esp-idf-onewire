# UART OneWire Example

This example demonstrates how to utilize UART interfaces of ESP32 to use as OneWire bus. It uses a few DS18B20 sensors to scan a bus and write CALC command.

## How to use example

### Hardware Required

The example can be run on any ESP32 development board connected to a PC with a single USB cable for flashing and
monitoring. The external interface should have 3.3V outputs. You may use e.g. 3.3V compatible USB-to-Serial dongle.

### Setup the Hardware

#### Prepare schema 3v3 and 5v negotiations

Connect the external serial interface to the ESP32 board as follows.

  | ESP32 Interface | #define | ESP32 Pin 
  | --- | --- | --- | --- |
  | Transmit Data (TxD) | ECHO_TEST_TXD | GPIO16 
  | Receive Data (RxD) | ECHO_TEST_RXD | GPIO17 
  | Ground  | n/a | GND | GND |


Optionally, you can set-up and use a VDD that has +3.3V

### Configure the project

```
idf.py menuconfig
```
or
```
idf.py menuconfig
```

* Set serial port under Serial Flasher Options.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```
or
```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

## Example Output

#todo

Type some characters in the terminal connected to the external serial interface. As result you should see echo in the
terminal which is used for flashing and monitoring. You can verify if the echo indeed comes from ESP32 by
disconnecting either `TxD` or `RxD` pin: no characters will appear when typing.

## Troubleshooting

You are not supposed to see the echo in the terminal which is used for flashing and monitoring, but in the other one
which is connected to UART1.
