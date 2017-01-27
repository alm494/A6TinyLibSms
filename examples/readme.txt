Compile results with the serial RX buffer length set to 160 bytes.
(edit SoftwareSerial.h or AltSoftSerial.cpp files to play around)

Using HardwareSerial:

AVR Memory Usage
----------------
Device: atmega328p
Program:    3120 bytes (9.5% Full)
(.text + .data + .bootloader)

Data:        346 bytes (16.9% Full)
(.data + .bss + .noinit)


Using SoftwareSerial with DEBUG set off:

AVR Memory Usage
----------------
Device: atmega328p
Program:    4524 bytes (13.8% Full)
(.text + .data + .bootloader)

Data:        365 bytes (17.8% Full)
(.data + .bss + .noinit)


Using AltSoftSerial with DEBUG set off:

AVR Memory Usage
----------------
Device: atmega328p
Program:    4504 bytes (13.7% Full)
(.text + .data + .bootloader)

Data:        446 bytes (21.8% Full)
(.data + .bss + .noinit)
