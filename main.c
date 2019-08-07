// тестовый файл для компиляции
/*05 08 2019
Работа с портом Д. Сделано.
Работа с портом Ц. Сделано.
Работа с портом Е. Сделано.
Внутренние связи компонентов.*/
#include "1886ve4d.h"
#include "int17xxx.h"
#pragma origin 0x8

typedef unsigned char uc;

const uc Translate_num_to_LED[10] = {
//  0,	  1,	2,	  3,	4,	  5,	6,	  7,	8,	  9.
	0xC0, 0xF9, 0xA4, 0xB0, 0x98, 0x92, 0x82, 0xF8, 0x80, 0x90};
uc LED [5];
int led_active;
bit not_send;
uc mode;

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
#include "Functions.h"
void Btns_action (uc btn);
void Send();
bit Check(uc num);

void main(void)
{
    Btns_action(0);	// потому что компилятор.
    Send();
    
    not_send = 0;
    mode = 0;
    
	int d_line = 0;
	uc d_bonus = 0;
	//uc LED[5] = {0, 0, 0, 0, 0};
	
	LED [0] = 0;
	LED [1] = 0;
	LED [2] = 0;
	LED [3] = 0;
	LED [4] = 0;
    Check(255);
    
	led_active = 4;	// При запуске  будет активен крайне правый индикатор
    int led_blink = 0;
	uc temp = 0;
    
    uc mode_temp = 0, mode_time = 0;
    uc buttons = 0, buttons_time = 0; // , buttons_time_out = 255
	while (1)
	{
		temp = 0x03 << d_line;
		temp |= d_bonus;
		PORTD = temp;
		d_line ++;
		if (d_line > 4)
			d_line = 0;
		
		if (d_line == led_active)
		{
			led_blink ++;
			if (led_blink > 2)	// задержка для моргания
				led_blink = 0;
		}
		if ((d_line == led_active) && led_blink) 
			temp = 0;	
		else
		{
			temp = LED[(int)d_line];
			temp = Translate_num_to_LED[(int)temp];
		}
		PORTC = temp;
		/* DONE (#1#): Настроить мигание выбранного индикатора */
		
		temp = PORTE >> 3;
		if((d_line & 0x01) && (temp > 0))	// mode
		{
			if (d_line == 3)
				temp |= 0x80;	// 0b100xxxxx
				
			if (mode != temp)
			{
				if( mode_temp == temp)
				{
					mode_time ++;
					if (mode_time > 20)
						mode = temp;
				}
				else
				{
					mode = 255;
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
				if (buttons_time <= 50)
					buttons_time ++;
					/* TODO (#2#): Функция зажатия клавиши - таймаута */
				if (buttons_time == 50 || not_send)
					Btns_action (buttons);
			}
			else 
				buttons_time = 0;
		}
	}
}

#include "Functions.c"
