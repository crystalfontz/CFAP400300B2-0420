//=============================================================================
// "Arduino" example program for Crystalfontz ePaper.
//
// This project is for the CFAP400300B2-0420 which uses the SSD1683 controller:
//   https://www.crystalfontz.com/product/cfap400300b20420
//   https://www.crystalfontz.com/controllers/Solomon%20Systech/SSD1683/
//
// It was written against a Seeduino v4.2 @3.3v. An Arduino UNO modified to
// operate at 3.3v should also work.
//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>
//=============================================================================
// Connecting the Arduino to the display
//
// ARDUINO |CFA10098 |Wire Color |Function
// --------+---------+-----------+--------------------
// D3      |17       |Green      |Busy Line
// D4      |18       |Brown      |Reset Line
// D5      |15       |Purple     |Data/Command Line
// D10     |16       |Blue       |Chip Select Line
// D11     |14       |White      |MOSI
// D13     |13       |Orange     |Clock
// 3.3V    |5        |Red        |Power
// GND     |3        |Black      |Ground
//
//=============================================================================
// Creating image data arrays
//
// Bmp_to_epaper is code that will aid in creating bitmaps necessary from .bmp files.
// The code can be downloaded from the Crystalfontz website: https://www.Crystalfontz.com
// or it can be downloaded from github: https://github.com/crystalfontz/bmp_to_epaper
//=============================================================================

#include <SPI.h>
#include <SD.h>
#include <avr/io.h>

// Include the images prepared with "bmp_to_epaper"
#include "Images_for_CFAP400300B20420.h"

//=============================================================================
// Pin definitions and macros
//=============================================================================
#define EPD_READY 3
#define EPD_RESET 4
#define EPD_DC 5
#define EPD_CS 10
#define SD_CS 8

#define EPD_WIDTH 400
#define EPD_HEIGHT 300
#define EPD_NUM_PIX (EPD_WIDTH * EPD_HEIGHT / 8)

// Hardware control macros
#define ePaper_RST_0 (digitalWrite(EPD_RESET, LOW))
#define ePaper_RST_1 (digitalWrite(EPD_RESET, HIGH))
#define ePaper_CS_0 (digitalWrite(EPD_CS, LOW))
#define ePaper_CS_1 (digitalWrite(EPD_CS, HIGH))
#define ePaper_DC_0 (digitalWrite(EPD_DC, LOW))
#define ePaper_DC_1 (digitalWrite(EPD_DC, HIGH))

// Wait for display to be ready (BUSY line goes LOW when ready)
#define EPDReadBusy while (1 == digitalRead(EPD_READY))

//=============================================================================
// Low-level SPI communication functions
//=============================================================================

// Send a command byte to the display
void writeCMD(uint8_t command)
{
  ePaper_DC_0; // Command mode
  ePaper_CS_0; // Select chip
  SPI.transfer(command);
  ePaper_CS_1; // Deselect chip
}

// Send a data byte to the display
void writeData(uint8_t data)
{
  ePaper_DC_1; // Data mode
  ePaper_CS_0; // Select chip
  SPI.transfer(data);
  ePaper_CS_1; // Deselect chip
}

//=============================================================================
// Display update functions
//=============================================================================

// Full screen refresh update (slower, but clears ghosting)
void updateFull(void)
{
  writeCMD(0x22);  // Display Update Control
  writeData(0xF7); // Full refresh sequence
  writeCMD(0x20);  // Activate Display Update Sequence
  EPDReadBusy;     // Wait for update to complete
}

// Fast refresh update (faster but may accumulate ghosting over time)
void updateFast(void)
{
  writeCMD(0x22);  // Display Update Control
  writeData(0xC7); // Fast refresh sequence
  writeCMD(0x20);  // Activate Display Update Sequence
  EPDReadBusy;     // Wait for update to complete
}

// Partial refresh update (for small changes - has ghosting potential)
void updatePartial(void)
{
  writeCMD(0x22);  // Display Update Control
  writeData(0xFF); // Partial refresh sequence
  writeCMD(0x20);  // Activate Display Update Sequence
  EPDReadBusy;     // Wait for update to complete
}

//=============================================================================
// Display initialization functions
//=============================================================================

