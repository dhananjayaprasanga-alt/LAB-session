#include <Arduino.h>

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define BUTTON1 PD2
#define BUTTON2 PD3
#define BUTTON3 PD4
#define BUTTON4 PD5

#define GREEN_LED PB0
#define RED_LED   PB1

int main(void)
{
    // Password sequence
    uint8_t password[4] = {1, 2, 3, 4};

    // Store user input
    uint8_t entered[4];

    uint8_t index = 0;
    uint8_t button = 0;
    uint8_t i;
    uint8_t correct;

    // LED pins as output
    DDRB |= (1 << GREEN_LED) | (1 << RED_LED);

    // Button pins as input
    DDRD &= ~((1 << BUTTON1) |
              (1 << BUTTON2) |
              (1 << BUTTON3) |
              (1 << BUTTON4));

    // Enable internal pull-up resistors
    PORTD |= (1 << BUTTON1) |
             (1 << BUTTON2) |
             (1 << BUTTON3) |
             (1 << BUTTON4);

    while (1)
    {
        button = 0;

        // Detect button press
        if (!(PIND & (1 << BUTTON1)))
        {
            button = 1;
        }

        else if (!(PIND & (1 << BUTTON2)))
        {
            button = 2;
        }

        else if (!(PIND & (1 << BUTTON3)))
        {
            button = 3;
        }

        else if (!(PIND & (1 << BUTTON4)))
        {
            button = 4;
        }

        // If any button pressed
        if (button != 0)
        {
            entered[index] = button;
            index++;

            _delay_ms(300); // Debounce delay

            // Wait until button released
            while (!(PIND & (1 << BUTTON1)) ||
                   !(PIND & (1 << BUTTON2)) ||
                   !(PIND & (1 << BUTTON3)) ||
                   !(PIND & (1 << BUTTON4)));
        }

        // Check password after 4 inputs
        if (index == 4)
        {
            correct = 1;

            for (i = 0; i < 4; i++)
            {
                if (entered[i] != password[i])
                {
                    correct = 0;
                    break;
                }
            }

            // Correct password
            if (correct)
            {
                PORTB |= (1 << GREEN_LED);
                _delay_ms(2000);
                PORTB &= ~(1 << GREEN_LED);
            }

            // Wrong password
            else
            {
                PORTB |= (1 << RED_LED);
                _delay_ms(2000);
                PORTB &= ~(1 << RED_LED);
            }

            // Reset input index
            index = 0;
        }
    }
}