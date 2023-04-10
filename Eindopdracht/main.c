#define F_CPU 8e6

#include "display.h"
#include "lcd.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <time.h>

//Initiations
void init_board();
void init_matrix();
void init_lcd();

//Game
void play_round();
void set_guess(int num);

//Helper
int input_conversion(int);
int random_number_generator();

//Matrix
void matrix_arrow(bool);
void matrix_check();
void turn_on_matrix_led(int x, int y);
void clear_matrix();

//Game variables
bool is_ready_for_input = false;
int guess = 0;
int guess_counter = 0;
int random_number = -1;
int timer = 0;
int completion_timer = 0;

//Interrupts for input + helper
void set_guess(int num) {
	if (is_ready_for_input) guess = num;
}

ISR(INT1_vect) {
	set_guess(1);
}
ISR(INT2_vect) {
	set_guess(2);
}
ISR(INT3_vect) {
	set_guess(3);
}
ISR(INT4_vect) {
	set_guess(4);
}
ISR(INT5_vect) {
	set_guess(5);
}
ISR(INT6_vect) {
	set_guess(6);
}
ISR(INT7_vect) {
	set_guess(7);
}
ISR(INT8_vect) {
	set_guess(8);
}


//Timer
ISR( TIMER1_COMPA_vect )
{
	timer = timer + 1;
}

//Main function - initiations and followed by main game loop
int main(void)
{
	//Initiations
	init_board();
	init_matrix();
	init_lcd();
	
	while (1) {
		//Start a round
		play_round();
		
		//Clear
		_delay_ms(10);
		lcd_clear();
		clear_matrix();
		
		//Clear debugging LED
		PORTB = 0x00;
		_delay_ms(10);
		
		//Display guess information
		char guess_text[16];
		sprintf(guess_text, "Good! %d %s!", 
			guess_counter, 
			guess_counter == 1 ? "try" : "tries");
		display_text(guess_text);
		
		//Display guess information for 1 sec
		_delay_ms(1000);
		lcd_clear();
		
		int seconds = completion_timer / 10;
		int milli = completion_timer % 10;
		
		//Display timer information
		char timer_text[16];
		sprintf(timer_text, "In %d.%d seconds!",
		seconds,
		milli);
		display_text(timer_text);
		
		//Display timer information for 1 sec
		_delay_ms(1000);
		
		//Reset variables
		guess = 0;
		guess_counter = 0;
	}
}

//Round loop - read the players guess, check if it's correct, if not show the player to guess lower or higher. Repeat.
void play_round() {
	//Generate a random number
	random_number = random_number_generator();
	timer = 0;
	
	//Display random_number on LEDs for debugging
	PORTB = 1 << (random_number - 1);

	//Keep asking for inputs until the random_number is guessed
	while (1)
	{
		//Clear
		lcd_clear();
		clear_matrix();
		display();
		_delay_ms(10);
		guess = 0;
		
		//Display the guess counter on LCD
		guess_counter++;
		char text[16];
		sprintf(text, "Enter guess %d!", guess_counter);
		display_text(text);
		_delay_ms(10);

		//Keep polling for inputs until one is entered
		is_ready_for_input = true;
		while (1)
		{			
			//Don't overload the CPU
			_delay_ms(100);
			
			//Stop once input is detected
			if (guess != 0) {
				is_ready_for_input = false;
				break;
			}
		}
		
		//Check if the guessed number was correct
		if (guess == random_number)
		{
			completion_timer = timer;
			
			matrix_check();
			_delay_ms(1000);
			return;
		} else if (guess > random_number)
		{
			//Guess was to high - display down-arrow on matrix
			matrix_arrow(false);
		} else {
			//Guess was to low - display up-arrow on matrix
			matrix_arrow(true);
		}
		_delay_ms(1000);
	}
}

//Initiations
void init_board() {
	DDRA = 0x00;
	PORTA = 0xFF;
	
	//Interrupts
	EICRA = 0b11111111;
	EICRB = 0b11111111;
	EIMSK = 0b11111111;
	
	//Timer
	OCR1A = 31250 / 100;
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001100;
	TIMSK |= (1<<4);
	
	sei();
	
	//LED for debugging
	DDRB = 0xFF;
}

void init_matrix() {
	displayInit();
	clear_matrix();
	display();
}

void init_lcd() {
	DDRC = 0xFF;
	PORTC = 0x00;
	
	init();
	_delay_ms(100);
	lcd_clear();
	_delay_ms(100);
	display_text("Welcome!");
	_delay_ms(1000);
}

//Generate a random number for the player to guess
int random_number_generator() {
	//srand(time(NULL));
	int rand_num = rand();
	return (rand_num % 8) + 1;
}

//Matrix functions
void matrix_arrow(bool is_up) {
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			//Use of virtual_y for flipping the arrow
			int virtual_y = is_up ? 7 - y : y;
			
			//Middle columns
			if (x == 3 || x == 4)
			{
				displaySetPixel(x, virtual_y);
			}
			
			//Middle rows
			else if ((y == 3 || y == 4) && (x < 2 || x > 5))
			{
				displaySetPixel(x, virtual_y);
			}
			
			//Outer middle columns
			else if ((x == 2 || x == 5) && (y > 3 && y != 7))
			{
				displaySetPixel(x, virtual_y);
			}
			
			//Extra dots
			else if ((x == 1 || x == 6) && y == 5)
			{
				displaySetPixel(x, virtual_y);
			}
		}
	}
	
	display();
}

void matrix_check() {
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			//x == 0
			if (x == 0 && (y > 3 && y < 6))
			{
				turn_on_matrix_led(x, y);
			}
			
			//x == 1 || x == 4
			else if ((x == 1 || x == 4) && (y > 3 && y != 7))
			{
				turn_on_matrix_led(x, y);
			}
			
			//x == 2 || x == 3
			else if ((x == 2 || x == 3) && y > 4)
			{
				turn_on_matrix_led(x, y);
			}
			
			//x == 5
			else if (x == 5 && (y > 1 && y < 6))
			{
				turn_on_matrix_led(x, y);
			}
			
			//x == 6
			else if (x == 6 && y < 5)
			{
				turn_on_matrix_led(x, y);
			}
			
			//x == 7
			else if (x == 7 && y < 3)
			{
				turn_on_matrix_led(x, y);
			}
		}
	}
	
	display();
}

void turn_on_matrix_led(int x, int y) {
	displaySetPixel(x, y);
}

void clear_matrix() {
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			displayClrPixel(x, y);
		}
	}
	
	display();
}