// Full screen refresh initialization
void initEPD(void)
{
  // Hardware reset sequence
  ePaper_RST_0;
  delay(10);
  ePaper_RST_1;
  delay(10);

  EPDReadBusy;
  writeCMD(0x12); // Software reset
  EPDReadBusy;

  // Driver output control
  writeCMD(0x01);
  writeData((EPD_HEIGHT - 1) % 256);
  writeData((EPD_HEIGHT - 1) / 256);
  writeData(0x00); // GD = 0; SM = 0; TB = 0;
                   // TB = 1 rotates 180*

  // Display update control
  writeCMD(0x21);
  writeData(0x40);
  writeData(0x00);

  // Border waveform control
  writeCMD(0x3C);
  writeData(0x05);

  // Data entry mode
  writeCMD(0x11);
  writeData(0x01); // Y decrement; X increment

  // Set RAM X address start/end positions
  writeCMD(0x44);
  writeData(0x00);
  writeData(EPD_WIDTH / 8 - 1);

  // Set RAM Y address start/end positions
  writeCMD(0x45);
  writeData((EPD_HEIGHT - 1) % 256);
  writeData((EPD_HEIGHT - 1) / 256);
  writeData(0x00);
  writeData(0x00);

  // Set RAM X address counter
  writeCMD(0x4E);
  writeData(0x00);

  // Set RAM Y address counter
  writeCMD(0x4F);
  writeData((EPD_HEIGHT - 1) % 256);
  writeData((EPD_HEIGHT - 1) / 256);

  EPDReadBusy;
}

// Fast refresh initialization
void initEPDFast(void)
{
  // Hardware reset sequence
  ePaper_RST_0;
  delay(10);
  ePaper_RST_1;
  delay(10);

  EPDReadBusy;
  writeCMD(0x12); // Software reset
  EPDReadBusy;

  // Driver output control - set display resolution
  // Not necessary if unchanged from full init
  writeCMD(0x01);
  writeData((EPD_HEIGHT - 1) % 256);
  writeData((EPD_HEIGHT - 1) / 256);
  writeData(0x00); // GD = 0; SM = 0; TB = 0;
                   // TB = 1 rotates 180*

  // Display update control
  writeCMD(0x21);
  writeData(0x40);
  writeData(0x00);

  // Border waveform control
  writeCMD(0x3C);
  writeData(0x05);

  // Temperature sensor control for fast refresh
  writeCMD(0x1A);
  writeData(0x6E);

  // Load temperature value
  writeCMD(0x22);
  writeData(0x91);
  writeCMD(0x20);
  EPDReadBusy;

  // Data entry mode
  writeCMD(0x11);
  writeData(0x01); // Y decrement; X increment

  // Set RAM X address start/end positions
  writeCMD(0x44);
  writeData(0x00);
  writeData(EPD_WIDTH / 8 - 1);

  // Set RAM Y address start/end positions
  writeCMD(0x45);
  writeData((EPD_HEIGHT - 1) % 256);
  writeData((EPD_HEIGHT - 1) / 256);
  writeData(0x00);
  writeData(0x00);

  // Set RAM X address counter
  writeCMD(0x4E);
  writeData(0x00);

  // Set RAM Y address counter
  writeCMD(0x4F);
  writeData((EPD_HEIGHT - 1) % 256);
  writeData((EPD_HEIGHT - 1) / 256);

  EPDReadBusy;
}

//=============================================================================
// Display data transfer functions
//=============================================================================
// Full screen refresh display from PROGMEM (RLE compressed)
void showImage(const unsigned char *image)
{
  uint8_t count;
  uint8_t data;
  EPDReadBusy;
  // Write to RAM buffer 0x24 (black/white image data)
  writeCMD(0x24);
  for (uint16_t i = 0; i < MONO_ARRAY_SIZE; i += 2)
  {
    count = pgm_read_byte(&image[i]);    // Read repeat count
    data = pgm_read_byte(&image[i + 1]); // Read data byte
    for (uint8_t j = 0; j < count; j++)
    {
      writeData(data); // Write data 'count' times
    }
  }

  // Write to RAM buffer 0x26 (prevents ghosting)
  writeCMD(0x26);
  for (uint16_t i = 0; i < MONO_ARRAY_SIZE; i += 2)
  {
    count = pgm_read_byte(&image[i]);
    data = pgm_read_byte(&image[i + 1]);
    for (uint8_t j = 0; j < count; j++)
    {
      writeData(data);
    }
  }
  updateFull();
}

// Fast refresh display from PROGMEM (RLE compressed)
void showImageFast(const unsigned char *image)
{
  uint8_t count;
  uint8_t data;
  EPDReadBusy;
  // Write to both RAM buffers
  writeCMD(0x24);
  for (uint16_t i = 0; i < MONO_ARRAY_SIZE; i += 2)
  {
    count = pgm_read_byte(&image[i]);     // pgm_read_byte reads from PROGMEM
    data = pgm_read_byte(&image[i + 1]);  // due to limited address space of AVR
    for (uint8_t j = 0; j < count; j++)
    {
      writeData(data);
    }
  }

  writeCMD(0x26);
  for (uint16_t i = 0; i < MONO_ARRAY_SIZE; i += 2)
  {
    count = pgm_read_byte(&image[i]);
    data = pgm_read_byte(&image[i + 1]);
    for (uint8_t j = 0; j < count; j++)
    {
      writeData(data);
    }
  }

  updateFast();
}

