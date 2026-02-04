# CFAP400300B2-0420

Sample code for the CFAP400300B2-0420, a 4.2" 400x300 ePaper display with the SSD1683 controller.

Product page: <https://www.crystalfontz.com/product/cfap400300B20420>

## Hardware

This code was written for a Seeeduino v4.2 @ 3.3V. An Arduino UNO modified to run at 3.3V will also work.

### Display Wiring

| Arduino | CFA10098 | Function           |
|---------|----------|--------------------|
| GND     | 3        | Ground             |
| D3      | 17       | Busy Line          |
| D4      | 18       | Reset Line         |
| D5      | 15       | Data/Command Line  |
| D10     | 16       | Chip Select Line   |
| D11     | 14       | MOSI               |
| D13     | 13       | Clock              |
| 3.3V    | 5        | Power              |


## Features

- Full screen refresh (best quality, clears ghosting)
- Fast refresh (faster updates, may accumulate ghosting)
- Partial refresh (for small updates)
- Deep sleep mode for power saving

## Creating Images

Use the [bmp_to_epaper](https://github.com/crystalfontz/bmp_to_epaper) tool to convert BMP files to image arrays.

## License

This is free and unencumbered software released into the public domain. See the [Unlicense](http://unlicense.org/) for details.
