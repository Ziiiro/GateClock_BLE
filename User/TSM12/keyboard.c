#include "./Delay/delay.h"
#include "./TSM12/keyboard.h"
#include "./TSM12/TSM12.h"
#include "./Delay/delay.h"
#include "./usart/debug_usart.h"
#include "./lock_ctrl/lock_ctrl.h"
#include <stdio.h>

uint8_t Key_Buffer[30];
extern float 	Battery_quantity;


uint8_t IsKey(void)
//���ذ��� ������Ϊ 0 1 2 3 ~ 9  �Ǻ� 0x0a ���� 0x0b��
//ע�� �����ӳٵȴ�
{
	uint32_t timeout_cnt=0;
	uint8_t key;
	key = TMS12_ReadOnKey();
	if(key == KEY_NULL)
		return NO_KEY;
	else{
		// ����˵�������˰�����������һ������������Ϊ���¾��죬��������������
		SPEAK_DUDUDU();
		// �ȴ�����
		while (TMS12_ReadOnKey()!=KEY_NULL){
			timeout_cnt++;
			if(timeout_cnt==0x5000)
			{
				Disp_sentence(0,0,"keyboard error",1);
				delay_ms(1000);
				break;
			}
		}
		switch (key){
			case KEY_0:
			{
				//printf("0\n");
				return 0x00;
			}
			case KEY_1:
			{
				//printf("1\n");
				return 0x01;
			}
			case KEY_2:
			{
				//printf("2\n");
				return 0x02;
			}
			case KEY_3:
			{
				//printf("3\n");
				return 0x03;
			}
			case KEY_4:
			{
				//printf("4\n");
				return 0x04;
			}
			case KEY_5:
			{
				//printf("5\n");
				return 0x05;
			}
			case KEY_6:
			{
				//printf("6\n");
				return 0x06;
			}
			case KEY_7:
			{
				//printf("7\n");
				return 0x07;
			}
			case KEY_8:
			{
				//printf("8\n");
				return 0x08;
			}
			case KEY_9:
			{
				//printf("9\n");
				return 0x09;
			}
			case KEY_AST:
			{
				//printf("*\n");
				return 0x0a;
			}
			case KEY_POU:
			{
				//printf("#\n");
				return 0x0b;
			}
			default:
				return 100;
		}
	}
}
uint8_t Key_Cap_6bits(uint8_t first_key, uint8_t* length,uint8_t disp_or_not)//���水������
//�������ֻ����6λ���룬�����������룬��Ϊֻ������6λ
{
	int i;
	int j;
	int k;
	uint8_t key;
	*length = 0;
	char disp[7] = "      ";
	disp[0] = first_key + 48;
	if (disp_or_not)
			Disp_sentence(0,3,"*",1);
	else
			Disp_sentence(0,3,disp,1);
	for (i=0;i<30;i++)
	{
		Key_Buffer[i] = NO_KEY;
	}


		Disp_line(1,22,127,0);
		Disp_line(1,41,127,0);

	Key_Buffer[0] = first_key;
	for (i=1;i<7;i++)//����iȷʵ�Ǵ�1��ʼ��������0��û������,��i����1ʱ��Key_Bufferֻ������Key_Buffer[0],Ҳ����˵����ʱֻ��һλ����
	{
		key = Wait_Key();
		if(key == NO_KEY)//���볬ʱ
		{
			for (i=0;i<6;i++)//���볬ʱ����հ���buffer
			{
				if(Key_Buffer[i] == NO_KEY)
					break;
				else
					Key_Buffer[i] = NO_KEY;
			}
			return KEY_TIMEOUT;
		}
		else if(key == 0x0a)//����*ȡ��
		{
			if(i==0)
			{
				return KEY_CANCEL;
			}
			else
	  	{
				i=i-1;
				Key_Buffer[i] = NO_KEY;//ɾ����һλ
				i=i-1;//i��-1 ����for�����i++
				//��ʾ����
				k = i-5;//�ӵ�ǰ����λ��ǰ��6λ��ʼ��ʾ
				j = 0;
				while(j<6)
				{
					if (k>=0)
					{
						if (disp_or_not)
							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
						else
							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
						j++;
					}
					k++;
				}
	  	}
		}

		else if (key == 0x0b)//����#ȷ��
		{
			*length = i;
			if(i<6)
			{
				Disp_sentence(2,2,"�������Ϊ6λ",1);
				delay_ms(1000);
								OLED_CLS();
				Disp_line(1,22,127,0);			//////*********wenti ;
				Disp_line(1,41,127,0);

				i--;
			}
			else
				return KEY_CONFIRM;
		}
		else //�����������buffer
		{
			if(i==6)//i==6˵���Ѿ����빻6λ�ˣ���ʱ��i��Ϊ5���ٴβ������һλ
			{
				i=5;
			}
			Key_Buffer[i] = key;
			//��ʾ����
			k = i-5;//�ӵ�ǰ����λ��ǰ��8λ��ʼ��ʾ
			j = 0;
			while(j<6)
			{
				if (k>=0)
				{
					if (disp_or_not)
							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
					else
							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
					j++;
				}
				k++;
			}
		}
			Disp_sentence(1,3,disp,0);
	}
	//��������ܵ����˵������̫����ʵ�������ܲ�������ģ�
	return KEY_TOO_LONG;
}

