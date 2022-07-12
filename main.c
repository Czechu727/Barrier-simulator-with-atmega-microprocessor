/*
 * Projekt koncowy.c
 *
 * Created: 31/05/2021 22:53:12
 * Author : szymo
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define LCD_DATA_DDR DDRA
#define LCD_DATA_PORT PORTA
#define LCD_CTRL_DDR DDRD
#define LCD_CTRL_PORT PORTD

#define PIN_RS (1<<PD1)
#define SET_RS LCD_CTRL_PORT |= PIN_RS
#define CLR_RS LCD_CTRL_PORT &= ~(PIN_RS)

#define PIN_EN (1<<PD0)
#define SET_EN LCD_CTRL_PORT |= PIN_EN
#define CLR_EN LCD_CTRL_PORT &= ~(PIN_EN)

#define B1 (1<<PB1)
#define B2 (1<<PB2)
#define EN12 (1<<PB3)
#define stop PORTB &= ~B1; PORTB &= ~B2
#define right PORTB &= ~B2; PORTB |= B1

#define FNC_SET 0x38                        //ustawienie: tryb 8bit, 2linie, znaki 5x8
#define DISP_OFF 0x08                        //wyl. wyswietlacza
#define DISP_ON 0x0C                        //wl. wyswietlacza
#define DISP_CLR 0x01                        //wyczyszczenie zawartosci pamieci DDRAM (wyczyszczenie wyswietlacza)
#define ENTRY_MODE 0x06                        //tryb wpisywania (inkrementacja pozycji kursora, brak przemieszczenia ekranu)

#define MODE (1<<PD4)
#define MODE_DDR DDRD
#define MODE_PORT PORTD

#define DISP_LEFT 0x1C
#define DISP_RIGHT 0x18
#define DISP_HOME 0x02

void send_command(uint8_t command);            //wysylanie komendy
void send_data(uint8_t data);                //wysylanie danych (znakow) do pamieci
void lcd_init(void);                        //inicjalizacja wyswietlacza (dokumentajca str. 45)
void lcd_goto(uint8_t row, uint8_t col);    //ustawienie pozycji kursora na wyswietlaczu
void lcd_write(char *string);                //wyslanie napisu do wyswietlacza

uint8_t sprawdz_przycisk (uint8_t button);
uint8_t x=1;
uint8_t y=1;

int main(void)
{
	LCD_DATA_DDR |= 0xFF;
	LCD_DATA_PORT &= ~(0x00);
	
	LCD_CTRL_DDR |= (PIN_RS | PIN_EN);
	CLR_RS;
	CLR_EN;
	
	DDRA = 0xFF;
	PORTA = 0x00;
	DDRB |=B1| B2| EN12;
	PORTB |= EN12;
	
	
	PORTD |= MODE;
	DDRD &= ~(MODE);
	
	
	uint8_t stan;
	
	TCCR1A |= (1<<COM1A1);
	TCCR1A |= (1<<WGM11);
	TCCR1B |= (1<<WGM13)|(1<<WGM12);
	ICR1 = 1249;
	OCR1A = 62;
	DDRD = (1<<PD5);
	TCCR1B |= (1<<CS12);
	
	uint8_t drc = 1;
	DDRB= 0xFF;
	DDRC=0xFF;
	sei();

	while (1)
	{
		
		if (sprawdz_przycisk(MODE))
		{
			stan = 1;
		}
		else
		stan = 0;
		
		if (stan == 0)
		{
			right;
			DISP_CLR;
			PORTC = _BV(PC4);
			_delay_ms(100);
			lcd_init();
			lcd_goto(1,2);
			lcd_write("Szerokiej drogi!");
		}
		
		if (stan == 1)
		{
			stop;
			DISP_CLR;
			PORTC = _BV(PC5);
			_delay_ms(100);
			lcd_goto(1,2);
			lcd_write("Prosze pobrac");
			lcd_goto(2,2);
			lcd_write("bilet");
		}
	}
}

void send_command(uint8_t command)
{
	CLR_RS;
	CLR_EN;
	LCD_DATA_PORT = command;
	SET_EN;
	_delay_us(2);
	CLR_EN;
	_delay_us(100);
}

void send_data(uint8_t data)
{
	SET_RS;
	CLR_EN;
	LCD_DATA_PORT = data;
	SET_EN;
	_delay_us(2);
	CLR_EN;
	_delay_us(100);
}

void lcd_init()
{
	_delay_ms(15);
	send_command(0x30);
	_delay_ms(4.1);
	send_command(0x30);
	_delay_us(100);
	send_command(0x30);
	
	send_command(FNC_SET);
	send_command(DISP_OFF);
	send_command(DISP_CLR);
	send_command(ENTRY_MODE);
	send_command(DISP_ON);
}

void lcd_goto(uint8_t row, uint8_t col)  //ustawienie kursora
{
	if (row<1 || row >2) return;
	if (col<1 || col>16) return;
	
	col--;
	if (row == 2) col+= 0x40;
	send_command(col | 0x80);
}

void lcd_write(char *string)
{
	while(*string) send_data(*string++);
}


uint8_t sprawdz_przycisk (uint8_t button)
{
	if (!(PIND&button))
	{
		_delay_ms(25);
		if (!(PIND&button)) return 1;
	}
	return 0;
}