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

void Btns_action (uc btn);
void Send();

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
	led_active = 0;
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
				}
			}	
		}
		else	//Buttons
		{
			if (temp == buttons)
			{
				if (buttons_time <= 50)
					buttons_time ++;
					/* TODO (#2#): Функция зажатия клавиши - таймаута */
				if (buttons_time == 50)
					Btns_action (buttons);
			}
			else 
				buttons_time = 0;
		}
	}
}

void Btns_action (uc btn)
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

void Send()
{
	if (mode == 255)
	{
		not_send = 1;
		return;
	}
	// работа функции опирается на ожидание что данные 
	// не превышают лимиты.
	// Как проверить 2047?
	uc Package [4], temp = 0;
	
	//Package [0]
	Package[0] = 0;
	temp = mode & 0x1F;	// 0b00011111
	while (temp != 0x01)
	{
		temp = temp >> 1;
		Package[0] += 1;
	}
	if (mode & 0x80)
	{
		if (mode & 0x10)
			Package[0] = 12;	// Авария
		else
			Package[0] += 5;
	}
	
	//Package [1]
	//if (mode & 0x90)	// 0b10010000
	if (Package[0] == 12)
	{	
		Package[1] = LED[4] << 6;
		Package[2] = Package[3] = 0;
	}
	else
	{
		Package[1] = LED[0];
		Package[2] = (LED[1] << 4) | LED[2];
		Package[3] = (LED[3] << 4) | LED[4];
	}
	Package[1] |= 0x80;
	/* TODO (#1#): Узнать, будет ли режим чтения у запросов */
	
	
	Package[0] = (Package[0] << 4) | 0x0F;
	/* TODO (#1#): Дописать непосредственно отправку посылки */
	
	
	return;
}
