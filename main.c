#include "fsl_device_registers.h"
#include "stdbool.h"

bool isOpen = true;
bool buttonPressed = false;
bool emergencyStop = false;

void PORTE_IRQHandler(void){
	NVIC_ClearPendingIRQ(PORTE_IRQn);
	emergencyStop = !emergencyStop;
	PORTE_ISFR = (1 << 24); /* Clear ISFR for PORTE, Pin 24*/
}

void PORTA_IRQHandler(void) {
	NVIC_ClearPendingIRQ(PORTA_IRQn); /* Clear pending interrupts */
	buttonPressed = true;
	PORTA_ISFR = (1 << 1); /* Clear ISFR for PORTA, Pin 1*/
	PORTA_ISFR = (1 << 2); //Clear ISFR for PORTA, Pin 2
} //End of Interrupt

int main(void)
{
	// Enable Port A, B, C, D, E, and FTM3 for clock gating.
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

	// Configure PA[2:1], PB[9], PD[5:0], PC[4], PE[24] for GPIO
	PORTA_GPCLR = 0x00060100;
	PORTB_GPCLR = 0x02000100;
	PORTC_GPCLR = 0x00100100;
	PORTD_GPCLR = 0x003F0100;
	PORTE_GPCHR = 0x01000100;

	// Set PA[2:1] to trigger interrupts
	PORTA_PCR1 = 0x90100;
	PORTA_PCR2 = 0x90100;
	PORTE_PCR24 = 0xB0100;

	//Clear PORTA ISFR
	PORTA_ISFR = (1 << 1);
	PORTA_ISFR = (1 << 2);
	PORTE_ISFR = (1 << 24);

	// Set PA[2:1] for input; 0
	GPIOA_PDDR = 0x00000000;

	// Set PC[4] for output; 1
	GPIOC_PDDR = 0x00000010;

	// Set PD[5:0] for output; 1
	GPIOD_PDDR = 0x0000003F;

	// Set PE[24] for input; 0
	GPIOE_PDDR = 0x00000000;

	NVIC_EnableIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTE_IRQn);

	int i;
	int longDelay = 50000;

	while(1){
		if(buttonPressed){
			int count = 0;

			if(isOpen){
				GPIOC_PDOR = 0x10; //Turn on ultrasonic sensor
				while(count != 14){ //Backwards Direction
					GPIOD_PDOR = 0x36;
					for (i = 0; i < longDelay; i++);
					GPIOD_PDOR = 0x3A;
					for (i = 0; i < longDelay; i++);
					GPIOD_PDOR = 0x39;
					for (i = 0; i < longDelay; i++);
					GPIOD_PDOR = 0x35;
					for (i = 0; i < longDelay; i++);
					count += 1;
				}
				isOpen = false;
				GPIOC_PDOR = 0x00; // Turn off ultrasonic sensor
			}
			else{
				while(count != 14){ //Forward Direction
					GPIOD_PDOR = 0x36;
					for (i = 0; i < longDelay; i++);
					GPIOD_PDOR = 0x35;
					for (i = 0; i < longDelay; i++);
					GPIOD_PDOR = 0x39;
					for (i = 0; i < longDelay; i++);
					GPIOD_PDOR = 0x3A;
					for (i = 0; i < longDelay; i++);
					count += 1;
				}
				isOpen = true;
			}

			GPIOD_PDOR = 0x00; // Turn off the stepper motor
			buttonPressed = false;
		}
	}
	return 0;
}
