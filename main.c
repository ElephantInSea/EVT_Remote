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
bit flag_msg_received;
bit flag_parity_check;
bit flag_receive_error;
// Cells for receiving messages
uc a, b, c, d;
uc count_receive_data;
interrupt iServer(void)
{
	multi_interrupt_entry_and_save

	PERIPHERAL_service:
		PEIF = 0;
		/*Прием даных. В a, b, c, d
		msg_received = 1;*/
		uc fuze = 0;
		uc parity_marker = 0;	// Маркер четности для всех сообщений
								// Parity marker for all messages
		// RCIF == Флаг запроса прерывания от приемника USART
		while (RCIF && (fuze < 50) && OERR == 0)	
		{
			fuze ++;
			// RCIF = 0; // Read only
			
			count_receive_data++;
			uc mail = RCREG;
			// Проверка четного бита
			if (flag_parity_check)
			{
				/* TODO (#1#): Написать функцию провекри четности, и 
				               продумать реакцию. */
				// Вероятно компилятор не переварит конструкцию
				//parity_marker |= Parity_check(mail,RX9D) << count_receive_data;
			}
			if (count_receive_data == 0)
				a = mail;
			else if (count_receive_data == 1)
				b = mail;
			else if (count_receive_data == 2)
				c = mail;
			else
			{
				d = mail;
				count_receive_data = 0;
				flag_msg_received = 1;
				fuze = 51; // RCIF должен быть сброшен. Это излишек.
				// Выключить приемник?
				// Receiver OFF
				CREN = 0;
				// Включать сначала передачи. 
			}
			fuze ++;
			// Что делать с ошибками OERR FERR?
			if (OERR && parity_marker)
			{
				flag_msg_received = 0;
				// Нужно написать еще один флаг об ошибке приема. 
				// Что бы не ожидать таймер отправки следующего сообщения.
			}
			if (OERR)
			{
				CREN = 0;
				// ...
				CREN = 1;
				//FERR
				flag_receive_error = 0;
			}
		}
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
/******************/

#include "math24.h"
#include "Functions.h"

void main(void)
{
	LED[0] = LED[1] = LED[2] = LED[3] = LED[4] = 0;
    Check(255);
    Btns_action(0);	// Bugs and features of the compiler
    Send();
    
    flag_send_mode = 0;		// Turn on to receive data
    int send_mode_count = 0;	// Send iteration
    int send_error_count = 0;	
    flag_msg_received = 0;	// Flag of received message
    
	int d_line = 0;	// Working indicator number
	uc d_bonus = 0; // In case of work with 1 and 0 bits of port D
	
	led_active = 4;	// The number of the selected indicator. 
					// 4 is the far left
    int led_blink = 0;
	uc temp = 0;
    
    mode = 0;
    uc mode_temp = 0, mode_time = 0;
    uc buttons = 0, buttons_time = 0; 
    
    count_receive_data = 0;
    a = b = c = d = 0;
    flag_parity_check = 0;
    flag_receive_error = 0;
    
	while (1)
	{
		if (flag_send_mode == 1)
		{
			if (mode == 255)
			{
				if (d_line == 2)
					d_line = 3;
				else
					d_line = 1;
			}
			else
				d_line = 0;
		}
		
		
		temp = 0x08 << d_line;
		temp |= d_bonus;
		PORTD = temp;
		d_line ++;
		if (d_line > 4)
			d_line = 0;
		
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
		
		/* TODO (#1#): Что делать с 14м контактом, который 
		               RE2_WR_TXOE? */
		
		temp = (PORTE ^ 0xF8) >> 3;	// Port E is inverted
		if((d_line & 0x01) && (temp > 0))	// mode
		{
			// Parity condition and nonzero reception
			
			// Mark of the second line of switches
			if (d_line == 3)
				temp |= 0x80;	// 0b100xxxxx
				
			if (mode != temp)
			{
				if(mode_temp == temp)
				{
					mode_time ++;
					if (mode_time > 20)
						mode = temp;
				}
				else
				{
					mode = 255;		// Fuse
					mode_temp = temp;
					mode_time = 0;
					led_active = 4;
					LED[0] = LED[1] = LED[2] = LED[3] = LED[4] = 0;
				}
			}	
		}
		else if ((mode & 0x01) == 0)	//Buttons
		{
			if (temp == buttons)
			{
				if (buttons_time <= 50)	// A pressed key will work
					buttons_time ++;	// only once
					//
				if (buttons_time == 50)
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
	}
}

#include "Functions.c"
