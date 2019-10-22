/*The project "Respondent". Association "EVT".*/


void Check_mail (uc mail, bit nine)
{		
	while (mail)
	{
		if (mail & 0x01)
			nine = !nine;
		mail = mail >> 1;
	}
	if ((nine == 1) || (OERR || FERR))
	{
		error_code_interrupt = 1;
		CREN = 0;	//Receiver off
	}
}

void Handler_receiver ()
{
	/*Reception of data in variables a, b, c, d*/
		
	if(count_receive_data == 0)
		error_code_interrupt = 0;
	
	// RCIF == USART receiver interrupt request flag
	while (RCIF)	
	{
		// RCIF = 0; // Read only
		
		bit mail_parity = RX9D;
		uc mail = RCREG;
				
		Check_mail (mail, mail_parity);
		if ((error_code_interrupt == 0) && (count_receive_data < 4))
		{
			if (count_receive_data == 0)
				a = mail;
			else if (count_receive_data == 1)
				b = mail;
			else if (count_receive_data == 2)
				c = mail;
			else
			{
				d = mail;
			}
			count_receive_data++;
		}
	}
	
	if ((error_code_interrupt > 0) || (count_receive_data > 3))
	{
		flag_msg_received = 1;
		CREN = 0;	//Receiver off
	}
	
	PEIF = 0;
	PIR1 = 0; // Just in case
}
