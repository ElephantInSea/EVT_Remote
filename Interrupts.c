/*The project "Respondent". Association "EVT".*/


void Check_mail (uc mail, bit nine)
{		
	while (mail)
	{
		if (mail & 0x01)
			nine = !nine;
		mail = mail >> 1;
	}
	if ((nine == 1 ) || (OERR || FERR))
		error_code = 1;
}

void Handler_receiver ()
{
	/*Прием даных. В a, b, c, d */
	
	/* Сейчас любая ошибка будет выглядеть одинаково.
	Что бы вывести разницу, следует вводить флаги.*/
	
	uc parity_marker = 0;	// Маркер четности для всех сообщений
							// Parity marker for all messages
	if(count_receive_data == 0)
		error_code = 0;
	
	// RCIF == Флаг запроса прерывания от приемника USART
	while (RCIF)	
	{
		// RCIF = 0; // Read only
		
		// bit flag_parity = RX9D; // Без рабочей проверки четности не нужен
		bit mail_parity = RX9D;
		uc mail = RCREG;
		// Переменная mail нужна для проверки четности. 
		// Проверка четного бита
		
		Check_mail (mail, mail_parity);
		if ((error_code > 0) || (count_receive_data > 3))
		{
			while(RCIF)
				mail = RCREG;
			flag_msg_received = 1;
			CREN = 0;
		}
		else
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
	PEIF = 0;
	PIR1 = 0; // На всякий случай
}
