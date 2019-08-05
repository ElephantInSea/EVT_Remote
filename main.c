/*05 08 2019
Работа с портом Д.*/
#include "1886ve4d.h"
#include "int17xxx.h"
#pragma origin 0x8

typedef unsigned char uc;

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
    
	while (1)
	{
		PORTD = d_bonus | (0x01 << d_line);
		d_line ++;
		if (d_line > 4)
			d_line = 0;
		
			
	}
}
