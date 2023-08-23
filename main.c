#include <msp430.h>
#include "stdint.h"

#define LED1 BIT0
#define LED2 BIT7

#define LED1_DIR P1DIR
#define LED2_DIR P4DIR
#define LED1_OUT P1OUT
#define LED2_OUT P4OUT

#define ACLK_DELAY_1S 32767

uint8_t FLAGS = 0x0000;
uint16_t btndbc;

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    LED1_DIR |= LED1;         // P1.0 output
    LED2_DIR |= LED2;         // P4.7 output

    P1DIR &= ~BIT1; // Button P1.1
    P1REN |= BIT1;  // Pull-up enabled
    P1OUT |= BIT1;  // Set P1.1 high

    TA0CCTL0 = CCIE; // CCR0 interrupt enabled
    TA0CCR0 = 50000;
    TA0CTL = TASSEL__SMCLK + MC__UP + TACLR; // SMCLK, upmode, clear TAR

    P1IE |= BIT1;

    while (1)
    {
        if (FLAGS & BIT0)
        {
            // P1OUT ^= BIT0; // Toggle P1.0
            LED2_OUT ^= LED2; // Toggle P4.7
            FLAGS &= ~BIT0;
        }

        if (FLAGS & BIT1)
        {
            LED1_OUT ^= LED1; // Toggle P1.0
            FLAGS &= ~BIT1;
        }
        // Debouncing with shift register
        // See https://www.best-microcontroller-projects.com/easy_switch_debounce.html#Shift_Register_debounce
        if (FLAGS & BIT2)
        {
            btndbc = (btndbc << 1) | !(P1IN & BIT1) | 0xe000;
            FLAGS &= ~BIT2;
        }

        if (btndbc == 0xf000)
        {
            TA1CTL = MC__STOP; // stop debouncing timer
            P1IE |= BIT1;      // enable PORTA interrupt back

            LED1_OUT ^= LED1;
        }

        // start timer for polling debouncing button
        if (FLAGS & BIT3)
        {
            TA1CTL = TASSEL__ACLK + MC__UP + TACLR + TAIE; // ACLK, upmode to CCR0, clear TAR  enable interrupt
            TA1CCR0 = ACLK_DELAY_1S / 1000 * 20;           // Debouncing time approx 20ms

            TA1CCTL1 = CCIE; // CCR0 interrupt enabled

            FLAGS &= ~BIT3;
        }
        __bis_SR_register(LPM0_bits + GIE); // Enter LPM0, enable interrupts
        __no_operation();                   // For debugger
    }
}

// Timer0 A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER0_A0_VECTOR))) TIMER0_A0_ISR(void)
#else
#error Compiler not supported!
#endif
{
    FLAGS |= BIT0;

    __bic_SR_register_on_exit(LPM0_bits);
}

// Timer1_A3 Interrupt Vector (TAIV) handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER1_A1_VECTOR))) TIMER1_A1_ISR(void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(TA1IV, 14))
    {
    case 0:
        break; // No interrupt
    case TA1IV_TACCR1:
        FLAGS |= BIT2; // CCR1
        break;
    case 4:
        // CCR2
        break;
    case 6:
        break; // reserved
    case 8:
        break; // reserved
    case 10:
        break; // reserved
    case 12:
        break; // reserved
    case 14:
        // overflow
        break;
    default:
        break;
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(PORT1_VECTOR))) PORT1_ISR(void)
#else
#error Compiler not supported!
#endif
{
    if (P1IV & P1IV_P1IFG1)
    {
        FLAGS |= BIT3;
        P1IE &= ~BIT1; // disable interrupt for PORT A
        P1IV &= ~P1IV_P1IFG1;
    }
    __bic_SR_register_on_exit(LPM0_bits);
}