// ���������롿����
// first_key	�ǵ�һ����������Ϊ���������Ҫ�����̴��������Ե�һ���������ں���֮��ͷ�����
// length		�ǰ������еĳ��ȣ�������ȷ�ϼ�
// disp_or_not	������ѡ��disp_or_not=0,��ʾ��������֣����ڵǼǣ�disp_or_not=1������ʾ��������֣����ڽ�����ͨ����֤
uint8_t Key_Cap(uint8_t first_key, uint8_t* length,uint8_t disp_or_not)
//���в�׽����ȫ�ֱ��� Key_Buffer
//������������������ʱ����Ϊ��ʱ��������ܶ�λ���룬����������ʱ��ֻ������6λ
{
	int i;
	int j;
	int k;
	uint8_t key;
	*length = 0;
	char disp[9] = "        ";
	disp[0] = first_key + 48;
	if (disp_or_not)
		Disp_sentence(0,3,"*",1);
	else
		Disp_sentence(0,3,disp,1);
	for (i=0;i<30;i++){
		Key_Buffer[i] = NO_KEY;
	}

	Disp_line(0,22,127,0);
	Disp_line(0,41,127,0);

	Key_Buffer[0] = first_key;

	// ����iȷʵ�Ǵ�1��ʼ��������0��û������,��i����1ʱ��Key_Bufferֻ������Key_Buffer[0],Ҳ����˵����ʱֻ��һλ����
	for (i=1;i<30;i++){
		key = Wait_Key();
		// ���볬ʱ
		if(key == NO_KEY){
			// ���볬ʱ����հ���buffer
			for (i=0;i<30;i++){
				if(Key_Buffer[i] == NO_KEY)
					break;
				else
					Key_Buffer[i] = NO_KEY;
			}
			return KEY_TIMEOUT;
		}
		// ����*ȡ��
		else if(key == 0x0a){
			if(i==0){
				return KEY_CANCEL;
			}
			else {
				i=i-1;
				Key_Buffer[i] = NO_KEY;//ɾ����һλ
				i=i-1;//i��-1 ����for�����i++
				//��ʾ����
				k = i-7;//�ӵ�ǰ����λ��ǰ��8λ��ʼ��ʾ
				j = 0;
				while(j<8){
					if (k>=0){
						if (disp_or_not)
							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
						else
							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
						j++;
					}
					k++;
				}
			}
		}
		// ����#ȷ��
		else if (key == 0x0b){
			Key_Buffer[i] = 0x0b;//ȷ��λ������������
			*length = i;
			if(i==0){
				Disp_sentence(18,4,"���벻��Ϊ��",1);
				delay_ms(1000);
				OLED_CLS();
				Key_Buffer[i] = NO_KEY;//ɾ����λ
				i=i-1;//����i++,����¼��first_key
			}
			else
				return KEY_CONFIRM;
		}
		// �����������buffer
		else{
			Key_Buffer[i] = key;
			// ��ʾ����
			// �ӵ�ǰ����λ��ǰ��8λ��ʼ��ʾ
			k = i-7;
			j = 0;
			while(j<8){
				if (k>=0){
					if (disp_or_not)
						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
					else
						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
					j++;
				}
				k++;
			}
		}
		Disp_sentence(0,3,disp,0);
	}
	// ��������ܵ����˵������İ�����̫����
	return KEY_TOO_LONG;
}

