# Milestone 1: Communicating with Will Byers
For this milestone project, the MSP430F5529 was the selected board. Simply connect the 
TX and RX of a UART cable to pints 3.3 and 3.4 respectively and send data via USB using
RealTerm. A single package with pwm vales for 1 board is 5 bits, where the first bit is
the package length, the second bit is the red pwm value, the third bit is the green pwm
value, the fourth bit is the blue pwm value, and the fifth bit is the exit bit 0x0D. Any
message after doesnt need a package size bit due to the code generating one automatically,
so a double package would be 8 bits with the first message being the same as a single 
package, and the second message just with the pwm values, and exit bit at the end.