// Clear screen to white
void clearScreen(void)
{
  uint16_t i;
  EPDReadBusy;
  writeCMD(0x24);
  for (i = 0; i < EPD_NUM_PIX; i++)
  {
    writeData(0xFF);
  }

  writeCMD(0x26);
  for (i = 0; i < EPD_NUM_PIX; i++)
  {
    writeData(0xFF);
  }

  updateFull();
}

// Clear screen to black
void clearScreenBlack(void)
{
  uint16_t i;
  EPDReadBusy;
  writeCMD(0x24);
  for (i = 0; i < EPD_NUM_PIX; i++)
  {
    writeData(0x00);
  }

  writeCMD(0x26);
  for (i = 0; i < EPD_NUM_PIX; i++)
  {
    writeData(0x00);
  }

  updateFull();
}

void clearScreenPattern(uint8_t pattern)
{
  uint16_t i;
  EPDReadBusy;
  writeCMD(0x24);
  for (i = 0; i < EPD_NUM_PIX; i++)
  {
    writeData(pattern);
  }

  writeCMD(0x26);
  for (i = 0; i < EPD_NUM_PIX; i++)
  {
    writeData(pattern);
  }
  updateFull();
}

// Initialize partial refresh mode (call once before multiple partial updates)
void initPartialMode(void)
{
  // Hardware reset for partial refresh
  ePaper_RST_0;
  delay(10);
  ePaper_RST_1;
  delay(10);

  // Configure for partial refresh
  writeCMD(0x3C); // Border waveform
  writeData(0x80);

  writeCMD(0x21); // Display update control
  writeData(0x00);
  writeData(0x00);
}

// Write data to RAM for partial refresh (doesn't update display)
void writePartialImage(uint16_t x_start, uint16_t y_start,
                       const unsigned char *image,
                       uint16_t width, uint16_t height)
{
  uint16_t i;
  uint16_t x_end, y_end;

  // Convert pixel coordinates to byte addresses
  x_start = x_start / 8;
  x_end = x_start + width / 8 - 1;
  y_end = y_start + height - 1;

  EPDReadBusy;
  // Set RAM X address start/end for partial area
  writeCMD(0x44);
  writeData(x_start);
  writeData(x_end);

  // Set RAM Y address start/end for partial area
  writeCMD(0x45);
  writeData(y_start % 256);
  writeData(y_start / 256);
  writeData(y_end % 256);
  writeData(y_end / 256);

  // Set RAM X address counter
  writeCMD(0x4E);
  writeData(x_start);

  // Set RAM Y address counter
  writeCMD(0x4F);
  writeData(y_start % 256);
  writeData(y_start / 256);

  // Write image data to partial area in RAM
  writeCMD(0x24);
  for (i = 0; i < height * width / 8; i++)
  {
    writeData(pgm_read_byte(&image[i]));
  }
}

// Convenience function for single partial update
void showPartialImage(uint16_t x_start, uint16_t y_start,
                      const unsigned char *image,
                      uint16_t width, uint16_t height)
{
  initPartialMode();
  writePartialImage(x_start, y_start, image, width, height);
  updatePartial();
}

// Deep sleep mode (power saving)
void enterDeepSleep(void)
{
  writeCMD(0x10); // Deep sleep mode
  writeData(0x01);
  delay(100);
}

//=============================================================================
// Arduino setup and main loop. If unfamiliar with Arduino, setup() runs once
// at startup, and loop() is your super loop that runs repeatedly.
//=============================================================================

void setup(void)
{
  // Initialize serial for debug output
  Serial.begin(115200);
  Serial.println("CFAP400300B2-0420 Demo");

  // Configure pin directions
  pinMode(EPD_CS, OUTPUT);
  pinMode(EPD_RESET, OUTPUT);
  pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_READY, INPUT);
  pinMode(SD_CS, OUTPUT);

  // Initialize SD card
  digitalWrite(SD_CS, LOW);
  if (!SD.begin(SD_CS))
  {
    Serial.println("SD card initialization failed");
  }
  else
  {
    Serial.println("SD card initialized");
  }

  // Initialize SPI interface
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.begin();

  Serial.println("Initialization complete");
}

//=============================================================================
// Demo selection
//=============================================================================
#define DEMO_FULL_REFRESH 1    // Full screen refresh demo
#define DEMO_FAST_REFRESH 1    // Fast refresh demo
#define DEMO_PARTIAL_LETTERS 1 // Partial refresh with CFAP letters
#define DEMO_CLEAR_WHITE 1     // Clear to white
#define DEMO_CLEAR_BLACK 1     // Clear to black

