#define F_CPU 16000000UL  // Define CPU Frequency as 16MHz

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>  // For itoa() function

#define LCD_Data_Dir DDRD  // Define LCD data port direction
#define LCD_Data_Port PORTD  // Define LCD data port
#define LCD_Command_Dir DDRB  // Define LCD command port direction register
#define LCD_Command_Port PORTB  // Define LCD command port
#define RS PB0  // Define Register Select (data/command reg.)pin
#define RW PB1  // Define Read/Write signal pin
#define EN PB2  // Define Enable signal pin

void LCD_Command(unsigned char cmnd) {
	LCD_Data_Port = cmnd;
	LCD_Command_Port &= ~(1<<RS);  // RS=0 command reg.
	LCD_Command_Port &= ~(1<<RW);  // RW=0 Write operation
	LCD_Command_Port |= (1<<EN);  // Enable pulse
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(3);
}

void LCD_Char(unsigned char char_data) {
	LCD_Data_Port = char_data;
	LCD_Command_Port |= (1<<RS);  // RS=1 Data reg.
	LCD_Command_Port &= ~(1<<RW);  // RW=0 write operation
	LCD_Command_Port |= (1<<EN);  // Enable Pulse
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(1);
}

void LCD_Init(void) {
	LCD_Command_Dir = 0xFF;  // Make LCD command port direction as output
	LCD_Data_Dir = 0xFF;  // Make LCD data port direction as output
	_delay_ms(20);  // LCD Power ON delay always >15ms

	LCD_Command(0x38);  // Initialization of 16X2 LCD in 8-bit mode
	LCD_Command(0x0C);  // Display ON Cursor OFF
	LCD_Command(0x06);  // Auto Increment cursor
	LCD_Command(0x01);  // Clear display
	_delay_ms(2);
	LCD_Command(0x80);  // Cursor at home position
}

void LCD_Clear() {
	LCD_Command(0x01);  // Clear display
	_delay_ms(2);
}

void LCD_String(char *str) {
	int i = 0;
	while (str[i] != 0) {  // Send each char of string till the NULL
		LCD_Char(str[i]);
		i++;
	}
}

void ADC_Init() {
	// AREF = AVcc
	ADMUX = (1 << REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1 << ADEN) | (7 << ADPS0);
}

uint16_t ReadADC(uint8_t ch) {
	// Select ADC Channel ch must be 0-7
	ch = ch & 0b00000111;
	ADMUX = (ADMUX & 0xF8) | ch;
	
	// Start Single Conversion
	ADCSRA |= (1 << ADSC);
	
	// Wait for conversion to complete
	while (!(ADCSRA & (1 << ADIF)));
	
	// Clear ADIF by writing one to it
	ADCSRA |= (1 << ADIF);
	
	return ADC;
}

void Relay_Init() {
	// Set PORTC0 as output for relay
	DDRC |= (1 << PC0);
	PORTC &= ~(1 << PC0);  // Initially turn off the relay
}

int main(void) {
	ADC_Init();
	Relay_Init();
	LCD_Init();  // Initialize LCD

	uint16_t moisture_level;
	char buffer[10];
	
	while (1) {
		moisture_level = ReadADC(0);  // Read the soil moisture sensor value
		
		// Convert moisture level to string and display on LCD
		itoa(moisture_level, buffer, 10);
		LCD_Clear();
		LCD_Command(0x80);  // Set cursor to first line
		LCD_String("Moisture:");
		LCD_Command(0xC0);  // Set cursor to second line
		LCD_String(buffer);
		
		// Check moisture level and control relay
		if (moisture_level < 512) {  // Threshold value for soil moisture
			PORTC |= (1 << PC0);  // Turn on the relay
			} else {
			PORTC &= ~(1 << PC0);  // Turn off the relay
		}
		
		_delay_ms(1000);  // Delay to reduce sensor read frequency
	}
}
