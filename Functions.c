/*The project "Respondent". Association "EVT".*/

void Btns_action (uc btn)
{
	uc temp = btn, count = 0;
	while(temp)
	{
		if (temp & 1)
			count ++;
		temp = temp >> 1;
	}
	// Will not work if none is pressed, or pressed more than 2
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
	{
		static int send_error_count;
		static int send_mode_count;	// Send iteration
		if( flag_send_mode == 0)	// First start
		{
			send_error_count = 0;
			send_mode_count = 0;
		}
		
		flag_send_mode = 1;
		if (mode == 255)
			return;
			
		if(Send())
			send_error_count ++;
		send_mode_count ++;
		
		uc max = 255;
		for (temp = 0; temp < max; temp ++)
			for (count = 0; count < max; count ++)
			{
				if (flag_receive_error)
					max = 0;
			}
		flag_receive_error = 0;
		/* TODO (#1#): Написать обработчик прерывания по заполнению 
		               буфера. Какого, интересно?*/
		               
		// Exit conditions
		if (flag_msg_received || (send_mode_count > 2))
		{
			/* TODO (#1#): Функция вывода принятых 
			данных на экран. Т.е. 4 принятые посылки трансформировать в числа */
			if(Read_Msg() == 0)
			{
				/* TODO (#1#): Нужна функции которая бы 
				отображала аварию. */
				Show_ERROR(flag_send_mode);
			}
			// Receive OFF
			CREN = 0;
			send_error_count = send_mode_count = 0;
			flag_send_mode = flag_msg_received = 0;
		}
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

bit Read_Msg()
{
	// Если есть пометка о записи/ чтении - сравнение или помещение принятых
	// данных в индикаторы.
	// bit flag_correct = 1;
	// Package[0]
	uc temp = a >> 4;
	bit flag_d_line_3 = 0;
	
	if (temp > 4)
	{
		flag_d_line_3 = 1;
		if (temp == 12)
			temp = 9;	
		temp -= 5;	// Проверил в комплияторе - код норм
	}
	
	uc temp2 = 0x01 << temp;
	if (flag_d_line_3)
		temp2 |= 0x80;
		
	if (temp2 != mode)
		// flag_correct = 0;
		return 0;
	// Обнаружил ошибку в 0й посылке
	// Исправил
	
	uc Rcv_numbers [5];
	
	Rcv_numbers[0] = Rcv_numbers[1] = 0;
	Rcv_numbers[2] = Rcv_numbers[3] = Rcv_numbers[4] = 0;
	// Package[1] - Package[3]
	if (temp2 == 4 || temp2 == 5)
	{
		Rcv_numbers[4] = b & 0x01;
	}
	else if (temp2 == 12)
	{
		Rcv_numbers[4] = (b & 0x40) >> 6;
	}
	else
	{
		Rcv_numbers[0] = b & 0x0F;
		Rcv_numbers[1] = c >> 4;
		Rcv_numbers[2] = c & 0x0F;
		Rcv_numbers[3] = d >> 4;
		Rcv_numbers[4] = d & 0x0F;
	}
	
	for (temp = 0; temp < 5; temp ++)
	{
		temp2 = Rcv_numbers[temp];	// Bugs and features of the compiler
		if (LED[temp] != temp2)
			// flag_correct = 0;
			return 0;
	}
	
	
	return 1;
}

bit Send()
{	
	uc Package [4], temp = 0;
	
	//Package [0]
	Package[0] = 0;
	temp = mode & 0x1F;	// 0b00011111
	while (temp != 0x01)
	{
		temp = temp >> 1;
		Package[0] += 1;
	}
	Package[0] -= 1; // 0b00000001 == 0) Дальность
	if (mode & 0x80)
	{
		if (mode & 0x10)	// Это потому что авария как режим была
							// Нужно ли она как режим переключателя 
							// или я сам ее выставляю - загадка.
			Package[0] = 12;	// Авария
		else
			Package[0] += 5;
	}
	
	if (Check(Package[0]) == 0)
		return 0;
	
	//Package [1]
	//if (mode & 0x90)	// 0b10010000
	if (Package[0] == 12 || Package[0] == 4 || Package[0] == 5)
	{	
		Package[1] = LED[4];
		if (Package[0] == 12)
			Package[1] = Package[1] << 6;
		Package[2] = Package[3] = 0;
	}
	else
	{
		Package[1] = LED[0];
		Package[2] = (LED[1] << 4) | LED[2];
		Package[3] = (LED[3] << 4) | LED[4];
	}
	Package[1] |= 0x80;
	
	if (flag_manual_auto)
		Package[1] |= 0x20;
		
	/* TODO (#1#): Узнать, будет ли режим чтения у запросов */
	
	Package[0] = (Package[0] << 4) | 0x0F;
	
	//for (i=0;i<4;i++)
	int i = 0, max = 4;
	temp = 0;
	
	// Receiver ON
	CREN = 1;	
	
	while ((i < max) && (temp < 250))
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
		else
		{
			temp ++;
		}
		//not_send = 0;
	}
	if (i == max)
		return 0;
	return 1;
	/* TODO (#1#): temp - предлохренитель, нигде не используется. */
	 
}

void Show_ERROR(bit flag_error)
{
	// Возможно с помощью лампочек Авария/Работа 
	return;
}
