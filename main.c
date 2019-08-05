/*05 08 2019
Работа с портом Д.*/
#include "1886ve4d.h"
#include "int17xxx.h"
#pragma origin 0x8

typedef unsigned char uc;

const uc Translate_num_to_LED[10] = {
//  0,	  1,	2,	  3,	4,	  5,	6,	  7,	8,	  9.
	0x3F, 0x06, 0x5B, 0x4F, 0x67, 0x6D, 0x7D, 0x70, 0x7F, 0x6F};
	
interrupt iServer(void)
{
	multi_interrupt_entry_and_save

	PERIPHERAL_service:
		PEIF = 0;
		interrupt_exit_and_restore
	TMR0_service:
		// save on demand: PRODL,PRODH,TBLPTRH,TBLPTRL,FSR0,FSR1
		/* process Timer 0 interrupt */
		// T0IF is automatically cleared when the CPU vectors to 0x10
		// restore on demand: PRODL,PRODH,TBLPTRH,TBLPTRL,FSR0,FSR1
		T0IF = 0;
		interrupt_exit_and_restore
	T0CKI_service:
		T0CKIF = 0;
		interrupt_exit_and_restore
	INT_service:
		INTF = 0;
		interrupt_exit_and_restore
}
/******************/

#include "math24.h"

void main(void)
{
	uc d_line = 0, d_bonus = 0;
    uc LED[5] = {0, 0, 0, 0, 0};
    uc temp = 0;
	while (1)
	{
		temp = d_bonus | (0x03 << d_line);
		PORTD = temp;
		d_line ++;
		if (d_line > 4)
			d_line = 0;
		
		temp = LED[(int)d_line];
		temp = Translate_num_to_LED[(int)temp];
		PORTC = temp;
	}
}
