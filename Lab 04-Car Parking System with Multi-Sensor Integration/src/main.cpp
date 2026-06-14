#define F_CPU 16000000UL
#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>

#define INITIAL_CAPACITY 10

#define TRIG    PD2
#define ECHO    PD3
#define GREEN   PD4
#define YELLOW  PD5
#define RED     PD6
#define BUTTON  PD7

uint8_t vehicles_entered = 0;
uint8_t available_slots  = INITIAL_CAPACITY;

void uart_init(void)
{
    UBRR0H = 0;
    UBRR0L = 103;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_char(char c)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_string(const char *s)
{
    while (*s)
        uart_char(*s++);
}

void uart_number(uint8_t n)
{
    if (n >= 10)
        uart_char('0' + n / 10);
    uart_char('0' + n % 10);
}

void print_status(void)
{
    uart_string("Vehicles: ");
    uart_number(vehicles_entered);
    uart_string("  |  Available: ");
    uart_number(available_slots);
    uart_string("\r\n");
}

uint16_t get_distance_cm(void)
{
    PORTD &= ~(1 << TRIG);
    _delay_us(2);
    PORTD |=  (1 << TRIG);
    _delay_us(10);
    PORTD &= ~(1 << TRIG);

    while (!(PIND & (1 << ECHO)));

    uint16_t count = 0;
    while (PIND & (1 << ECHO))
    {
        _delay_us(1);
        count++;
    }

    return count / 58;
}

void update_leds(void)
{
    PORTD &= ~((1 << GREEN) | (1 << YELLOW) | (1 << RED));

    if (available_slots == 0)
        PORTD |= (1 << RED);
    else if (available_slots <= INITIAL_CAPACITY / 2)
        PORTD |= (1 << YELLOW);
    else
        PORTD |= (1 << GREEN);
}

int main(void)
{
    DDRD |= (1 << TRIG) | (1 << GREEN) | (1 << YELLOW) | (1 << RED);
    DDRD &= ~((1 << ECHO) | (1 << BUTTON));
    PORTD |= (1 << BUTTON);

    uart_init();
    uart_string("Parking System Ready\r\n");

    uint8_t prev_state = 0;
    update_leds();

    while (1)
    {
        uint16_t dist = get_distance_cm();
        uint8_t curr_state = (dist <= 20) ? 1 : 0;

        if (curr_state == 1 && prev_state == 0)
        {
            if (vehicles_entered < INITIAL_CAPACITY)
            {
                vehicles_entered++;
                available_slots = INITIAL_CAPACITY - vehicles_entered;
                update_leds();
                print_status();
            }
            else
            {
                uart_string("Lot FULL\r\n");
            }
            _delay_ms(50);
        }

        prev_state = curr_state;

        if (!(PIND & (1 << BUTTON)))
        {
            _delay_ms(20);
            if (!(PIND & (1 << BUTTON)))
            {
                vehicles_entered = 0;
                available_slots  = INITIAL_CAPACITY;
                prev_state = 0;
                update_leds();
                uart_string("System Reset\r\n");
                print_status();
                while (!(PIND & (1 << BUTTON)));
            }
        }

        _delay_ms(60);
    }
}