// The project "Respondent". Association "EVT".
/*20 08 2019
I remember the work done, and fix bugs. I am translating comments 
for the git. Done.
Add to the end the function of sending messages.
*/
#include "1886ve4d.h"
#include "int17xxx.h"
#pragma origin 0x8

typedef unsigned char uc;

const uc Translate_num_to_LED[10] = {
//  0,	  1,	2,	  3,	4,	  5,	6,	  7,	8,	  9.
	0xC0, 0xF9, 0xA4, 0xB0, 0x98, 0x92, 0x82, 0xF8, 0x80, 0x90};
uc LED [5];
int led_active;
uc mode;

bit flag_send_mode;
bit flag_rw; // 0 read, 1 write
bit flag_msg_received;
bit flag_manual_auto;
// Cells for receiving messages
uc a, b, c, d;
uc count_receive_data;
uc error_code;
uc error_code_interrupt;

#include "Interrupts.h"

interrupt iServer(void)
{
	multi_interrupt_entry_and_save

	PERIPHERAL_service:
		
		Handler_receiver ();
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
		
	/*Прием даных. В a, b, c, d
	msg_received = 1;*/
}

#include "Interrupts.c"
/******************/

#include "math24.h"
#include "Functions.h"

void main(void)
{
	Reg_Start_up();

	uc temp = 0;
	
    Check(255);
    Btns_action(0);	// Bugs and features of the compiler
    Send();
    
	int d_line = 0;	// Working indicator number
	
    int led_blink = 0;
    
    uc mode_temp = 0, mode_time = 0;
    uc buttons = 0, buttons_time = 0; 
    
    bit flag_first_launch = 1;
	
	while (1)
	{		
		clrwdt();
		// PORT D --------------------------------------------------------------
		temp = 0x08 << d_line;
		temp |= Show_ERROR (); //d_work_light;
		PORTD = temp;
		
		// PORT C --------------------------------------------------------------
		if (d_line == led_active)	// For two iterations, the selected
		{							// indicator will be turned off
			led_blink ++;
			if (led_blink > 2)		// Delay for blinking
				led_blink = 0;
		}
		
		if ((d_line == led_active) && led_blink)
			temp = 0;
		else
		{
			temp = LED[(int)d_line];// The order of indicators is determined 
									// here.
			temp = Translate_num_to_LED[(int)temp];
		}
		PORTC = temp;
		
		
		clrwdt();
		
		// PORT E --------------------------------------------------------------
		if (PORTE & 0x04)			//0b00000100
			flag_manual_auto = 0;	// invert
		else
			flag_manual_auto = 1;
		
		temp = (PORTE ^ 0xF8) >> 3;	// Port E is inverted
		if((d_line & 0x01) && (temp > 0))	// mode
		{
			// Parity condition and nonzero reception
			temp = Get_port_e(d_line);
			
			if (mode != temp)
			{
				if(mode_temp == temp)
				{
					mode_time ++;
					if (mode_time > 20)
					{
						mode = temp;
						flag_send_mode = 1;
						flag_rw = 0; //Read
					}
				}
				else
				{
					mode = 255;		// Fuse
					flag_send_mode = 0;
					mode_temp = temp;
					mode_time = 0;
					led_active = 4;
					LED[0] = LED[1] = LED[2] = LED[3] = LED[4] = 0;
				}
			}	
		}
		else if ((d_line & 0x01) == 0)	//Buttons
		{
			if (temp == buttons)
			{
				if (buttons_time <= 50)	// A pressed key will work
					buttons_time ++;	// only once
					//
				if ((buttons_time == 50) && buttons > 0)
				{
					/* TODO (#1#): Определить что делать с неотправленным 
					               сообщением */
					Btns_action (buttons);
				}
			}
			else 
			{
				buttons_time = 0;
				buttons = temp;
			}
		}
		clrwdt();
		
		// Send Part -----------------------------------------------------------
		if ((flag_send_mode == 1) && (mode != 255))
		{
			Send_part(flag_first_launch);
			if (flag_first_launch)
				flag_first_launch = 0;
		}
		
		d_line ++;
		if (d_line > 4)
			d_line = 0;
		
	}
}

#include "Functions.c"
