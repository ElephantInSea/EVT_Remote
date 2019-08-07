/*Проект "Ответчик". Ассоциация ЭВТ.*/

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
		if (LED[led_active] == 0)
			LED[led_active] = 10;
		LED[led_active] = LED[led_active] - 1;
	}
	else if (btn & 0x04)	// Left
	{
		if (led_active == 0)
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
		Send();
	return;
}

void Send()
{
	if (mode == 255)
	{
		not_send = 1;
		return;
	}
	
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
	
	if (Check(Package[0]) == 0)
		return;
	
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
	
	//for (i=0;i<4;i++)
	int i = 0, max = 4;
	while (i < max)
	{	
		if (TXIF == 1)	// TXIF или TRMT.
		{
			bit parity = 0;
			int t = (int)Package[i];
			while (t)
			{
				if (t & 0x01)
					parity = !parity;
				t = t >> 1;
			}
			TX9D = parity; //1
			
			TXREG = Package[i];
			TXEN = 1;
			i++;
		}
		not_send = 0;
	}
	return;
}

bit Check(uc num)
{
	if (num > 13)
		return 0;
		
	int i = 0;
	int24 led_max = 1;	
	if (num == 0)
		led_max = 199;
	else if (num == 1)
		led_max = 99999;
	else if (num == 2)
		led_max = 1999;
	else if (num == 3)
		led_max = 100;
	else if (num > 5 && num < 9)	// 6, 7, 8
		led_max = 2047;	
	
	int24 led_real = 0;
	int24 factor = 1;
	for (i = 0; i < 5; i ++)
	{
		int j = 0, j_max = (int)LED[i];
		int24 temp = 0;
		//led_real += factor * temp;	compilator fail
		for (j = 0; j < j_max; j ++)
			temp += factor;
			
		led_real += temp;
		factor = factor * 10;
	}
	if (led_real > led_max)
		return 0;	
	return 1;
}
