#include <msp430.h>

// This project does simple LED blink using timer T0 routine
// Tested platform: MSP430F5529 Launchpad
// https://www.ti.com/tool/MSP-EXP430F5529LP/

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    P1DIR |= 0x01;            // P1.0 output
    TA0CCTL0 = CCIE;          // CCR0 interrupt enabled
    TA0CCR0 = 50000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK, upmode, clear TAR

    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0, enable interrupts
    __no_operation();                   // For debugger
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
    P1OUT ^= 0x01; // Toggle P1.0
}
