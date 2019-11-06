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
	// Will not work if none is pressed, or pressed more than 2 buttons
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
		if (led_active == 4 - led_count)
			led_active = 5;
		led_active --;
	}
	else if (btn & 0x08)	// Right
	{
		led_active ++;
		if (led_active > 4)
			led_active = 4 - led_count;
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

void Change_led_count (uc num)
{
	if (num == 0)
		led_count = 2;
	else if (num == 1)
		led_count = 4;
	else if (num == 3)
		led_count = 1;
	else if (num > 7 && num < 10)	// 8, 9
		led_count = 0;
	else
		led_count = 3;
	led_active = 4;
}

bit Check(uc num)
{
	
	clrwdt();
	if ((num == 7) || (num == 10) || (num == 11) || (num == 15))
	{
		error_code = 4;
		return 0;
	}	
	else if (flag_rw == 0) // When reading, only the mode number is important
		return 1;
		
	int i = 0;
	int24 led_max = 2047;
	if (num == 0)
		led_max = 199;
	else if (num == 1)
		return 1;
		//led_max = 99999; // This is the maximum scoreboard
	else if (num == 2)
		led_max = 1999;
	else if (num == 3)
		led_max = 99;
	else if (num > 7 && num < 10)	// 8, 9
		led_max = 1;
	// For 4-6 and 12-14 modes, the limit will remain 1
	
	int24 led_real = 0;
	int24 factor = 1;
	for (i = 0; i < 5; i ++)
	{
		int j = 0, j_max = (int)LED[4-i];
		int24 temp = 0;
		//led_real += factor * temp;	compilator fail
		for (j = 0; j < j_max; j ++)
			temp += factor;
			
		led_real += temp;
		factor = factor * 10;
	}
	
	//If the limit is exceeded - the display will reset to the maximum value
	if (led_real > led_max)
	{
		//return 0;
		int24 temp = 10000;
		for (i = 0; i < 5; i ++)
		{
			//I doubt it
			// LED[i] = (led max / (10000 / (10^i))) % 10`
			factor = led_max / temp;
			factor = factor % 10;
			LED[i] = factor;
			temp /= 10;
		}
	}
	return 1;
}

uc Get_port_e(uc part)
{
	uc ans = 0;
	if (part == 3)
		ans += 5;
	
	part = (PORTE ^ 0xF8) >> 3; // 0b000xxxxx
	
	while (part != 0x01)
	{
		part = part >> 1;
		ans += 1;
	}
	if (ans > 6)
		ans += 1;
	/* Here you can enter the setting of the amplitude mode 1, 2, 3 */
	return ans;
}

void Read_Msg()
{
	
	clrwdt();
	// Call from Send_part()
	// bit flag_correct = 1;
	// Package[0]
	
	error_code = error_code_interrupt;
	
	uc temp = a >> 4;
	
	if (flag_mode_ampl == 1)
		temp -= 8;
	
	if (temp != mode)
		error_code = 1; 
	
	if (error_code == 0)
	{
		temp = a >> 4;
		uc Rcv_numbers [5];
		
		Rcv_numbers[0] = Rcv_numbers[1] = 0;
		Rcv_numbers[2] = Rcv_numbers[3] = Rcv_numbers[4] = 0;
		
		// Package[1] - Package[3]
		if (temp == 8 || temp == 9)
			Rcv_numbers[4] = b & 0x01;
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
			if (error_code == 0)
			{
				uc temp2 = Rcv_numbers[temp];	// Bugs and features of 
												// the compiler
				if (flag_rw == 0)
					LED[temp] = temp2;
				else if (LED[temp] != temp2)
					error_code = 1;
			}
		}
		// (error_code == 0) - - Otherwise, the alarm signal will be 
		// replaced by a parity error

		if (b & 0x40)	// Alarm signal
			error_code = 3;
	}
	return ;//1;
}

