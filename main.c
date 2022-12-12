#include <msp430.h>
#include "stdint.h"

#define LED1 BIT0
#define LED2 BIT7

#define LED1_DIR P1DIR
#define LED2_DIR P4DIR
#define LED1_OUT P1OUT
#define LED2_OUT P4OUT

uint8_t FLAGS = 0x0000;

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    LED1_DIR |= LED1;         // P1.0 output
    LED2_DIR |= LED2;         // P4.7 output

    TA0CCTL0 = CCIE; // CCR0 interrupt enabled
    TA0CCR0 = 50000;
    TA0CTL = TASSEL__SMCLK + MC__UP + TACLR; // SMCLK, upmode, clear TAR

    TA1CTL = TASSEL__ACLK + MC__UP + TACLR + TAIE; // ACLK, contmode, clear TAR  enable interrupt
    TA1CCR0 = 32768;
    TA1CCTL1 = CCIE;

    while (1)
    {
        if (FLAGS & BIT0)
        {
            // P1OUT ^= BIT0; // Toggle P1.0
            LED2_OUT ^= LED2; // Toggle P1.0
            FLAGS &= ~BIT0;
        }

        if (FLAGS & BIT1)
        {
            LED1_OUT ^= LED1; // Toggle P1.0
            FLAGS &= ~BIT1;
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
    case 2:
        FLAGS |= BIT1; // overflow
        break;         // CCR1 not used
    case 4:
        break; // CCR2 not used
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
