/*
 * Part3.c
 *
 *  Created on: Feb 16, 2023
 *      Author: Matthew Zmuda
 *
 *      YOU NEED TO FILL IN THIS AUTHOR BLOCK
 */
#include <msp430.h>

#define BLINK_PERIOD_DEFAULT 24576 // Blink period for default speed (4 Hz)
#define BLINK_PERIOD_MIN 16384 // Minimum blink period (6.4 Hz)
#define BLINK_PERIOD_MAX 65535 // Maximum blink period (1 Hz)

volatile unsigned int button0_press_time = 0;
volatile unsigned int blink_period = BLINK_PERIOD_DEFAULT;
volatile unsigned char speed_changed = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    P1DIR |= BIT0; // P1.0 (red LED) as output
    P1OUT &= ~BIT0; // Initially turn off the LED

    P2DIR &= ~BIT3; // P2.3 (button 0) as input
    P2REN |= BIT3; // Enable pull-up resistor
    P2IES |= BIT3; // Interrupt on high-to-low transition
    P2IFG &= ~BIT3; // Clear interrupt flag
    P2IE |= BIT3; // Enable interrupt

    P4DIR &= ~BIT1; // P4.1 (button 1) as input
    P4REN |= BIT1; // Enable pull-up resistor

    TB0CCR0 = BLINK_PERIOD_DEFAULT; // Set the blink period to default value
    TB0CTL = TBSSEL_1 | MC_1 | TBCLR; // ACLK, Up mode, clear TBR

    __bis_SR_register(LPM3_bits | GIE); // Enter LPM3 with interrupts enabled

    while(1)
    {
        if(speed_changed)
        {
            blink_period = BLINK_PERIOD_DEFAULT; // Reset blink period to default value
            speed_changed = 0;
        }
    }
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if(P2IN & BIT3) // Button released
    {
        unsigned int button0_hold_time = TB0R - button0_press_time;
        if(button0_hold_time < 32768) // Limit the maximum hold time to 1 second
        {
            blink_period = BLINK_PERIOD_MAX - (BLINK_PERIOD_MAX - BLINK_PERIOD_MIN) * button0_hold_time / 32768;
        }
    }
    else // Button pressed
    {
        button0_press_time = TB0R;
    }
    P2IFG &= ~BIT3; // Clear interrupt flag
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer0_B0(void)
{
    P1OUT ^= BIT0; // Toggle the LED
    TB0CCR0 = blink_period; // Update the blink period
}

#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    if((P4IN & BIT1) == 0) // Button 1 pressed
    {
        speed_changed = 1;
    }
    P4IFG &= ~BIT1; // Clear interrupt flag
}






