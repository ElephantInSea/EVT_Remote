// тестовый файл для компиляции
/*05 08 2019
Работа с портом Д. Сделано.
Работа с портом Ц. Сделано.
Работа с портом Е.*/
#include "1886ve4d.h"
#include "int17xxx.h"
#pragma origin 0x8

typedef unsigned char uc;

const uc Translate_num_to_LED[10] = {
//  0,	  1,	2,	  3,	4,	  5,	6,	  7,	8,	  9.
	0xC0, 0xF9, 0xA4, 0xB0, 0x98, 0x92, 0x82, 0xF8, 0x80, 0x90};
uc LED [5];
int led_active;

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

void btns_action (uc btn);

void main(void)
{
	int d_line = 0;
	uc d_bonus = 0;
	//uc LED[5] = {0, 0, 0, 0, 0};
	
	LED [0] = 0;
	LED [1] = 0;
	LED [2] = 0;
	LED [3] = 0;
	LED [4] = 0;
	led_active = 0;
    uc temp = 0;
    
    uc mode = 0;
    uc buttons = 0, buttons_time = 0; // , buttons_time_out = 255
    btns_action(0);
	while (1)
	{
		temp = 0x03 << d_line;
		temp |= d_bonus;
		PORTD = temp;
		d_line ++;
		if (d_line > 4)
			d_line = 0;
		
		temp = LED[(int)d_line];
		temp = Translate_num_to_LED[(int)temp];
		PORTC = temp;
		
		temp = PORTE >> 3;
		if(d_line & 0x01)	// mode
		{
		}
		else	//Buttons
		{
			if (temp == buttons)
			{
				if (buttons_time <= 50)
					buttons_time ++;
					/* TODO (#2#): Функция зажатия клавиши - таймаута */
				if (buttons_time == 50)
					btns_action (buttons);
			}
			else 
				buttons_time = 0;
		}
	}
}

void btns_action (uc btn)
{
	uc temp = btn, count = 0;
	while(temp)
	{
		if (temp & 1)
			count ++;
		temp = temp >> 1;
	}
	if (count != 1)
		return;
		
	if (btn & 0x01)	// Up
	{
		LED[led_active] = LED[led_active] + 1;
		if (LED[led_active] > 9)
			LED[led_active] = 0;
	}
	else if (btn & 0x02)	// Down
	{
		if (LED[led_active] == 9)
			LED[led_active] = 10;
		LED[led_active] = LED[led_active] - 1;
	}
	else if (btn & 0x04)	// Left
	{
		if (led_active < 0)
			led_active = 5;
		led_active --;
	}
	else if (btn & 0x08)	// Right
	{
		led_active ++;
		if (led_active > 4)
			led_active = 0;
	}
	else if (btn & 0x10)	// Send
	/* TODO (#1#): Дописать функцию отправки */
	{}
	return;
}
