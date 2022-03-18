# color-inverter

## Purpose

The color inverter is a small box. It reads a color from the bottom color sensor
and display the color on top via some multicolor LEDs.

## Realization

We need a microcontroller board, a rgb sensor, some programmable rgb leds. Preferrably
we choose a board with connectivity.

## Bill of Materials (BOM)

* Arduino Nano 33 IoT
* TCS34725 rgb color sensor
* WS2812B led rgb stripe or any other format
* battery, e.g. 4xAA or 1 LiIon battery

## Remarks

The color inverter is not calibrated in any sense, therefore we don't see always the expected
result. Linearity of sensor and output is unclear, there's no gamma correction, only a very
basic white balance correction (offset, no function) and the output could be calibrated against
the input.
