 #include <msp430.h>
 #include <macros.h>

volatile unsigned int i = 0;
volatile unsigned int j = 0;
volatile unsigned int r = 0;
volatile unsigned int g = 0; // setup of needed variables
volatile unsigned int b = 0;
volatile unsigned int byteCnt = 0;
volatile unsigned int byteNum = 0;
int message[80];
int rx;

void initClock(void)
{
    TA0CCR0 = 256 - 1; // period for PWM
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // uses SMCLK, sets up mode, clears timer
}

void initPWM(void)
{
    P1DIR |= BIT2 + BIT3 + BIT4; // pins 1.2, 1.3, and 1.4 are set as output
    P1SEL |= BIT2 + BIT3 + BIT4; // sets same pins to get timer A0 CCRx signal
    TA0CCTL1 = OUTMOD_7; // CCR1 output mode reset/set
    TA0CCR1 = r; // CCR1 set to red led PWM
    TA0CCTL2 = OUTMOD_7; // CCR2 output mode reset/set
    TA0CCR2 = g; // CCR2 set to green led PWM
    TA0CCTL3 = OUTMOD_7; // CCR3 output mode reset/set
    TA0CCR3 = b; // CCR3 set to blue led PWM
}

void initUART(void)
{
    P3SEL |= BIT3 + BIT4; // sets pin 3.3 and 3.4 for USCI_A0 TXD/RXD
    UCA0CTL1 |= UCSWRST; // resets USCI logic to hold in reset state
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 6;                              // 1MHz 9600 (see User's Guide)
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Modln UCBRSx=0, UCBRFx=0,
    UCA0CTL1 &= ~(UCSWRST); // starts USCI state machine
    UCA0IE |= UCRXIE; // enables RX interrupt on USCI_A0

}
/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	initClock(); // initializes clock
	initPWM(); // initializes pwm
	initUART(); // initializes UART
	
	__bis_SR_register(LPM0_bits + GIE); // enters LPM0 and enables global interrupt
}
 // USCI ISR
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0(void)
{
    switch (__even_in_range(UCA0IV, 4))
    {
    case 0:
        break; // vector 0, no interrupts
    case 2: // RXIFG
        while(!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = UCA0RXBUF; //displays character sent
        rx = UCA0RXBUF; // holds recieved charater
        if(byteNum == 0) // checks first number of the message to see total bytes in package
        {
            byteNum = rx;
            byteCnt++;
            message[i] = byteNum - 3;
            i++;
        }

        //assinging RGB values to CC registers
       else if(byteCnt >= 1 && byteCnt <= 3)
        {
            switch(byteCnt)
            {
            case 1:
                r = rx; // set red to the second int recieved
                TA0CCR1 = r; // pwm duty cycle is red
                byteCnt++; // increment byte count
                break;
            case 2:
                g = rx; // sets green to third int recieved
                TA0CCR2 = g; // pwm duty cycle is green
                byteCnt++; // increment byte count
                break;
            case 3:
                b = rx; // sets blue to fourth int recieved
                TA0CCR3 = b; // pwm duty cycle is blue
                byteCnt++;
                break;
            }
        }

        //write other values
        else if (byteCnt < byteNum)
        {
            //add messages to send
            message[i] = rx;
            i++;
            byteCnt++;
            //if message over, transmit to next in line
            if (byteCnt == byteNum)
            {
                while(message[j] != 0x0D)
                {
                    while(!(UCA0IFG & UCTXIFG));

                    UCA0TXBUF = message[j];
                    j++;
                }
                while(!(UCA0IFG & UCTXIFG));
                UCA0TXBUF = 0x0D; // add to end of message
            }
        }
        break;
    case 4:
        break;
    default:
        break;

    }
}
