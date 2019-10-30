/*
Master

*/

//define the clock frequency of the micro controller for the delay to work correctly, 16Mhz
//==================================SERIAL TX START
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#include <avr/interrupt.h>
#include <string.h>


//no semicolon needed at the end of the following lines
#define BUAD 9600
#define BRC ((F_CPU/16/BUAD)-1)

#define TX_BUFFER_SIZE 128
char serialBuffer[TX_BUFFER_SIZE];

//create a dynamic queue method
uint8_t serialReadPos = 0; //unsigned 8 bit integer
uint8_t serialWritePos = 0;

void appendSerial(char c);
void serialWrite(char c[]);

//for numbers
char str[40];
//==================================SERIAL TX END===========

//==================================SERIAL RX START=========
#define RX_BUFFER_SIZE  128
char rxBuffer[RX_BUFFER_SIZE];

uint8_t rxReadPos = 0;
uint8_t rxWritePos = 0;

char getChar(void);
char peekChar(void);
//==================================SERIAL RX END===========

//==================================TWI Functions===========
void TWI_Init(){
	TWBR = 0x00; //Clear status register
	TWBR = 0x0C; //Set bit rate
}

void TWI_Start(){
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
	while((TWCR&(1 << TWINT))==0);
	while((TWSR & (0xF8))!= 0x08);
}

void TWI_Write_Addr(unsigned char Addr){
	TWDR = Addr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while((TWCR& (1 << TWINT))==0);
	while((TWSR&(0xF8)) != 0x18);
}

void TWI_Write_Data(uint8_t Data){
	TWDR = Data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while((TWCR&(1<<TWINT)) == 0);
	while((TWSR&(0xF8)) != 0x28);
}

void TWI_Stop(){
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTO);
}


int main(void)
{
	//==================================SERIAL TX START
	UBRR0H = (BRC >> 8); //Put BRC to UBRR0H and move it right 8 bits.
	UBRR0L = BRC;
	UCSR0B |= (1 << TXEN0) | (1 << TXCIE0);
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); //8 BIT data frame
	//==================================SERIAL TX END

	//==================================SERIAL RX START
	UCSR0B |= (1 << RXEN0) | (1 << RXCIE0);
	//==================================SERIAL RX END
	//ENABLE interrupts
	sei();
	
	TWI_Init();
	
	while(1)
	{
		TWI_Start();
		//_delay_ms(1000);
		TWI_Write_Addr(0x20);
		//_delay_ms(1000);
		TWI_Write_Data(2);
		//_delay_ms(1000);
		TWI_Stop();
		_delay_ms(500);
	}//while(1)
}//int main()


//==============================================================SERIALS==========================================
//==================================SERIAL TX START
void appendSerial(char c){
	serialBuffer[serialWritePos] =c;
	serialWritePos++;
	
	if (serialWritePos >= TX_BUFFER_SIZE){
		serialWritePos = 0;
	}
}

void serialWrite(char c[]){
	for (uint8_t i=0; i< strlen(c); i++){
		appendSerial(c[i]);
	}
	if (UCSR0A & (1 << UDRE0)){
		UDR0 = 0; //send an off character
	}
}

//set up the interrupt vector
//This interrupt gets triggered when the transmitting (sending data) is done
ISR(USART_TX_vect){
	if(serialReadPos != serialWritePos){
		UDR0 = serialBuffer[serialReadPos];
		serialReadPos++;
		
		if(serialReadPos >= TX_BUFFER_SIZE){
			serialReadPos=0;
		}
	}
}
//==================================SERIAL TX END


char peekChar(void)
{
	char ret = '\0';
	
	if(rxReadPos != rxWritePos)
	{
		ret = rxBuffer[rxReadPos];
	}
	
	return ret;
}

char getChar(void)
{
	char ret = '\0';
	
	if(rxReadPos != rxWritePos)
	{
		ret = rxBuffer[rxReadPos];
		
		rxReadPos++;
		
		if(rxReadPos >= RX_BUFFER_SIZE)
		{
			rxReadPos = 0;
		}
	}
	
	return ret;
}

ISR(USART_RX_vect)
{
	rxBuffer[rxWritePos] = UDR0;
	
	rxWritePos++;
	
	if(rxWritePos >= RX_BUFFER_SIZE)
	{
		rxWritePos = 0;
	}
}
