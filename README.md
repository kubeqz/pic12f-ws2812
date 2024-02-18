# PIC12F1840 + WS2812

## Intro
This project was initially based on http://www.friendlywire.com/tutorials/ws2812/#resources

## The goal
I just wanted to create an RGB lamp powered from USB, using 8-LED WS2812 module.

## Issues were met
The main difference compared to ready-to-use solution above is that I only had PIC12F1840 in my drawer so had to modify the original code quite significantly.

The main problem was making the WS2812 work with PIC12F that is running "only" @32MHz clock (that in fact means 8MHz instruction cycle time). It turned out that with such frequency, it's quite difficult to match narrow WS2812 timing. Any decision points in code was increasing that time, so the only solution was to generate single bits with use of HW - in this case I used SPI (with only SDO used).

It should be quite straightforward trying to port this to other PIC microcontrollers, but I would not expect to work with parts that can't achieve at least 32MHz oscillator speed.