void Reg_Start_up ()
{
	GLINTD = 1;		// Disable All Interrupts
	PORTE = 0x00;	// Getting button codes and modes
	DDRE  = 0x00;
	PORTC = 0x00;	// Numbers on the scoreboard cell
	DDRC  = 0x00;
	PORTD = 0x00;	// Power for indicator and button polling
	DDRD  = 0x00;
	
	// Start signal. Only needed for layout
	DDRE = 0;
	PORTE = 0x2E; // 0b00101110
	
	for (a = 0; a < 255; a ++)
		for (b = 0; b < 255; b ++);
	
	a = b = 0;	
	
	PIR1    = 0x00;	// Reset Interrupt Request Flags
	PIE1    = 0x01;	// RCIE setting: USART receiver interrupt enable bit 
					// (there is data in the receiver buffer)
	T0STA   = 0x28;	// Switching on TMR0 (internal clock frequency, 1:16 pre-selector)
	// T0STA does not matter since interruptions not allowed
	INTSTA  = 0x08;	// PEIE setting
	
	TXSTA = 0x42;	// 0b01000010 9bit, asynchronous,
	RCSTA = 0x90;	// 0b10010000 on port, 9bit, continuous reception
	SPBRG = 0x9B;	// 155
	USB_CTRL = 0x01;	// USB launch. Low Speed (1.5 Ìבטע/c),
	
	
	GLINTD  = 0; // Reset All Interrupt Disable Bit
	CREN = 0;
	
	
	DDRE = 0xFC; 	// 0b11111100 Buttons * 5 and MANUAL/AUTO
	
	PORTE = 0;
	
	LED[0] = LED[1] = LED[2] = LED[3] = LED[4] = 0;
	
    flag_send_mode = 0;		// Turn on to receive data
    flag_rw = 0;
	led_active = 4;	// The number of the selected indicator. 
					// 4 is the far left
    mode = 255;
    
    count_receive_data = 0;
    a = b = c = d = 0;
    flag_msg_received = 0;	// Flag of received message
    error_code = 0;
    error_code_interrupt = 0;
    led_count = 3;
    
	flag_mode_ampl = 0;	
}

void Send()
{	
	uc Package [4], temp = 0;
	
	clrwdt();
	// Receiver ON
	CREN = 1;	
	count_receive_data = 0; // In case of loss of parcels, or line break
	a = b = c = d = 0;
	flag_msg_received = 0;
	//error_code = 0; //in Read_Msg()
	
	//Package [0]
	Package[0] = mode;
	
	
	if ((Package[0] > 3) && (Package[0] < 7)) 
	{
		if ((flag_mode_ampl == 0) && (LED[0] == 9))
			flag_mode_ampl = 1;
	}

	if (flag_mode_ampl == 1)
	{
		if ((Package[0] < 4) || (Package[0] > 6))
			flag_mode_ampl = 0;
		else
		{
			Package[0] += 8;
			flag_rw = 0;
		}
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
		if (Package[0] == 8 || Package[0] == 9)
		{	
			Package[1] = LED[4];
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
	
	Package[0] = (Package[0] << 4) | 0x0F;
	
	//for (i=0;i<4;i++)
	int i = 0, max = 4;
	temp = 0;
	
	
	while ((i < max) && (temp < 150))
	{	
		clrwdt();
		if (i == 4)
			TXEN = 0; // Transmitter Turn Off
		else if (TXIF == 1)	// TXIF or TRMT.
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
	 
}

void Send_part(bit flag_first_launch)
{
	static uc i;
	static uc j;
	
	clrwdt();
	
	j ++;
	if (j > 100)
	{
		j = 0;
		i ++;
	}
	
	if (((i == 0) && (j == 1)) || (flag_first_launch == 1))
		Send();
	else if ((i == 3) || ((flag_msg_received == 1) && (flag_mode_ampl == 0)))
	{
		i = j = 0;
		if (flag_msg_received == 1)
		{
			Read_Msg();
			flag_msg_received = 0;
			if ((flag_mode_ampl == 0) && (error_code == 0))
					flag_send_mode = 0;
			
		}
		else if (error_code != 4)
			error_code = 2; // Line break
	}
}


uc Show_ERROR() // Remove 
{
	static uc i; // time_show_0;
	static uc j; // time_show_1;
	uc work_led = 0x02;	// 0x02 work; 0x01 error
	clrwdt();
	
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
		if (i < 100)//128 
			work_led = 0x01;
		else if((i > 150) && (i <= 200)) // 171, 214
			work_led = 0x01;
	}

	if (i == 255)
		i = 0;
		
	return work_led;
}
