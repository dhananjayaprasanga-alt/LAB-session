#include <arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// =================================================
// FLAGS
// =================================================

volatile uint8_t emergencyFlag = 0;
volatile uint8_t pedestrianFlag = 0;
volatile uint8_t maintenanceModeFlag = 0;

// =================================================
// LED FUNCTIONS
// =================================================

void allOFF()
{
    PORTB &= ~(1 << PB0);
    PORTB &= ~(1 << PB1);
    PORTB &= ~(1 << PB2);
    PORTB &= ~(1 << PB3);
    PORTB &= ~(1 << PB4);
}

void roadGreenMode()
{
    // Road Green ON
    PORTB |= (1 << PB2);

    // Road Red OFF
    PORTB &= ~(1 << PB0);

    // Yellow OFF
    PORTB &= ~(1 << PB1);

    // Pedestrian Red ON
    PORTB |= (1 << PB3);

    // Pedestrian Green OFF
    PORTB &= ~(1 << PB4);
}

void roadRedMode()
{
    // Road Red ON
    PORTB |= (1 << PB0);

    // Road Green OFF
    PORTB &= ~(1 << PB2);

    // Yellow OFF
    PORTB &= ~(1 << PB1);

    // Pedestrian Green ON
    PORTB |= (1 << PB4);

    // Pedestrian Red OFF
    PORTB &= ~(1 << PB3);
}

// =================================================
// EMERGENCY MODE
// =================================================

void emergencyMode()
{
    allOFF();

    // Road Green ON
    PORTB |= (1 << PB2);

    // Pedestrian Red ON
    PORTB |= (1 << PB3);

    for (int i = 0; i < 100; i++)
    {
        _delay_ms(100);
    }

    emergencyFlag = 0;
}

// =================================================
// PEDESTRIAN MODE
// =================================================

void pedestrianMode()
{
    // Yellow ON
    PORTB |= (1 << PB1);

    for (int i = 0; i < 50; i++)
    {
        _delay_ms(100);
    }

    // Road Red ON
    PORTB |= (1 << PB0);

    // Road Green OFF
    PORTB &= ~(1 << PB2);

    // Yellow OFF
    PORTB &= ~(1 << PB1);

    // Pedestrian Green ON
    PORTB |= (1 << PB4);

    // Pedestrian Red OFF
    PORTB &= ~(1 << PB3);

    for (int i = 0; i < 100; i++)
    {
        _delay_ms(100);
    }

    pedestrianFlag = 0;
}

// =================================================
// MAINTENANCE MODE
// =================================================

void maintenanceMode()
{
    while (maintenanceModeFlag)
    {
        // All OFF
        allOFF();

        // Yellow ON
        PORTB |= (1 << PB1);

        _delay_ms(500);

        // Yellow OFF
        PORTB &= ~(1 << PB1);

        _delay_ms(500);
    }
}

// =================================================
// INTERRUPTS
// =================================================

// INT0 -> Emergency
ISR(INT0_vect)
{
    emergencyFlag = 1;
}

// INT1 -> Pedestrian
ISR(INT1_vect)
{
    pedestrianFlag = 1;
}

// PCINT -> Maintenance
ISR(PCINT2_vect)
{
    // Check button press
    if (!(PIND & (1 << PD4)))
    {
        _delay_ms(50);

        if (!(PIND & (1 << PD4)))
        {
            // Toggle maintenance mode
            maintenanceModeFlag ^= 1;

            // Wait until release
            while (!(PIND & (1 << PD4)));
        }
    }
}

// =================================================
// MAIN
// =================================================

int main(void)
{
    // =================================================
    // OUTPUT PINS
    // D8-D12
    // =================================================

    DDRB |= (1 << PB0);
    DDRB |= (1 << PB1);
    DDRB |= (1 << PB2);
    DDRB |= (1 << PB3);
    DDRB |= (1 << PB4);

    // =================================================
    // INPUT PINS
    // D2 D3 D4
    // =================================================

    DDRD &= ~(1 << PD2);
    DDRD &= ~(1 << PD3);
    DDRD &= ~(1 << PD4);

    // Enable Pullups
    PORTD |= (1 << PD2);
    PORTD |= (1 << PD3);
    PORTD |= (1 << PD4);

    // =================================================
    // INT0 FALLING EDGE
    // =================================================

    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);

    // =================================================
    // INT1 FALLING EDGE
    // =================================================

    EICRA |= (1 << ISC11);
    EICRA &= ~(1 << ISC10);

    // Enable INT0 and INT1
    EIMSK |= (1 << INT0);
    EIMSK |= (1 << INT1);

    // =================================================
    // PIN CHANGE INTERRUPT
    // =================================================

    PCICR |= (1 << PCIE2);

    // PD4 = PCINT20
    PCMSK2 |= (1 << PCINT20);

    // =================================================
    // GLOBAL INTERRUPTS ENABLE
    // =================================================

    sei();

    // =================================================
    // MAIN LOOP
    // =================================================

    while (1)
    {
        // Maintenance Highest in loop
        if (maintenanceModeFlag)
        {
            maintenanceMode();
        }

        // Emergency
        else if (emergencyFlag)
        {
            emergencyMode();
        }

        // Pedestrian
        else if (pedestrianFlag)
        {
            pedestrianMode();
        }

        // NORMAL LOOP
        else
        {
            // GREEN MODE
            roadGreenMode();

            for (int i = 0; i < 50; i++)
            {
                _delay_ms(100);

                if (maintenanceModeFlag || emergencyFlag || pedestrianFlag)
                    break;
            }

            // RED MODE
            roadRedMode();

            for (int i = 0; i < 50; i++)
            {
                _delay_ms(100);

                if (maintenanceModeFlag || emergencyFlag || pedestrianFlag)
                    break;
            }
        }
    }
}