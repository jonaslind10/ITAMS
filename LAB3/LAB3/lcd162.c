/*-------------------------------------------------------------------
  File name: "lcd162.c"

  Driver for "LCD Keypad Shield" alphanumeric display.
  Display controller = HD44780U (LCD-II).
  
  Max. uC clock frequency = 16 MHz (Tclk = 62,5 ns)

  Connection : PORTx (4 bit mode) :
  [LCD]        [Portx]
  RS   ------  PH 5
  RW   ------  GND
  E    ------  PH 6
  DB4  ------  PG 5
  DB5  ------  PE 3
  DB6  ------  PH 3
  DB7  ------  PH 4

  Henning Hargaard, February 5, 2018
---------------------------------------------------------------------*/
#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>
// Enabling us to use macro _NOP() to insert the NOP instruction
#include <avr/cpufunc.h>
#include "lcd162.h"

// library function itoa() is needed
#include <stdlib.h>

//*********************** PRIVATE (static) operations *********************
static void waitBusy()
{
  // To be implemented
  _delay_ms(2);
}  

static void pulse_E()
{
  // To be implemented 
  PORTH |= 0b01000000;
  _NOP();
  _NOP();
  _NOP();
  _NOP();
  PORTH &= 0b10111111;
}

// Sets the display data pins according to the 4 lower bits of data
static void set4DataPins(unsigned char data)
{
  PORTH = (PORTH & 0b11100111) | ((data<<1) & 0b00011000);
  PORTE = (PORTE & 0b11110111) | ((data<<2) & 0b00001000);
  PORTG = (PORTG & 0b11011111) | ((data<<5) & 0b00100000);  
}

static void sendInstruction(unsigned char data)
{      
  // To be implemented
  waitBusy();
  PORTH &= 0b11011111;	// RS = 0
  _NOP();
  	
  // Send MSB
  set4DataPins(data>>4);
  pulse_E();
  _NOP();
  
  // Send LSB
  set4DataPins(data);
  pulse_E();
  _NOP();
}

static void sendData(unsigned char data)
{      
  // To be implemented
  waitBusy();
  PORTH |= 0b00100000;	// RS = 1
  _NOP();
   
  // Send MSB
  set4DataPins(data>>4);
  pulse_E();
  _NOP();
   
  // Send LSB
  set4DataPins(data);
  pulse_E();
  _NOP();
  _delay_us(50);
}

//*********************** PUBLIC functions *****************************

// Initializes the display, blanks it and sets "current display position"
// at the upper line, leftmost character (cursor invisible)
// Reference: Page 46 in the HD44780 data sheet
void LCDInit()
{
  // Initializing the used port
  DDRH |= 0b01111000;  // Outputs
  DDRE |= 0b00001000;
  DDRG |= 0b00100000;
  DDRB |= 0b00010000;  // LCD Backlight
    
  // Setup PWM
  TCCR2A = 0b10000011;
  TCCR2B = 0b00000001;
  
  PORTB &= 0b11101111;
   
  // Wait 50 ms (min. 15 ms demanded according to the data sheet)
  _delay_ms(50);
  // Function set (still 8 bit interface)
  PORTG |= 0b00100000;
  PORTE |= 0b00001000;
  pulse_E();
  
  // Wait 10 ms (min. 4,1 ms demanded according to the data sheet)
  _delay_ms(10);
  // Function set (still 8 bit interface)
  pulse_E();

  // Wait 10 ms (min. 100 us demanded according to the data sheet)
  _delay_ms(10);
  // Function set (still 8 bit interface)
  pulse_E();

  // Wait 10 ms (min. 100 us demanded according to the data sheet)
  _delay_ms(10);
  // Function set (now selecting 4 bit interface !)
  PORTG &= 0b11011111;
  pulse_E();

  // Function Set : 4 bit interface, 2 line display, 5x8 dots
  sendInstruction( 0b00101000 );
  // Display, cursor and blinking OFF
  sendInstruction( 0b00001000 );
  // Clear display and set DDRAM adr = 0	
  sendInstruction( 0b00000001 );
  // By display writes : Increment cursor / no shift
  sendInstruction( 0b00000110 );
  // Display ON, cursor and blinking OFF
  sendInstruction( 0b00001100 );
  // Set Backlight
  setBacklight(60);
}

// Blanks the display and sets "current display position" to
// the upper line, leftmost character
void LCDClear()
{
	// Clear display and set DDRAM adr = 0
	sendInstruction(0b00000001);
}

// Sets DDRAM address to character position x and line number y
void LCDGotoXY( unsigned char x, unsigned char y )
{
  uint8_t addr = 0b00100000;
  
  if (y == 0)
	addr = 0x80;
  else if (y == 1)
	addr = 0xC0;
  if (x < 16)
	addr += x;
	
  sendInstruction(addr);
}

// Display "ch" at "current display position"
void LCDDispChar(char ch)
{
	sendData(ch);
}

// Displays the string "str" starting at "current display position"
void LCDDispString(char* str)
{
	// To be implemented
	if (str == NULL)
		return;
  
	while(*str != '\0')
	sendData(*str++);
}

// Displays the value of integer "i" at "current display position"
void LCDDispInteger(int i)
{
	// Convert integer to string with itoa()
	char c[7];
	itoa(i, c, 10);
	LCDDispString(c);
}

// Loads one of the 8 user definable characters (UDC) with a dot-pattern,
// pre-defined in an 8 byte array in FLASH memory
void LCDLoadUDC(unsigned char UDCNo, const unsigned char *UDCTab)
{
  // To be implemented	
  int i;
  for (i = 0; i < 8; i++)
  {
	  sendInstruction((0b01000000 | UDCNo << 3) | i);
	  sendData(*UDCTab++);
  }    	
}

// Selects, if the cursor has to be visible, and if the character at
// the cursor position has to blink.
// "cursor" not 0 => visible cursor.
// "blink" not 0 => the character at the cursor position blinks.
void LCDOnOffControl(unsigned char cursor, unsigned char blink)
{
	sendInstruction(0b00001100 | (cursor<<1) | blink);
}


// Moves the cursor to the left
void LCDCursorLeft()
{
	sendInstruction(0b00010000);
}

// Moves the cursor to the right
void LCDCursorRight()
{
	 sendInstruction(0b00010100);
}

// Moves the display text one position to the left
void LCDShiftLeft()
{
 	sendInstruction(0b00011000);
}

// Moves the display text one position to the right
void LCDShiftRight()
{
	sendInstruction(0b00011100);
}

// Sets the backlight intensity to "percent" (0-100)
void setBacklight(unsigned char percent)
{
  OCR2A = ((256*percent)/100)-1;
}

// Reads the status for the 5 on board keys
// Returns 0, if no key pressed
unsigned char readKeys()
{
  // To be implemented
}