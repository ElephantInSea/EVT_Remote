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
		if(flag_send_mode == 0)
		{
			flag_send_mode = 1;
			flag_rw = 1; //Write
		}
		else //STOP sending
		{
			flag_send_mode = flag_rw = 0;
		}
		//--------------------------
	}
	return;
}

bit Check(uc num)
{
	if (num > 12)
	{
		error_code = 4;
		return 0;
	}	
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
	{
		//return 0;
		int24 temp = 10000;
		for (i = 0; i < 5; i ++)
		{
			//I doubt it - ���������� �
			factor = led_max / temp;
			factor = factor % 10;
			LED[i] = factor;
			temp /= 10;
		}
	}
		
			
	return 1;
}

void Read_Msg()
{
	// Call from Send_part()
	// ���� ���� ������� � ������/ ������ - ��������� ��� ��������� ��������
	// ������ � ����������.
	// bit flag_correct = 1;
	// Package[0]
	uc temp = a >> 4;
	bit flag_d_line_3 = 0;
	
	if (temp > 4)
	{
		flag_d_line_3 = 1;
		if (temp == 12)
			temp = 9;	
		temp -= 5;	// �������� � ����������� - ��� ����
	}
	
	uc temp2 = 0x01 << temp;
	if (flag_d_line_3)
		temp2 |= 0x80;
		
	if (temp2 != mode)
	{
		error_code = 1; 
	}
	// ��������� ������ � 0� �������
	// ��������
	
	if (error_code == 0)
	{
		uc Rcv_numbers [5];
		
		Rcv_numbers[0] = Rcv_numbers[1] = 0;
		Rcv_numbers[2] = Rcv_numbers[3] = Rcv_numbers[4] = 0;
		
		// Package[1] - Package[3]
		if (temp2 == 4 || temp2 == 5)
			Rcv_numbers[4] = b & 0x01;
		else if ((temp2 == 12) || (b & 0x40))	// Alarm signal
			error_code = 3;
			//Rcv_numbers[4] = (b & 0x40) >> 6;
		else
		{
			Rcv_numbers[0] = b & 0x0F;
			Rcv_numbers[1] = c >> 4;
			Rcv_numbers[2] = c & 0x0F;
			Rcv_numbers[3] = d >> 4;
			Rcv_numbers[4] = d & 0x0F;
		}
		
		//(error_code == 0) - Otherwise, the alarm signal will be 
		// replaced by a parity error
		if ((flag_rw == 1) || (error_code == 0)) // Only when recording
			for (temp = 0; temp < 5; temp ++)
			{
				temp2 = Rcv_numbers[temp];	// Bugs and features of the compiler
				if (LED[temp] != temp2)
					error_code = 1;
					// flag_correct = 0;
					//return 0;
			}			
	}
	if (error_code > 0)
		flag_msg_received = 0;
	return ;//1;
}

void Reg_Start_up ()
{
	GLINTD = 1;		// ������ ���� ����������
	PORTE = 0x00;
	DDRE  = 0x00;	// ������� ���������� � ������ ������
	PORTC = 0x00;
	DDRC  = 0x00;	// �������� ����������
	PORTD = 0x00;  
	DDRD  = 0x00;
	
	// ������������� ������ � ������ � �������
	DDRE = 0;
	PORTE = 0x2E; // 0b00101110
	
	for (a = 0; a < 255; a ++)
		for (b = 0; b < 255; b ++);
	
	a = b = 0;	
	
	PIR1    = 0x00;	// ����� ������ �������� ����������
	PIE1    = 0x01;	// ��������� RCIE: ��� ���������� ���������� �� 
					// ��������� USART (� ������ ��������� ���� ������
	T0STA   = 0x28;	// ��������� TMR0 (�����. �������� �������, ������������ 1:16)
	// T0STA �� ����� ��������, �.�. ���������� �� ���������
	INTSTA  = 0x08;	// ��������� PEIE
	
	TXSTA = 0x42;	// 0b01000010 9���, ��������,
	RCSTA = 0x90;	// 0b10010000 ��� ����, 9���, ����������� �����
	SPBRG = 0x9B;	// 155
	USB_CTRL = 0x01;	// ������ USB. Low Speed (1.5 ����/c),
	
	
	GLINTD  = 0; // ����� ���� ������� ���� ����������
	CREN = 0;
	
	
	DDRE = 0xF8; // ������ � �������������
	PORTE = 0;
	
	LED[0] = LED[1] = LED[2] = LED[3] = LED[4] = 0;
	
    flag_send_mode = 0;		// Turn on to receive data
    flag_rw = 0;
	led_active = 4;	// The number of the selected indicator. 
					// 4 is the far left
    mode = 0;
    
    count_receive_data = 0;
    a = b = c = d = 0;
    flag_parity_check = 0;
    flag_receive_error = 0;
    flag_msg_received = 0;	// Flag of received message
    error_code = 0;
    
}

