#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define GAS_THRESHOLD 500

//------------------------------------------------
// UART Functions
//------------------------------------------------
void UART_Init(unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0B = (1 << TXEN0); // Enable TX

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
}

void UART_Transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = data;
}

void UART_SendString(char *str)
{
    while (*str)
    {
        UART_Transmit(*str++);
    }
}

//------------------------------------------------
// ADC Initialization
//------------------------------------------------
void ADC_Init(void)
{
    ADMUX = (1 << REFS0);

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);
}

//------------------------------------------------
// ADC Read Function
//------------------------------------------------
uint16_t ADC_Read(uint8_t channel)
{
    channel &= 0x07;

    ADMUX = (ADMUX & 0xF8) | channel;

    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));

    return ADC;
}

//------------------------------------------------
// Main Program
//------------------------------------------------
int main(void)
{
    uint16_t ldr_value;
    uint16_t pot_value;
    uint16_t gas_value;

    char buffer[20];

    DDRB |= (1 << PB0) | (1 << PB1);

    ADC_Init();

    // 9600 baud @ 16MHz
    UART_Init(103);

    while (1)
    {
        ldr_value = ADC_Read(0);
        pot_value = ADC_Read(1);
        gas_value = ADC_Read(2);

        //-------------------------------
        // Adaptive Lighting
        //-------------------------------
        if (ldr_value < pot_value)
            PORTB |= (1 << PB0);
        else
            PORTB &= ~(1 << PB0);

        //-------------------------------
        // Gas Alert
        //-------------------------------
        if (gas_value > GAS_THRESHOLD)
            PORTB |= (1 << PB1);
        else
            PORTB &= ~(1 << PB1);

        //-------------------------------
        // Display Gas Value
        //-------------------------------
        UART_SendString("Gas Value = ");

        itoa(gas_value, buffer, 10);
        UART_SendString(buffer);

        UART_SendString("\r\n");

        _delay_ms(500);
    }
}