// 
uint8_t start_interface_key_cap(uint8_t first_key, uint8_t* length,uint8_t disp_or_not){
	int i;
	int j;
	int k;
	uint8_t key;
	*length = 0;
	char disp[9] = "        ";
	disp[0] = first_key + 48;


	for (i=0;i<30;i++){
		Key_Buffer[i] = NO_KEY;
	}

	Key_Buffer[0] = first_key;

	// ����iȷʵ�Ǵ�1��ʼ��������0��û������,��i����1ʱ��Key_Bufferֻ������Key_Buffer[0],Ҳ����˵����ʱֻ��һλ����
	for (i=1;i<30;i++){
		key = Wait_Key();
		// ���볬ʱ
		if(key == NO_KEY){
			// ���볬ʱ����հ���buffer
			for (i=0;i<30;i++){
				if(Key_Buffer[i] == NO_KEY)
					break;
				else
					Key_Buffer[i] = NO_KEY;
			}
			return KEY_TIMEOUT;
		}
		// ����*ȡ��
		else if(key == 0x0a){
			if(i==0){
				return KEY_CANCEL;
			}
			else {
				i=i-1;
				Key_Buffer[i] = NO_KEY;//ɾ����һλ
				i=i-1;//i��-1 ����for�����i++
				//��ʾ����
				k = i-7;//�ӵ�ǰ����λ��ǰ��8λ��ʼ��ʾ
				j = 0;
				while(j<8){
					if (k>=0){
						if (disp_or_not)
							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
						else
							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
						j++;
					}
					k++;
				}
			}
		}
		// ����#ȷ��
		else if (key == 0x0b){
			Key_Buffer[i] = 0x0b;//ȷ��λ������������
			*length = i;
			if(i==0){
				Disp_sentence(16,4,"���벻��Ϊ��",1);
				delay_ms(1000);
				OLED_CLS();
				Key_Buffer[i] = NO_KEY;//ɾ����λ
				i=i-1;//����i++,����¼��first_key
			}
			else
				return KEY_CONFIRM;
		}
		// �����������buffer
		else{
			Key_Buffer[i] = key;
			// ��ʾ����
			// �ӵ�ǰ����λ��ǰ��8λ��ʼ��ʾ
			k = i-7;
			j = 0;
			while(j<8){
				if (k>=0){
					if (disp_or_not)
						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
					else
						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
					j++;
				}
				k++;
			}
		}
		Disp_sentence_singleline(0,4,disp,0);
	}
	//��������ܵ����˵������İ�����̫����
	return KEY_TOO_LONG;
}




// admin_verify3 ��������С����
uint8_t admin_verify3_key_cap(uint8_t first_key, uint8_t* length)
//���в�׽����ȫ�ֱ��� Key_Buffer
//������������������ʱ����Ϊ��ʱ��������ܶ�λ���룬����������ʱ��ֻ������6λ
{
	int i;
	int j;
	int k;
	uint8_t key;
	*length = 0;
	char disp[9] = "        ";
	disp[0] = first_key + 48;

	Disp_sentence(0,4,"*",0);

	for (i=0;i<30;i++){
		Key_Buffer[i] = NO_KEY;
	}


	Key_Buffer[0] = first_key;

	// ����iȷʵ�Ǵ�1��ʼ��������0��û������,��i����1ʱ��Key_Bufferֻ������Key_Buffer[0],Ҳ����˵����ʱֻ��һλ����
	for (i=1;i<30;i++){
		key = Wait_Key();
		// ���볬ʱ
		if(key == NO_KEY){
			// ���볬ʱ����հ���buffer
			for (i=0;i<30;i++){
				if(Key_Buffer[i] == NO_KEY)
					break;
				else
					Key_Buffer[i] = NO_KEY;
			}
			return KEY_TIMEOUT;
		}
		// ����*ȡ��
		else if(key == 0x0a){
			if(i==0){
				return KEY_CANCEL;
			}
			else {
				i=i-1;
				Key_Buffer[i] = NO_KEY;//ɾ����һλ
				i=i-1;//i��-1 ����for�����i++
				//��ʾ����
				k = i-7;//�ӵ�ǰ����λ��ǰ��8λ��ʼ��ʾ
				j = 0;
				while(j<8){
					if (k>=0){
						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
						j++;
					}
					k++;
				}
			}
		}
		// ����#ȷ��
		else if (key == 0x0b){
			Key_Buffer[i] = 0x0b;//ȷ��λ������������
			*length = i;
			if(i==0){
				Disp_sentence(18,4,"���벻��Ϊ��",1);
				delay_ms(1000);
				OLED_CLS();
				Key_Buffer[i] = NO_KEY;//ɾ����λ
				i=i-1;//����i++,����¼��first_key
			}
			else
				return KEY_CONFIRM;
		}
		// �����������buffer
		else{
			Key_Buffer[i] = key;
			// ��ʾ����
			// �ӵ�ǰ����λ��ǰ��8λ��ʼ��ʾ
			k = i-7;
			j = 0;
			while(j<8){
				if (k>=0){
					disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
					j++;
				}
				k++;
			}
		}
		Disp_sentence(0,4,disp,0);
	}
	//��������ܵ����˵������İ�����̫����
	return KEY_TOO_LONG;
}



// ���ȴ��İ�����ȡ
uint8_t Wait_Key(void)
{
	uint8_t key ;
	int cnt=0;
	do
	{
		key = IsKey();
		delay_ms(WAIT_SCAN_MS);
		cnt++;
	}while(key == NO_KEY && cnt < (WAIT_TIME_MS/WAIT_SCAN_MS));
	return key;
}
