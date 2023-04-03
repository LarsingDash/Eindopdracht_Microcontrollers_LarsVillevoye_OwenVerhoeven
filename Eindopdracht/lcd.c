#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

int cursor = 0;

void writeInitCommand_strobe_e(unsigned char byte)
{
	byte &= 0xF0;
	PORTC = byte | (1<<LCD_E);
	_delay_ms(1);
	PORTC &= ~(1<<LCD_E);
	_delay_ms(1);
}

void write2NibbleCommand_strobe_e(unsigned char byte)
{
	unsigned char firstNibble = byte & 0xF0;
	PORTC = firstNibble | (1<<LCD_E);
	_delay_ms(1);
	PORTC &= ~(1<<LCD_E);
	_delay_ms(1);
	
	unsigned char secondNibble = (byte << 4) & 0xF0;
	PORTC = secondNibble | (1<<LCD_E);
	_delay_ms(1);
	PORTC &= ~(1<<LCD_E);
	_delay_ms(1);
}

void write2NibbleData_Strobe_E(unsigned char byte)
{
	unsigned char firstNibble = byte & 0xF0;
	firstNibble |= (1<<LCD_RS); 
	PORTC = firstNibble | (1<<LCD_E);
	_delay_ms(1);
	PORTC &= ~(1<<LCD_E);
	_delay_ms(1);

	unsigned char secondNibble = (byte << 4) & 0xF0;
	secondNibble &= 0xF0;
	secondNibble |= (1<<LCD_RS);
	PORTC = secondNibble | (1<<LCD_E);
	_delay_ms(1);
	PORTC &= ~(1<<LCD_E);
	_delay_ms(1);
}

void init(void)
{
	writeInitCommand_strobe_e(0x20);

	write2NibbleCommand_strobe_e(0x28);

	//0x0C to turn off cursor and blink
	write2NibbleCommand_strobe_e(0x0C);

	write2NibbleCommand_strobe_e(0x06);
}

void display_text(char *str)
{
	while(*str) {
		cursor++;
		
		write2NibbleData_Strobe_E(*str++);
	}
}

void set_cursor(int position) {
	int difference = position - (cursor + 1);
	
	if (difference > 0)
	{
		for (int i = 0; i < difference; i++)
		{
			write2NibbleCommand_strobe_e(0b00010100);
		}
	} else {
		for (int i = 0; i > difference; i--)
		{
			write2NibbleCommand_strobe_e(0b00010000);
		}
	}
}

void lcd_clear()
{
	write2NibbleCommand_strobe_e (0x01);
}