void loop()
{
  Serial.println("Starting demo...");

#if DEMO_FULL_REFRESH
  // Full screen refresh demonstration
  // Provides best image quality, clears ghosting
  Serial.println("Full screen refresh demo");

  initEPD();
  //showImage(Mono_1BPP);
  clearScreenPattern(0x0F); // Create a pattern on the display
  enterDeepSleep();
  delay(2000);
#endif

#if DEMO_FAST_REFRESH
  // Fast refresh demonstration
  // Faster update but may cause ghosting over time
  // Periodic full refreshes are recommended to clear ghosting
  Serial.println("Fast refresh demo");

  initEPDFast();
  showImageFast(Mono_1BPP);
  enterDeepSleep();
  delay(2000);

#endif

#if DEMO_PARTIAL_LETTERS
  // Partial refresh demonstration with CFAP letters
  // Note: This display does not officially support partial refresh
  // Ghosting may occur - use full refresh periodically to clear
  Serial.println("Partial refresh demo - CFAP letters");

  // Initialize and set white background as base
  initEPD();
  clearScreen();
  delay(1000);

  // Display letters one at a time using partial refresh
  // Partials have to be sent twice if multiple partial updates are done
  // Letter size is defined in Images_for_CFAP400300B20420.h
  uint16_t x_pos = 336;
  uint16_t y_pos = 60;
  uint16_t x_spacing = 60;

  // Show 'C'
  Serial.println("Showing C");
  showPartialImage(x_pos, y_pos,
                   Mono_Letter_C,
                   LETTER_WIDTH_PIXELS,
                   LETTER_HEIGHT_PIXELS);
  showPartialImage(x_pos, y_pos,
                   Mono_Letter_C,
                   LETTER_WIDTH_PIXELS,
                   LETTER_HEIGHT_PIXELS);

  // Show 'F'
  Serial.println("Showing F");
  showPartialImage(x_pos - x_spacing, y_pos,
                   Mono_Letter_F,
                   LETTER_WIDTH_PIXELS,
                   LETTER_HEIGHT_PIXELS);
  showPartialImage(x_pos - x_spacing, y_pos,
                   Mono_Letter_F,
                   LETTER_WIDTH_PIXELS,
                   LETTER_HEIGHT_PIXELS);

  // Show 'A'
  Serial.println("Showing A");
  showPartialImage(x_pos - x_spacing * 2, y_pos,
                   Mono_Letter_A,
                   LETTER_WIDTH_PIXELS,
                   LETTER_HEIGHT_PIXELS);
  showPartialImage(x_pos - x_spacing * 2, y_pos,
                   Mono_Letter_A,
                   LETTER_WIDTH_PIXELS,
                   LETTER_HEIGHT_PIXELS);

  // Show 'P'
  Serial.println("Showing P");
  showPartialImage(x_pos - x_spacing * 3, y_pos,
                   Mono_Letter_P,
                   LETTER_WIDTH_PIXELS,
                   LETTER_HEIGHT_PIXELS);
  showPartialImage(x_pos - x_spacing * 3, y_pos,
                   Mono_Letter_P,
                   LETTER_WIDTH_PIXELS,
                   LETTER_HEIGHT_PIXELS);
  delay(500);

  // Demonstrate doing multiple partial updates at once
  Serial.println("Showing C F A P in different locations");
  initPartialMode();
  writePartialImage(200, 160,
                    Mono_Letter_C,
                    LETTER_WIDTH_PIXELS,
                    LETTER_HEIGHT_PIXELS);
  writePartialImage(140, 260,
                    Mono_Letter_F,
                    LETTER_WIDTH_PIXELS,
                    LETTER_HEIGHT_PIXELS);
  writePartialImage(60, 160,
                    Mono_Letter_A,
                    LETTER_WIDTH_PIXELS,
                    LETTER_HEIGHT_PIXELS);
  writePartialImage(260, 260,
                    Mono_Letter_P,
                    LETTER_WIDTH_PIXELS,
                    LETTER_HEIGHT_PIXELS);
  updatePartial();
  delay(1000);
  enterDeepSleep();
  delay(2000);
#endif

#if DEMO_CLEAR_WHITE
  // Clear screen to white
  Serial.println("Clearing to white");
  initEPD();
  clearScreen();
  enterDeepSleep();
  delay(2000);
#endif

#if DEMO_CLEAR_BLACK
  // Clear screen to black
  Serial.println("Clearing to black");
  initEPD();
  clearScreenBlack();
  enterDeepSleep();
  delay(2000);
#endif

  // End on a clear screen to prevent ghosting during storage
  Serial.println("Demo complete - clearing screen");
  initEPD();
  clearScreen();
  enterDeepSleep();
  // Stop here - loop runs once
  while (1)
    ;
}
//=============================================================================