void Send()
{	
	uc Package [4], temp = 0;
	
	// Receiver ON
	CREN = 1;	
	count_receive_data = 0; // In case of loss of parcels, or line break
	a = b = c = d = 0;
	flag_msg_received = 0;
	//error_code = 0; //in Read_Msg()
	
	//Package [0]
	Package[0] = 0;
	temp = mode & 0x1F;	// 0b00011111
	while (temp != 0x01)
	{
		temp = temp >> 1;
		Package[0] += 1;
	}
	Package[0] -= 1; // 0b00000001 == 0) ���������
	if (mode & 0x80)
	{
		if (mode & 0x10)	// ��� ������ ��� ������ ��� ����� ����
							// ����� �� ��� ��� ����� ������������� 
							// ��� � ��� �� ��������� - �������.
			Package[0] = 12;	// ������
		else
			Package[0] += 5;
	}
	
	// the mode is greater than 13, or does 
	// not fit into the limits for the mode
	if (Check(Package[0]) == 0)
	{
		flag_msg_received = 1;
		return ;//0;
	}
	
	if (flag_rw == 0) // Read
		Package[1] = Package[2] = Package[3] = 0;
	else //Write
	{
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
	}
	
	if (flag_manual_auto)
		Package[1] |= 0x20;
		
	/* TODO (#1#): ������, ����� �� ����� ������ � �������� */
	
	Package[0] = (Package[0] << 4) | 0x0F;
	
	//for (i=0;i<4;i++)
	int i = 0, max = 5;
	temp = 0;
	
	
	while ((i < max) && (temp < 250))
	{	
		if (i == 4)
			TXEN = 0; // Transmitter Turn Off
		else if (TXIF == 1)	// TXIF ��� TRMT.
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
			TXEN = 1; // Transmitter Turn On
			i++;
		}
		else
			temp ++;	// fuze
		//not_send = 0;
	}
	
	
	if (i != max) // Sent more or less
		error_code = 4; //return 0;
	return ;//1;
	/* TODO (#1#): temp - ���������������, ����� �� ������������. */
	 
}

void Send_part()
{
	static uc i;
	static uc j;
	// static flag_send;
	j --;
	if( j <= 0)
	{
		j = 100;
		i --;
	}
	
	if ((i <= 0) || (flag_msg_received == 1))
	{
		if ( i == 0 )
		{	
			i = 3;
			if (flag_msg_received == 0)
				error_code = 2;	// Line break
			}
		//j = 100;
		if (flag_msg_received == 1) // Receiver on
		{
			Read_Msg(); 
			if (error_code > 0)
				flag_msg_received = 0;
			else
			{
				i = 0;
				flag_msg_received = 0;
				flag_send_mode = 0;
				flag_rw = 0;
			}
		}
		
		CREN = 0; // Turn off the receiver to clear errors
		if (flag_msg_received == 0)
			Send();
	}
	
	// count_receive_data = 0; //in Send()
	
	// ��� ������� ������� �������� ��������� ������� ������ ���� ������, 
	// ���� ���� � ���� ������� ������ ���.
}

uc Show_ERROR() // Remove 
{
	static uc i; // time_show_0;
	static uc j; // time_show_1;
	uc work_led = 0x02;	// 0x02 work; 0x01 error
	j++;
	if (j == 255)
	{
		j = 0;
		i ++;		
	}
	
	if(error_code == 0)			// Work
		i = j = 0;
	else if (error_code == 1)	// Parity
	{
		if (i < 10)
			work_led = 0x01; 
		else if (i < 11)
			i --;
	}
	else if(error_code == 2)	// Line is broken
	{
		if (i < 128)
			work_led = 0x01;
	}
	else if(error_code == 3)	// 12 mode, Error
		work_led = 0x01;
	else if(error_code == 4)	// Send Error
	{
		if (i < 128) 
			work_led = 0x01;
		else if((work_led > 171) && (work_led <= 214))
			work_led = 0x01;
	}

	if (i == 255)
		i = 0;
	return work_led;
}
