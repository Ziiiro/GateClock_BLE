#include "./lock_ctrl/lock_ctrl.h"
#include <math.h>
#include <stdlib.h>

extern uint8_t Key_Buffer[30];
extern QS808_Rec_Buf_type QS808_Rec_Buf;
extern unsigned char HZ16x16[];
extern unsigned char F8X16[];
extern uint8_t qs808rec[26];
extern uint32_t ms10_cnt;	// �� timer.c ���ж��壬
extern float 	Battery_quantity;
extern uint8_t debug_data_read_flag;
extern uint8_t WAKEUP_SOURCE;
extern _calendar_obj calendar;//ʱ�ӽṹ��
extern uint8_t timenew;

// 0��ʾ������ 1��ʾ�ر�����
uint16_t SPEAK_flag=0;


/******************************************* ʱ����� *******************************************/
// �����桿��ȫ����ʾʱ��
void start_interface(){
	uint8_t key;
	uint8_t temp=0;
	uint8_t flag=0;	//�궨�Ǻ�
	uint32_t RFCARD_ID;
	uint32_t standby_cnt=0;
	ms10_cnt=0;
	int qs808_refresh_cnt=30;	//qs808���Ϳ��Ź����������������Ϊ0��˵����1.5s����û��ɨ��ָ���ˣ���ʱӦ��ˢ��qs808����

	QS808_Rec_Buf_refresh();
	Battery_quantity = Get_Battery();

	while(1) {

		// �������ʽ������Ӧ�ñ����룬������ǿϵͳ�ȶ��ԣ�����һ������²�������������
		qs808_refresh_cnt--;
		if(debug_data_read_flag){
			flashdata2usart();
		}
		if(qs808_refresh_cnt<=0){
			qs808_refresh_cnt=30;
			QS808_Rec_Buf_refresh();
		}

		standby_cnt++;

		// ˯��֮ǰҪȷ�����趼���ڹ���
		if((standby_cnt >= 100)&&(QS808_Rec_Buf.Trans_state == reset)){
			flag=0;//˯��֮ǰ���ֱ�����0
			standby_cnt=0;
			QS808_Rec_Buf_refresh();
			// ˯��֮ǰ��һ�ε���
			Battery_quantity=Get_Battery();
			PWR_Standby_Mode();
			// ���Ѻ󣬴����ﻹ���ܳ���
			// ��ָ�ƻ���ϵͳ����һ�ν����������ٶȿ�
			if(!WAKEUP_SOURCE){
				Disp_sentence(28,2,"������֤",1);
				// ָ�ƽ���,��һ����������
				if( unlock(0x00,1,0x00000000) ==  VERIFF_SUCCESS ){
					standby_cnt = 100;
				}
				// OLED_CLS();
			}
		}

		// ȫ����ʾʱ��
		if (standby_cnt<100){
			time_disp_big();
		}

		// ��⵽��Ƶ��
		if (IC_test (&RFCARD_ID) == RFCARD_DETECED){
			standby_cnt = 0;
			unlock(0x00,2,RFCARD_ID);//��Ƶ������,��һ����������
		}

		// ��⵽ָ��
		if (QS808_Rec_Buf.Trans_state == reset){
			qs808_refresh_cnt = 30;
			QS808_Detect_Finger();
		}
		// ��˵���������������һ֡
		if(QS808_Rec_Buf.Rec_state == busy){
			qs808_refresh_cnt = 30;
			temp = QS808_Detect_Finger_Unpack();//������һ֡
			if(temp == ERR_FINGER_DETECT)
			{
				standby_cnt = 0;
				unlock(0x00,1,0x00000000);//ָ�ƽ���,��һ����������
				QS808_Rec_Buf_refresh();//�ص������棬ˢ��ָ�ƽ���buf
			}
		}

		// ɨ�谴��
		key = IsKey();
		// *
		if (key==0x0a){
			standby_cnt = 0;
			flag = 1;
		}
		// #
		else if (key == 0x0b){
			standby_cnt = 0;
			if (flag){
				setting_interface();
				QS808_Rec_Buf_refresh();//�ص������棬ˢ��ָ�ƽ���buf
			}
		}
		else if (key != NO_KEY){
			standby_cnt = 0;
			flag = 0;
			// �������
			unlock(key,0,0x00000000);
			// unlock_interface(key);  ����������պ���д��������� unlock ������
		}


	}
}


/******************************************* ������� *******************************************/
// �����ܡ�������
// key		��ʾ���µİ���
// source	��ʾ����Դ��0��Ӧ���� 1��Ӧָ�� 2��Ӧ��Ƶ
// RFCARD_ID ��rf����id ��������ģʽ������޹�
uint8_t unlock(uint8_t key,uint8_t source,uint32_t RFCARD_ID){
	uint8_t temp_u8;
	uint8_t fingerID = 0x00;
	uint8_t key_length;
	uint32_t addr;
	int i;
	MY_USER my_user1;
	// ��ʼ��һ��������û����ͣ������ں���ĳ���debug
	user_type usertype_temp = error;
	// ָ�ƽ���
	if (source == 1){
		// ��qs808������ָ��
		temp_u8 = QS808_SEARCH(&fingerID);
		// ���������ָ��
		if (temp_u8 == ERR_SUCCESS){
			// ����fingerID�ж��û�����
			for (i=0; i<MY_USER_MAX_NUM; i++){
				// ��ȡ
				addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
				// ע������������ǰ���������ܹؼ�������д������my_user1Խ��,���Բ���my_user1�þֲ���������malloc���ǲ��Եġ�
				STMFLASH_Read(addr, (uint16_t*)&my_user1, MY_USER_length/2);
				// �ж��Ƿ�����Ч����
				if (my_user1.flag == 0xAA){
					if (my_user1.finger_number == fingerID){
						// �����û�����
						usertype_temp = my_user1.my_user_type;
						break;
					}
				}
			}
			// ��bug��,qs808����ָ�ƣ�stm32��û���û����Ͳ�����Ȼ����롾��ѭ����
			if (usertype_temp == error){
				Disp_sentence(0,0,"error1",1);
				while(1)
				{}
			}
			// ����Ǳ�ķ���ͻ�Ҫ����ʱ����֤
			if (usertype_temp == babysitter){
				// ������ڱ�ķ����ʱ��Σ��Ͳ�����������return VERIFF_FAIL
				if(!time_verify()){
					Disp_sentence(32,2,"ʱ�����",1);
					if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
					delay_ms(1000);
					OLED_CLS();
					return VERIFF_FAIL;
				}
			}
			// ��ʾ���Ž���
			show_clock_open_big();
			// ����
			Gate_Unlock();
			// �� flash д����ʷ����
			unlock_notes_write(UNLOCK_NOTES_ADDR, my_user1.number);
			// ����
			OLED_CLS();
			return VERIFF_SUCCESS;
		}
		// ���û��������ָ�ƣ��Ͳ�����������return VERIFF_FAIL
		else {
			Disp_sentence(32,2,"��֤ʧ��",1);
			if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
			delay_ms(1000);
			OLED_CLS();
			return VERIFF_FAIL;
		}
	}
	// �������
	else if (source == 0){
		// ���밴����׽����¼����
		// temp_u8 = Key_Cap(key,&key_length,1);
		temp_u8 = num_unlock_interface(key, &key_length,1);

		// ������� * ����������return USER_BACK
		if(temp_u8 == KEY_CANCEL){
			OLED_CLS();
			return USER_BACK;
		}
		// �����ʱ���Ͳ�����������return USER_LOGIN_TIMEOUT
		else if (temp_u8 == KEY_TIMEOUT){
			Disp_sentence(48,2,"��ʱ",1);
			delay_ms(1000);
			OLED_CLS();
			return USER_LOGIN_TIMEOUT;
		}
		// �������������Ͳ�����������return USER_BACK
		else if (temp_u8 == KEY_TOO_LONG){
			Disp_sentence(0,2,"����̫��",1);
			delay_ms(1000);
			OLED_CLS();
			return USER_BACK;
		}
		// ��������������
		else if (temp_u8 == KEY_CONFIRM){
			// ������֤����
			temp_u8 = password_verify_2unlock(key_length, &usertype_temp);
			// ����Ǳ�ķ�����ٽ���һ��ʱ����֤
			if (usertype_temp == babysitter){
				// ������ڲ��Ǳ�ķ����ʱ�䣬�Ͳ�����������return VERIFF_FAIL
				if(!time_verify()){
					Disp_sentence(32,2,"ʱ�����",1);
					if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
					delay_ms(1000);
					OLED_CLS();
					return VERIFF_FAIL;
				}
			}
			// �����֤ʧ�ܣ��Ͳ�����������return VERIFF_FAIL
			if (temp_u8 == VERIFF_FAIL){
				Disp_sentence(32,2,"��֤ʧ��",1);
				if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
				delay_ms(1000);
				OLED_CLS();
				return VERIFF_FAIL;;
			}
			// �����֤�ɹ����Ͳ�����������������return VERIFF_SUCCESS
			else{
				// ��ʾ���Ž���
				show_clock_open_big();
				// ����
				Gate_Unlock();
				// // �� flash д����ʷ����
				// unlock_notes_write(UNLOCK_NOTES_ADDR, my_user1.number);
				// ����
				OLED_CLS();
				return VERIFF_SUCCESS;
			}
		}
	}
	// ��Ƶ������
	else if (source == 2){
		for (i=0;i<MY_USER_MAX_NUM;i++){
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr, (uint16_t*)&my_user1, MY_USER_length/2);
			// ��������Ѿ�¼������û�
			if (my_user1.flag == 0xAA){
				// ��֤������Ƶ��
				if (my_user1.rfcard_id ==  RFCARD_ID){
					// ����Ǳ�ķ�����ڽ���ʱ����֤
					if (my_user1.my_user_type == babysitter){
						// ���ʱ����󣬾Ͳ�����������return VERIFF_FAIL
						if(!time_verify()){
							Disp_sentence(32,2,"ʱ�����",1);
							if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
							delay_ms(1000);
							OLED_CLS();
							return VERIFF_FAIL;
						}
						// ���ʱ����ȷ���Ͳ�����������������return VERIFF_SUCCESS
						else {
							show_clock_open_big();
							Gate_Unlock();
							OLED_CLS();
							return VERIFF_SUCCESS;
						}
					}
					// ������Ǳ�ķ����ֱ�Ӳ�����������������return VERIFF_SUCCESS
					else {
						show_clock_open_big();
						// ��������
						Gate_Unlock();
						OLED_CLS();
						return VERIFF_SUCCESS;
					}
				}
			}
		}
		// ���е����� ˵���˿�û��¼���
		Disp_sentence(32,2,"��֤ʧ��",1);
		if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
		delay_ms(1000);
		OLED_CLS();
		return VERIFF_FAIL;
	}
	// ��������е������˵����֤ʧ�ܣ�return VERIFF_FAIL
	return VERIFF_FAIL;
}
// �����桿�����뿪��
uint8_t num_unlock_interface(uint8_t first_key, uint8_t* length,uint8_t disp_or_not){
	uint8_t key;
	int i,j,k;
	*length = 0;
	char disp[9] = "        ";

	OLED_CLS();

	// ����̬����ʾ��һ���ĵ��ͼ�꣬������ȫ�ֱ��� Battery_quantity
	if (Battery_quantity<3.7)		OLED_Show_Power(0);
	else if (Battery_quantity<4.2)	OLED_Show_Power(1);
	else if (Battery_quantity<4.7)	OLED_Show_Power(2);
	else if (Battery_quantity<5.2)	OLED_Show_Power(3);
	else 							OLED_Show_Power(4);

	// ����̬��ʾ���ڶ����� �����������롯
	Disp_sentence_singleline(24,2,"����������",0);

	// ����̬��ʾ���������ĵ�ǰʱ�䣬��ʽΪ��2018-3-4 15:16
	time_disp_bottom();



	// ����̬��ʾ����������������������������ں�̨��������ָ��¼�����
	disp[0] = first_key + 48;
	for (i=0;i<30;i++){
		Key_Buffer[i] = NO_KEY;
	}
	Key_Buffer[0] = first_key;
	// ����ʾ��һ�����µ�������ţ���Ҫ�õ�ǰ�沶׽���������� first_key
	Disp_sentence_singleline(0,4,"*",0);

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
					disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
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


/******************************************* ���ð�� *******************************************/
// �����桿����֤����Ա
// func:	��Ҫ�������Ա���������ָ�ƻ�����Ƶ���������ʵ�ǹ���Ա���ǾͿ��Խ��롾���桿������ģʽ
void setting_interface(void){
	uint8_t temp;
	uint32_t user_id;
	char disp_arr[4];
	uint32_t while_cnt=0;
	user_type user_type_temp;	// ����һ�º�����ڣ�������û��
	QS808_Rec_Buf_refresh();	// ���½��棬ˢ��ָ�ƽ���buf
	while(1){
		delay_ms(WAIT_SCAN_MS);
		while_cnt++;

		// ���5s��ʱ����ô��������return
		if (while_cnt == WAIT_TIME_MS/WAIT_SCAN_MS){
			OLED_CLS();
			return;
		}
		// �������Ա����Ϊ0���ͽ������Աע�����
		if ((*(uint16_t*)admin_amount_addr&(0x00ff)) == 0){
			Disp_sentence(0,3,"��¼��һ������Ա",1);
			if(!SPEAK_flag) SPEAK_NO_ADMIN_NOW();
			delay_ms(1000);

			// ������
			temp = user_id_cap(&user_id,&user_type_temp);
			// �����ʱ���Ǿ�������return
			if (temp == USER_LOGIN_TIMEOUT){
				OLED_CLS();
				return;
			}
			// ���ѡ����ˣ��Ǿ�������return
			else if (temp ==USER_BACK){
				OLED_CLS();
				return;
			}
			// ����ظ�
			else if (temp == USER_ID_REDUP){
				Disp_sentence(32,0,"����ظ�",1);
				Disp_sentence(24,2,"����������",0);
				if(!SPEAK_flag) SPEAK_OPT_FAIL();
				delay_ms(1000);
				continue;
			}

			disp_arr[0] = user_id/100+48;
			disp_arr[1] = user_id/10%10+48;
			disp_arr[2] = user_id%10+48;
			disp_arr[3] = 0x00;
			//�����ųɹ�,��ʼ¼��
			Disp_sentence(0,0,"¼�����Ա",1);
			Disp_sentence(0,2,"���",0);
			Disp_sentence(32,2,disp_arr,0);
			Disp_sentence(0,4,"¼ָ�ƻ�������",0);
			Disp_sentence(0,6,"����ˢ��",0);
			if(!SPEAK_flag) SPEAK_CARD_FIGER_OR_PW();
			temp = user_login(admin,user_id);//�û�¼�뺯��

			// ���¼��ָ�Ƴ�ʱ���ǾͲ�����������return
			if(temp == USER_LOGIN_TIMEOUT){
				Disp_sentence(48,2,"��ʱ",1);
				delay_ms(1000);
				OLED_CLS();
				return;
			}
			// ���ָ���������á�����̫���������ظ���ԭ���µ�ʧ�ܣ��Ǿ� continue
			else if (temp == LOGIN_ERROR){
				continue;
			}
			// ���¼��ɹ����ǾͲ�����������return
			else if(temp == USER_LOGIN_SUCCESS){
				Disp_sentence(32,2,"¼��ɹ�",1);
				if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
				delay_ms(1000);
				OLED_CLS();
				return;
			}
			// ���ѡ����ˣ��Ǿ�������return
			else if (temp == USER_BACK){
				OLED_CLS();
				return;
			}
		}
		// �������Ա������Ϊ0���ͽ������Ա��֤����
		else {
			Disp_sentence(32,0,"��֤����Ա",1);
			Disp_sentence(16,2,"(ָ�ƻ�����)",0);
			show_hammer(14,0);
			if(!SPEAK_flag) SPEAK_CARD_FIGER_OR_PW();

			// ��ʼ��֤����Ա
			temp = admin_verify();

			// ���ѡ����ˣ���������return
			if (temp == USER_BACK){
				OLED_CLS();
				return;
			}
			// �����ʱ����������return
			else if (temp == USER_LOGIN_TIMEOUT){
				OLED_CLS();
				return;
			}
			// �����֤ʧ�ܣ��� continue
			else if (temp == VERIFF_FAIL)
				continue;
			// �����֤�ɹ����ͽ��롾����ģʽ�ڶ���������
			else if (temp == VERIFF_SUCCESS){
				setting_interface2();
				OLED_CLS();
				return; // �ܵ����˵���Ѿ���������ã�ֱ��reurn��������,����������
			}
		}
	}
}
// �����桿������ģʽ
// func:	���Խ���4��������ã�����Ա���á���ͨ�û����á�ϵͳ���á��������ã�
void setting_interface2 (void){
	uint8_t key;
	while(1){
		Disp_sentence(0,0,"1.����Ա����",1);
		Disp_sentence(0,2,"2.��ͨ�û�����",0);
		Disp_sentence(0,4,"3.ϵͳ����",0);
		Disp_sentence(0,6,"4.����ͳ�Ƽ���¼",0);

		key = Wait_Key();

		// �����ʱ��return
		if (key == NO_KEY){
			// Disp_sentence(48,2,"��ʱ",1);
			return;
		}
		// �������*��return
		else if (key == 0x0a){
			return;
		}
		// �������1���ͽ��롾����Ա���á�����
		else if (key == 0x01){
			admin_settings();
		}
		// �������2���ͽ��롾��ͨ�û����á�����
		else if (key == 0x02){
			nuser_settings();
		}
		// �������3���ͽ��롾ϵͳ���á�����
		else if (key == 0x03){
			system_settings();
		}
		// �������4���ͽ��롾��ķʱ�����á�����
		else if (key == 0x04){
			data_note();
		}
	}
}
// �����桿������Ա����
void admin_settings(void){
	uint8_t key;
	uint8_t temp_u8;
	uint32_t user_id;
	char disp_arr[4];
	user_type user_type_temp;//¼��ʱû�ã��޸ĺ�ɾ��ʱ���ù�����ж϶�Ӧ�ı���ǲ��ǹ���Ա
	uint8_t flag=0;//���flag���������ʱ�û����·��ؼ����������flag�����»ص�admin_settings����
	while(1)
	{
		Disp_sentence(0,0,"1.¼�����Ա",1);
		Disp_sentence(0,2,"2.�޸Ĺ���Ա",0);
		Disp_sentence(0,4,"3.ɾ������Ա",0);
		key = Wait_Key();
		if (key == NO_KEY)//�ȴ�������ʱ
		{
			Disp_sentence(48,2,"��ʱ",1);
			delay_ms(1000);
			return;
		}
		else if (key == 0x0a)//�Ǻŷ���
		{
			return;
		}
		/**********************ѡ��1 ¼�����Ա ******************************/
		else if (key == 0x01)//¼�����Ա
		{
			while(1)
				{
					while(1)//������
					{
						temp_u8 = user_id_cap(&user_id,&user_type_temp);
						if (temp_u8 == USER_LOGIN_TIMEOUT)
							return;
						else if (temp_u8 ==USER_BACK)
						{
							flag = 1;
							break;
						}
						else if (temp_u8 == USER_ID_REDUP)//����ظ�������������
						{
							Disp_sentence(25,0,"����ظ�",1);
							Disp_sentence(20,2,"����������",0);
							delay_ms(1000);
							continue;
						}
						else //��źϷ�
							break;
					}
					if (flag)
					{
						flag = 0;
						break;//��break ��ʹadmin_settings��������
					}

					disp_arr[0] = user_id/100+48;
					disp_arr[1] = user_id/10%10+48;
					disp_arr[2] = user_id%10+48;
					disp_arr[3] = 0x00;


				//�����ųɹ�,��ʼ¼��

					Disp_sentence(0,0,"¼�����Ա",1);
					Disp_sentence(0,2,"���",0);
					Disp_sentence(32,2,disp_arr,0);
					Disp_sentence(0,4,"¼ָ�ƻ�������",0);
				  Disp_sentence(0,6,"����ˢ��",0);
					if(!SPEAK_flag) SPEAK_CARD_FIGER_OR_PW();
					temp_u8 = user_login(admin,user_id);//��ʱ���Ǻ��˳� �������¼��
					if (temp_u8 == USER_LOGIN_TIMEOUT)
					{
						Disp_sentence(48,2,"��ʱ",1);
						delay_ms(1000);
						return;
					}
					else if (temp_u8 == LOGIN_ERROR)//ָ���������ã�����̫���������ظ���ԭ���µ�ʧ��
					{
						continue;
					}
					else if (temp_u8 == USER_BACK)
						return;
					else
					{
						Disp_sentence(0,0,"¼��ɹ�",1);
						Disp_sentence(0,2,"��#����¼��",0);
						Disp_sentence(0,4,"��*����",0);
						if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
						key = Wait_Key();
						if (key == NO_KEY)//�ȴ�������ʱ
						{
							Disp_sentence(48,2,"��ʱ",1);
							return;
						}
						else if (key == 0x0a)//�Ǻŷ���
						{
							break;//���ص�ǰ���棬������Ա���ý���
						}
						else if (key == 0x0b)//���ż���
							continue;
						//������ ����Ӧ
					}
				}
		}
	/***********************ѡ��2 �޸Ĺ���Ա **********************/
		else if (key == 0x02)//�޸Ĺ���Ա
		{
			while(1)
			{
				while(1)//������
				{
					temp_u8 = user_id_cap(&user_id,&user_type_temp);
					if (temp_u8 == USER_LOGIN_TIMEOUT)
						return;
					else if (temp_u8 ==USER_BACK)
					{
						flag = 1;
						break;
					}
					else if (temp_u8 != USER_ID_REDUP)//û��������
					{
						Disp_sentence(0,2,"��Ų�����",1);
						Disp_sentence(0,4,"����������",0);
					  if(!SPEAK_flag) SPEAK_OPT_FAIL();
						delay_ms(1000);
						continue;
					}
					else if(user_type_temp != admin)//�˱�Ų��ǹ���Ա
					{
						Disp_sentence(0,2,"��Ų��ǹ���Ա",1);
						Disp_sentence(0,4,"����������",0);
						if(!SPEAK_flag) SPEAK_OPT_FAIL();
						delay_ms(1000);
						continue;
					}
					else //��źϷ�
						break;
				}
				if (flag)
				{
					flag = 0;
					break;//��break ��ʹadmin_settings��������
				}



				disp_arr[0] = user_id/100+48;
				disp_arr[1] = user_id/10%10+48;
				disp_arr[2] = user_id%10+48;
				disp_arr[3] = 0x00;


				//�����ųɹ�,��ʼ�޸�
				Disp_sentence(0,0,"�޸Ĺ���Ա",1);
				Disp_sentence(0,2,"���",0);
				Disp_sentence(32,2,disp_arr,0);
				Disp_sentence(0,4,"¼ָ�ƻ�������",0);
        		Disp_sentence(0,6,"����ˢ��",0);
				if(!SPEAK_flag) SPEAK_CARD_FIGER_OR_PW();
				temp_u8 = user_modify(user_id);
				if(temp_u8 == USER_LOGIN_TIMEOUT)//��ʱ
				{
					Disp_sentence(48,2,"��ʱ",1);
					delay_ms(1000);
					OLED_CLS();
					return;
				}
				else if (temp_u8 == USER_LOGIN_SUCCESS)
				{
						Disp_sentence(0,0,"�޸ĳɹ�",1);
						Disp_sentence(0,2,"��#�����޸�",0);
						Disp_sentence(0,4,"��*����",0);
					  if(!SPEAK_flag) SPEAK_OPT_SUCCESS() ;
						key = Wait_Key();
						if (key == NO_KEY)//�ȴ�������ʱ
						{
							Disp_sentence(48,2,"��ʱ",1);
							return;
						}
						else if (key == 0x0a)//�Ǻŷ���
						{
							break;//���ص�ǰ���棬������Ա���ý���
						}
						else if (key == 0x0b)//���ż���
							continue;
						//������ ����Ӧ
				}
				else if (temp_u8 == LOGIN_ERROR)
				{
					continue;
				}
				else
					return;//�޸ĳ�ʱ�������������أ���ֱ�ӷ���
			}
		}
		/**********************����3 ɾ������Ա************************/
		else if (key == 0x03)//ɾ������Ա
		{
			while(1)
			{
				Disp_sentence(0,2,"1.�����ɾ��",1);
				Disp_sentence(0,4,"2.ȫ��ɾ��",0);
				key = Wait_Key();
				if (key == NO_KEY)//�ȴ�������ʱ
				{
					Disp_sentence(48,2,"��ʱ",1);
					delay_ms(1000);
					return;
				}
				else if (key == 0x0a)//�Ǻŷ���
				{
					break;
				}
				else if (key == 0x01)//�����ɾ��
				{

					while(1)//������
					{
						temp_u8 = user_id_cap(&user_id,&user_type_temp);
						if (temp_u8 == USER_LOGIN_TIMEOUT)
							return;
						else if (temp_u8 ==USER_BACK)
						{
							flag = 1;
							break;
						}
						else if (temp_u8 != USER_ID_REDUP)//û��������
						{
							Disp_sentence(0,2,"��Ų�����",1);
							Disp_sentence(0,4,"����������",0);
							delay_ms(1000);
							continue;
						}
						else if(user_type_temp != admin)//�˱�Ų��ǹ���Ա
						{
							Disp_sentence(0,2,"��Ų��ǹ���Ա",1);
							Disp_sentence(0,4,"����������",0);
							delay_ms(1000);
							continue;
						}
						else //��źϷ�
							break;
					}
						if (flag)
						{
							flag = 0;
							break;
						}
						disp_arr[0] = user_id/100+48;
						disp_arr[1] = user_id/10%10+48;
						disp_arr[2] = user_id%10+48;
						disp_arr[3] = 0x00;
						Disp_sentence(0,0,"ɾ��",1);
						Disp_sentence(32,0,disp_arr,0);
						Disp_sentence(0,2,"��#ȷ��",0);
						Disp_sentence(0,4,"��*����",0);
						key = Wait_Key();
						if (key == NO_KEY)//�ȴ�������ʱ
						{
							Disp_sentence(48,2,"��ʱ",1);
							delay_ms(1000);
							return;
						}
						else if (key == 0x0a)//�Ǻŷ���
						{
							break;
						}
						else if (key == 0x0b)//���ż���
						{
							user_delete(admin,user_id);//ɾ��
							Disp_sentence(0,0,"ɾ���ɹ�",1);
							Disp_sentence(0,2,"��#����ɾ��",0);
							Disp_sentence(0,4,"��*����",0);
							if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
							key = Wait_Key();
							if (key == NO_KEY)//�ȴ�������ʱ
							{
								Disp_sentence(48,2,"��ʱ",1);
								delay_ms(1000);
								return;
							}
							else if (key == 0x0a)//�Ǻŷ���
							{
								break;//���ص�ǰ���棬������Ա���ý���
							}
							else if (key == 0x0b)//���ż���
								continue;
							//������ ����Ӧ
						}
						//������ ����Ӧ
				}
				else if (key == 0x02)//ȫ��ɾ��
				{
					Disp_sentence(0,0,"ɾ��ȫ��",1);
					Disp_sentence(0,2,"��#ȷ��",0);
					Disp_sentence(0,4,"��*����",0);
					key = Wait_Key();
					if (key == NO_KEY)//�ȴ�������ʱ
					{
						Disp_sentence(48,2,"��ʱ",1);
						delay_ms(1000);
						return;
					}
					else if (key == 0x0a)//�Ǻŷ���
					{
						break;
					}
					else if (key == 0x0b)//���ż���
					{
						user_delete(admin,0xffff);
						Disp_sentence(25,2,"ɾ���ɹ�",1);
						if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
						delay_ms(1000);
						break;
					}
					//������ ����Ӧ
				}
			}
		}
	}
}
// �����桿����ͨ�û�����
void nuser_settings(void){
	uint8_t key;
	uint8_t temp_u8;
	uint32_t user_id;
	char disp_arr[4];
	user_type user_type_temp;//¼��ʱû�ã��޸ĺ�ɾ��ʱ���ù�����ж϶�Ӧ�ı���ǲ�����ͨ�û�
	user_type user_type_temp2;//��¼�û���Ҫ�����ĵ�����
	uint8_t flag=0;//���flag���������ʱ�û����·��ؼ����������flag�����»ص�admin_settings����
	while(1)
	{
		Disp_sentence(0,0,"1.¼����ͨ�û�",1);
		Disp_sentence(0,2,"2.�޸���ͨ�û�",0);
		Disp_sentence(0,4,"3.ɾ����ͨ�û�",0);
		key = Wait_Key();

		// ��ʱ��return
		if (key == NO_KEY){
			Disp_sentence(48,2,"��ʱ",1);
			delay_ms(1000);
			return;
		}
		// *��return
		else if (key == 0x0a){
			return;
		}
		// ����1�����롾¼����ͨ�û�������
		else if (key == 0x01){
			while(1)
			{
				//ѡ���û�����
				Disp_sentence(0,0,"ѡ���û�����",1);
				Disp_sentence(0,2,"1.����",0);
				Disp_sentence(0,4,"2.��ķ",0);
				while(1){
					key = Wait_Key();

					// �ȴ�������ʱ
					if (key == NO_KEY){
						Disp_sentence(48,2,"��ʱ",1);
						delay_ms(1000);
						return;
					}
					// �Ǻŷ���
					else if (key == 0x0a){
						flag = 1;
						break;
					}
					// ����
					else if (key == 0x01){
						user_type_temp2 = family;
						break;
					}
					// ��ķ
					else if (key == 0x02){
						user_type_temp2 = babysitter;
						break;
					}
					//�������͵ȴ�����
				}

				// ��� flag==1 ���Ǿ� break ������һ��˵�
				if (flag){
					flag = 0;
					break;
				}

				// ������
				while(1){
					temp_u8 = user_id_cap(&user_id,&user_type_temp);
					if (temp_u8 == USER_LOGIN_TIMEOUT){
						return;
					}
					else if (temp_u8 == USER_BACK){
						flag = 1;
						break;
					}
					// ����ظ�������������
					else if (temp_u8 == USER_ID_REDUP){
						Disp_sentence(0,2,"����ظ�",1);
						Disp_sentence(0,4,"����������",0);
						delay_ms(1000);
						continue;
					}
					// ��źϷ���break ������һ��˵�
					else {
						break;
					}
				}

				// ��� flag==1 ���Ǿ� break ������һ��˵�
				if (flag){
					flag = 0;
					break;	//������һ��
				}

				disp_arr[0] = user_id/100+48;
				disp_arr[1] = user_id/10%10+48;
				disp_arr[2] = user_id%10+48;
				disp_arr[3] = 0x00;

				// �����ųɹ�,��ʼ¼��
				Disp_sentence(0,0,"¼����ͨ�û�",1);
				Disp_sentence(0,2,"���",0);
				Disp_sentence(32,2,disp_arr,0);
				Disp_sentence(0,4,"¼ָ�ƻ�������",0);
				Disp_sentence(0,6,"����ˢ��",0);
				temp_u8 = user_login(user_type_temp2,user_id);//��ʱ���Ǻ��˳� �������¼��
				if (temp_u8 == USER_LOGIN_TIMEOUT){
					Disp_sentence(48,2,"��ʱ",1);
					delay_ms(1000);
					return;
				}
				// ָ���������ã�����̫���������ظ���ԭ���µ�ʧ��
				else if (temp_u8 == LOGIN_ERROR){
					continue;
				}
				else if (temp_u8 == USER_BACK){
					return;
				}
				else {
					Disp_sentence(0,0,"¼��ɹ�",1);
					Disp_sentence(0,2,"��#����¼��",0);
					Disp_sentence(0,4,"��*����",0);
					key = Wait_Key();
					if (key == NO_KEY)//�ȴ�������ʱ
					{
						Disp_sentence(48,2,"��ʱ",1);
						return;
					}
					else if (key == 0x0a)//�Ǻŷ���
					{
						break;//���ص�ǰ���棬������Ա���ý���
					}
					else if (key == 0x0b)//���ż���
						continue;
					//������ ����Ӧ
				}
			}
		}
		// ����2�����롾�޸���ͨ�û�������
		else if (key == 0x02){
			while(1){
				//ѡ���û�����
				Disp_sentence(0,0,"ѡ���û�����",1);
				Disp_sentence(0,2,"1.����",0);
				Disp_sentence(0,4,"2.��ķ",0);
				while(1){
					key = Wait_Key();
					if (key == NO_KEY)//�ȴ�������ʱ
					{
						Disp_sentence(48,2,"��ʱ",1);
						delay_ms(1000);
						return;
					}
					else if (key == 0x0a)//�Ǻŷ���
					{
						flag = 1;
						break;
					}
					else if (key == 0x01)//����
					{
						user_type_temp2 = family;
						break;
					}
					else if (key == 0x02)//��ķ
					{
						user_type_temp2 = babysitter;
						break;
					}
					//�������͵ȴ�����
				}
				// ������һ��˵�
				if (flag){
					flag = 0;
					break;
				}
				// ������
				while(1){
					temp_u8 = user_id_cap(&user_id,&user_type_temp);
					if (temp_u8 == USER_LOGIN_TIMEOUT)
						return;
					else if (temp_u8 ==USER_BACK)
					{
						flag = 1;
						break;
					}
					else if (temp_u8 != USER_ID_REDUP)//û��������
					{
						Disp_sentence(0,2,"��Ų�����",1);
						Disp_sentence(0,4,"����������",0);
						delay_ms(1000);
						continue;
					}
					else if(user_type_temp != user_type_temp2)//�˱�Ų���Ҫ�޸ĵ��û�����
					{
						Disp_sentence(0,2,"�û����ʹ���",1);
						Disp_sentence(0,4,"����������",0);
						delay_ms(1000);
						continue;
					}
					else //��źϷ�
						break;
				}
				if (flag){
					flag = 0;
					break;//��break ��ʹadmin_settings��������
				}

				disp_arr[0] = user_id/100+48;
				disp_arr[1] = user_id/10%10+48;
				disp_arr[2] = user_id%10+48;
				disp_arr[3] = 0x00;

				//�����ųɹ�,��ʼ�޸�
				Disp_sentence(0,0,"�޸���ͨ�û�",1);
				Disp_sentence(0,2,"���",0);
				Disp_sentence(32,2,disp_arr,0);
				Disp_sentence(0,4,"¼ָ�ƻ�������",0);
        		Disp_sentence(0,6,"����ˢ��",0);

				temp_u8 = user_modify(user_id);
				if(temp_u8 == USER_LOGIN_TIMEOUT)//��ʱ
				{
					Disp_sentence(48,2,"��ʱ",1);
					delay_ms(1000);
					OLED_CLS();
					return;
				}
				else if (temp_u8 == USER_LOGIN_SUCCESS)
				{
						Disp_sentence(0,0,"�޸ĳɹ�",1);
						Disp_sentence(0,2,"��#�����޸�",0);
						Disp_sentence(0,4,"��*����",0);
						key = Wait_Key();
						if (key == NO_KEY)//�ȴ�������ʱ
						{
							Disp_sentence(48,2,"��ʱ",1);
							return;
						}
						else if (key == 0x0a)//�Ǻŷ���
						{
							break;//���ص�ǰ���棬������Ա���ý���
						}
						else if (key == 0x0b)//���ż���
							continue;
						//������ ����Ӧ
				}
				else if (temp_u8 == LOGIN_ERROR)
				{
					continue;
				}
				else
					return;//�޸ĳ�ʱ�������������أ���ֱ�ӷ���
			}
		}
		// ����3�����롾ɾ����ͨ�û�������
		else if (key == 0x03){
			while(1){

				//ѡ���û�����
				Disp_sentence(0,0,"ѡ���û�����",1);
				Disp_sentence(0,2,"1.����",0);
				Disp_sentence(0,4,"2.��ķ",0);
				while(1){
					key = Wait_Key();
					// �ȴ�������ʱ
					if (key == NO_KEY){
						Disp_sentence(48,2,"��ʱ",1);
						delay_ms(1000);
						return;
					}
					// ���� * ���� flag ��һ
					else if (key == 0x0a){
						flag = 1;
						break;
					}
					// ����
					else if (key == 0x01){
						user_type_temp2 = family;
						break;
					}
					// ��ķ
					else if (key == 0x02){
						user_type_temp2 = babysitter;
						break;
					}
				}
				// ��� flag Ϊ1��break
				if (flag){
					flag = 0;
					// ��break ��ʹadmin_settings��������
					break;
				}
				Disp_sentence(0,2,"1.�����ɾ��",1);
				Disp_sentence(0,4,"2.ȫ��ɾ��",0);
				key = Wait_Key();
				// �ȴ�������ʱ
				if (key == NO_KEY){
					Disp_sentence(48,2,"��ʱ",1);
					delay_ms(1000);
					return;
				}
				// �Ǻŷ���
				else if (key == 0x0a){
					break;
				}
				// �����ɾ��
				else if (key == 0x01){

					while(1)//������
					{
						temp_u8 = user_id_cap(&user_id,&user_type_temp);
						if (temp_u8 == USER_LOGIN_TIMEOUT)
							return;
						else if (temp_u8 ==USER_BACK)
						{
							flag = 1;
							break;
						}
						else if (temp_u8 != USER_ID_REDUP)//û��������
						{
							Disp_sentence(24,0,"��Ų�����",1);
							Disp_sentence(24,2,"����������",0);
							delay_ms(1000);
							continue;
						}
						else if(user_type_temp != user_type_temp2)
						{
							Disp_sentence(24,0,"�û����ʹ���",1);
							Disp_sentence(24,2,"����������",0);
							delay_ms(1000);
							continue;
						}
						else //��źϷ�
							break;
					}
					if (flag){
						flag = 0;
						break;
					}
					disp_arr[0] = user_id/100+48;
					disp_arr[1] = user_id/10%10+48;
					disp_arr[2] = user_id%10+48;
					disp_arr[3] = 0x00;
					Disp_sentence(0,0,"ɾ��",1);
					Disp_sentence(32,0,disp_arr,0);
					Disp_sentence(0,2,"��#ȷ��",0);
					Disp_sentence(0,4,"��*����",0);
					key = Wait_Key();
					// �ȴ�������ʱ
					if (key == NO_KEY){
						Disp_sentence(48,2,"��ʱ",1);
						delay_ms(1000);
						return;
					}
					// * ����
					else if (key == 0x0a){
						break;
					}
					// # ����
					else if (key == 0x0b){
						user_delete(user_type_temp2,user_id);//ɾ��
						Disp_sentence(0,0,"ɾ���ɹ�",1);
						Disp_sentence(0,2,"��#����ɾ��",0);
						Disp_sentence(0,4,"��*����",0);
						key = Wait_Key();
						// �ȴ�������ʱ
						if (key == NO_KEY){
							Disp_sentence(48,2,"��ʱ",1);
							delay_ms(1000);
							return;
						}
						// �Ǻŷ���
						else if (key == 0x0a){
							break;
						}
						// ���ż���
						else if (key == 0x0b)
							continue;
					}
				}
				// ȫ��ɾ��
				else if (key == 0x02){
					Disp_sentence(0,0,"ɾ��ȫ��",1);
					Disp_sentence(0,2,"��#ȷ��",0);
					Disp_sentence(0,4,"��*����",0);
					key = Wait_Key();
					if (key == NO_KEY)//�ȴ�������ʱ
					{
						Disp_sentence(48,2,"��ʱ",1);
						delay_ms(1000);
						return;
					}
					else if (key == 0x0a)//�Ǻŷ���
					{
						break;
					}
					else if (key == 0x0b)//���ż���
					{
						user_delete(user_type_temp2,0xffff);
						Disp_sentence(32,2,"ɾ���ɹ�",1);
						delay_ms(1000);
						break;
					}
					//������ ����Ӧ
				}
			}
		}
	}
}
// �����桿��ϵͳ����
void system_settings(void){
	uint8_t u8_temp;
	while(1){
		Disp_sentence(0,0,"1.ϵͳʱ������",1);
		Disp_sentence(0,2,"2.��������",0);
		Disp_sentence(0,4,"3.�ָ���������",0);
		Disp_sentence(0,6,"4.��ķʱ������",0);
		uint8_t key;
		key = Wait_Key();

		// ��ʱ��return
		if (key == NO_KEY){
			// Disp_sentence(48,2,"��ʱ",1);
			return;
		}
		// *��return
		else if (key == 0x0a){
			return;
		}
		// ����1�����롾ʱ�����á�����
		else if (key == 0x01){
			// ���롾ϵͳʱ�����ý��桿�������� temp
			u8_temp = time_settings();
			if (u8_temp == USER_LOGIN_TIMEOUT)
				return;
			else if (u8_temp == USER_CONFIRM)
			{
				Disp_sentence(25,2,"���óɹ�",1);
				if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
				delay_ms(1000);
				continue;
			}
		}
		// ����2�����롾�������á�����
		else if (key == 0x02){
			Disp_sentence(0,0,"1.������",1);
			Disp_sentence(0,2,"2.�ر�����",0);
			key = Wait_Key();
			if (key == NO_KEY)//�ȴ�������ʱ
			{
				Disp_sentence(48,2,"��ʱ",1);
				delay_ms(1000);
				return;
			}
			else if (key == 0x0a)//�Ǻŷ���
			{
				break;
			}
			else if (key == 0x01)//������
			{
				SPEAK_flag=0;
				STMFLASH_Write(speak_flag_addr,&SPEAK_flag,1);//������
				Disp_sentence(25,2,"���óɹ�",1);
				if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
				delay_ms(1000);
				break;
			}
			else if (key == 0x02)//�ر�����
			{
				//���༭
				SPEAK_flag=1;
				STMFLASH_Write(speak_flag_addr,&SPEAK_flag,1);//������
				Disp_sentence(25,2,"���óɹ�",1);
				if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
				delay_ms(1000);
				break;
			}
		}
		// ����3�����롾�ָ��������á�����
		else if (key == 0x03){
				Disp_sentence(0,0,"ȷ�ϻָ���������",1);
				Disp_sentence(0,2,"��#ȷ��",0);
				Disp_sentence(0,4,"��*����",0);
				key = Wait_Key();
				if (key == NO_KEY)//�ȴ�������ʱ
				{
					Disp_sentence(48,2,"��ʱ",1);
					delay_ms(1000);
					return;
				}
				else if (key == 0x0a)//�Ǻŷ���
				{
					break;
				}
				else if (key == 0x0b)//���ż���
				{
					back2factory();//��������
					Disp_sentence(0,2,"�ָ��������óɹ�",1);
					if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
					delay_ms(1000);
					NVIC_SystemReset();
				}

		}
		// ����4�����롾��ķʱ�����á�
		else if (key == 0x04){
			nurse_time_settings();
		}
	}
}
// �����桿����ķʱ������
void nurse_time_settings(void){
	uint8_t u8_temp;
	u8_temp = unlock_time_fun();
	if (u8_temp == USER_CONFIRM){
		Disp_sentence(25,2,"���óɹ�",1);
		if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
		delay_ms(1000);
	}
}
// �����桿������ͳ�Ƽ���¼
void data_note(void){
	uint8_t key,pos=0;

	unlock_notes_disp(0);

	while (1){
		key = Wait_Key();

		// �������4���ͳ��Բ鿴��4����ʷ��¼������ͷ�˾Ͳ���Ӧ
		if (key == 0x04){
			// ˢ����ʾ����
			if (pos>0){
				pos--;
				// ��ʼ��ʾ
				unlock_notes_disp(pos);
			}
		}
		// �������6���ͳ��Բ鿴��4����ʷ��¼������ͷ�˾Ͳ���Ӧ
		else if (key == 0x06){
			// ˢ����ʾ����
			if (pos < ((UNLOCK_NOTES_SIZE/4)+((UNLOCK_NOTES_SIZE%4)>0))-1 ){
				pos++;
				// ��ʼ��ʾ
				unlock_notes_disp(pos);
			}
		}
		// �������*��return
		else if (key == 0x0a){
			return;
		}
		// �����ʱ��return
		else if (key == NO_KEY){
			Disp_sentence(48,2,"��ʱ",1);
			return;
		}
	}
}



/******************************************* �ײ㺯�� *******************************************/
// func:	�� flash ָ��λ��д�� ID �Լ� ʱ������
// addr:	��ʷ��¼���׵�ַ�������Ǹ��̶���ȫ�ֱ��� UNLOCK_NOTES_ADDR
// user_id:	д����û����
void unlock_notes_write(u32 addr, uint16_t user_id){
	UNLOCK_NOTES notes;
	UNLOCK_NOTES_AREA notes_area;

	// ��ȡ���µ���ʷ��¼��Ϣ
	notes.flag = UNLOCK_NOTES_USED_FLAG;
	notes.year = 	calendar.w_year;
	notes.month_date = (calendar.w_date<<8) | calendar.w_month;
	notes.hour_minute = (calendar.min<<8) | calendar.hour;
	notes.user_number = user_id;

	// ���·�������
	STMFLASH_Read(UNLOCK_NOTES_ADDR, (u16*)&notes_area, UNLOCK_NOTES_MSG_LENGTH*(UNLOCK_NOTES_SIZE-1));
	STMFLASH_Write(UNLOCK_NOTES_ADDR+2*UNLOCK_NOTES_MSG_LENGTH, (u16*)&notes_area, UNLOCK_NOTES_MSG_LENGTH*(UNLOCK_NOTES_SIZE-1));

	// �ڿ�ͷ�������µ���ʷ����
	STMFLASH_Write(UNLOCK_NOTES_ADDR, (u16*)&notes, UNLOCK_NOTES_MSG_LENGTH);
}

// func:	��ʾ4����ʷ��¼��Ϣ
// pos:		ÿ������ʷ��¼Ϊһ��pos���ڡ�1234������ʷ��¼��Ӧ��pos=0
void unlock_notes_disp(u8 pos){
	char disp1[16];

	u32 addr;
	u16 flag,id;
	u8 month,date,hour,min;

	for (u8 i=0; i<4; i++){
		addr	= UNLOCK_NOTES_ADDR + pos*40 + i*10;
		flag 	= *(u16*)(addr);
		// year 	= *(u16*)(addr+2);
		month 	= *(u8*)(addr+4);
		date 	= *(u8*)(addr+5);
		hour 	= *(u8*)(addr+6);
		min 	= *(u8*)(addr+7);
		id 		= *(u16*)(addr+8);

		// ���������Ϣ����ʵ�ġ���ʷ��¼������ô�Ϳ�ʼ��ӡ
		if (flag == 0x6666){
			// ��ʽ�磺02-28
			disp1[0] = 48 + (month)/10;
			disp1[1] = 48 + (month)%10;
			disp1[2] = '-';
			disp1[3] = 48 + (date)/10;
			disp1[4] = 48 + (date)%10;
			disp1[5] = ' ';
			// 18:31
			disp1[6] = 48 + (hour)/10;
			disp1[7] = 48 + (hour)%10;
			disp1[8] = ':';
			disp1[9] = 48 + (min)/10;
			disp1[10] = 48 + (min)%10;
			disp1[11] = ' ';
			// User Number
			disp1[12] = 48 + (id)/100;
			disp1[13] = 48 + (id/10)%10;
			disp1[14] = 48 + (id)%10;
			// ���������
			disp1[15] = 0;

			// ��ӡ�� OLED ��
			Disp_sentence_singleline(0,i*2,disp1,1);
		}
		else {
			Disp_sentence_singleline(0,i*2,"��",1);
		}
	}
}

// func:	������е���ʷ��¼����������ʷ���ݾ���Ϊ FF
void unlock_notes_set_all_ff(void){
	for (u16 i=0; i<UNLOCK_NOTES_SIZE; i++){
		for (u8 j=0; j<UNLOCK_NOTES_MSG_LENGTH; j++){
			Test_Write(UNLOCK_NOTES_ADDR+i*UNLOCK_NOTES_MSG_LENGTH*2+j*2, 0xFFFF);
		}
	}
}

// func:	�������е���ʷ��¼������������ʷ���ݵı�־λ����Ϊ 0xFF
void unlock_notes_set_all_flag_ff(void){
	for (u16 i=0; i<UNLOCK_NOTES_SIZE; i++){
		Test_Write(UNLOCK_NOTES_ADDR+i*UNLOCK_NOTES_MSG_LENGTH*2, 0xFFFF);
	}
}

// ������
// my_user_type:�û�����
// user_num���û����
// ����ֵ��
// USER_LOGIN_TIMEOUT ��ʱ
// USER_LOGIN_SUCCESS ¼��ɹ�
uint8_t user_login(user_type my_user_type,uint16_t user_num){
	uint8_t temp_u8 = 0x00;
	uint8_t key;
	uint8_t ID;
	uint16_t user_amount;
	uint8_t key_length;
	int i;
	int n=0;
	MY_USER user_temp;
	uint8_t rfcard;
	uint32_t RFCARD_ID;

	// ��ʼ�� user_temp
	user_temp.password_length=0;//�����г�ʼ������Ҫ��֮��ĳ��򣬿���ͨ��ָ��id�����볤���ж�����û��Ƿ����ָ�ƻ������롣��������¼��ɹ�ʱ����������
	user_temp.finger_number=0;
	user_temp.rfcard_id = 0x00000000;
	QS808_Rec_Buf_refresh();
	while(1)
	{
		n++;
		if(n>WAIT_TIME_MS/90)
				return USER_LOGIN_TIMEOUT;
		if (QS808_Rec_Buf.Trans_state == reset)
		{
			QS808_Detect_Finger();
		}
		if(QS808_Rec_Buf.Rec_state == busy)//��˵���������������һ֡
		{
			uint8_t temp = QS808_Detect_Finger_Unpack();//������һ֡
			if(temp == ERR_FINGER_DETECT)
			{
				temp_u8 = QS808_Login(&ID);		//��ȡָ�Ƶ�id�����ҷ���
				switch(temp_u8)
				{
					case QS808_WAIT_TIMEOUT:	//�ȴ���ʱ���������˵�
					{
							return USER_LOGIN_TIMEOUT;
					}
					case ERR_BAD_QUALITY:		//ָ�����������¼��
					{
							Disp_sentence(20,2,"ָ��������",1);
							Disp_sentence(20,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							 OLED_CLS();
							return LOGIN_ERROR;
					}
					case ERR_MERGE_FAIL:		//ָ�ƺϳ�ʧ�ܣ�����¼��
					{
							Disp_sentence(20,2,"¼��ʧ��",1);
							Disp_sentence(20,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							 OLED_CLS();
							return LOGIN_ERROR;
					}
					case ERR_DUPLICATION_ID:	//�Ѵ��ڴ���ָ������¼��
					{
							Disp_sentence(20,2,"ָ���Ѵ���",1);
							Disp_sentence(20,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();
							return LOGIN_ERROR;
					}
					case QS808_DATA_ERR:
					{
							Disp_sentence(0,2,"¼��ʧ��",1);
							Disp_sentence(0,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//����
							return LOGIN_ERROR;
					}
					case ERR_SUCCESS:			//�ɹ������Ҵ洢����û�
					{
						user_temp.flag = user_flag_value;
						user_temp.number = user_num;
						user_temp.password_length = 0;
						user_temp.finger_number = ID;
						user_temp.my_user_type = my_user_type;
						user_struct_store(user_num,(uint16_t *) &user_temp);

						//�����û�����
						if(my_user_type == admin)//����Ա
						{
							user_amount = *(vu16*)admin_amount_addr;
							user_amount++;
							STMFLASH_Write(admin_amount_addr,&user_amount,1);
						}
						else if(my_user_type == family)//����
						{
								user_amount = *(vu16*)family_amount_addr;
								user_amount++;
								STMFLASH_Write(family_amount_addr,&user_amount,1);
						}
						else if (my_user_type == babysitter)//��ķ
						{
								user_amount = *(vu16*)babysitter_amount_addr;
								user_amount++;
								STMFLASH_Write(babysitter_amount_addr,&user_amount,1);
						}
						user_amount = *(vu16*)alluser_amount_addr;//�û�������1
						user_amount++;
						STMFLASH_Write(alluser_amount_addr,&user_amount,1);
						return USER_LOGIN_SUCCESS;
					}
					default:
					{
							Disp_sentence(0,2,"¼��ʧ��",1);
							Disp_sentence(0,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//����
							return LOGIN_ERROR;
					}
				}
			}
		}


		//ɨ����Ƶ��
		////////////////////////////////////////////////////////////////////��Ƶ���ظ��Լ��û��
		rfcard = IC_test (&RFCARD_ID);
		if (rfcard == RFCARD_DETECED)
		{
			user_temp.flag = user_flag_value;
			user_temp.my_user_type = my_user_type;
			user_temp.number = user_num;
			user_temp.rfcard_id = RFCARD_ID;
			user_struct_store(user_num,(uint16_t *) &user_temp);
			//�����û�����
			if(my_user_type == admin)//����Ա
			{
				user_amount = *(vu16*)admin_amount_addr;
				user_amount++;
				STMFLASH_Write(admin_amount_addr,&user_amount,1);
			}
			else if(my_user_type == family)//����
			{
				user_amount = *(vu16*)family_amount_addr;
				user_amount++;
				STMFLASH_Write(family_amount_addr,&user_amount,1);
			}
			else if (my_user_type == babysitter)//��ķ
			{
				user_amount = *(vu16*)babysitter_amount_addr;
				user_amount++;
				STMFLASH_Write(babysitter_amount_addr,&user_amount,1);
			}
			user_amount = *(vu16*)alluser_amount_addr;//�û�������1
			user_amount++;
			STMFLASH_Write(alluser_amount_addr,&user_amount,1);
	//				Disp_sentence(0,2,"¼��ɹ�",1);
			return USER_LOGIN_SUCCESS;
		}
		//ɨ�谴��
		key = IsKey();//ʱ�䲻��
		if (key == NO_KEY)
		{
			continue;
		}
		else if (key == 0x0a)//�Ǻ�,����
		{
	//		printf("����\n");
			return USER_BACK;
		}
		else if (key == 0x0b)//���ţ�����Ӧ
		{
			continue;
		}
		else
		{
			temp_u8 = Key_Cap_6bits(key,&key_length,0);//���밴����׽����¼����														/////////////////////////
			if(temp_u8 == KEY_CANCEL)
			{
	//			printf("����\n");
				return USER_BACK;
			}
			else if (temp_u8 == KEY_TIMEOUT)
			{
	//					Disp_sentence(0,2,"��ʱ",1);
				return USER_LOGIN_TIMEOUT;
			}
			else if (temp_u8 == KEY_TOO_LONG)//�������
			{
				Disp_sentence(25,2,"����̫��",1);
				Disp_sentence(18,4,"������¼��",0);
				delay_ms(1000);
				return LOGIN_ERROR;
			}
			else if (temp_u8 == KEY_CONFIRM)//�����������
			{
				//����Ƿ��Ѿ��д����룬�����û���������ͬ�����룬��Ϊ����ķ�ͼ���������ͬʱ�����±�ķ��ʱ��ʧЧ
				user_type user_type1;
				temp_u8 = password_verify_2unlock(key_length,&user_type1);
				if (temp_u8 == VERIFF_SUCCESS)//˵�������Ѿ�����
				{
					Disp_sentence(25,2,"�����Ѵ���",1);
					Disp_sentence(25,4,"������¼��",0);
					delay_ms(1000);
					return LOGIN_ERROR;
				}

				user_temp.flag = user_flag_value;
				user_temp.my_user_type = my_user_type;
				user_temp.number = user_num;
				user_temp.password_length = key_length;
				for (i=0;i<key_length;i++)
					user_temp.password[i] = Key_Buffer[i];
				user_temp.finger_number = 0;
				user_struct_store(user_num,(uint16_t *) &user_temp);
				//�����û�����
				if(my_user_type == admin)//����Ա
				{
					user_amount = *(vu16*)admin_amount_addr;
					user_amount++;
					STMFLASH_Write(admin_amount_addr,&user_amount,1);
				}
				else if(my_user_type == family)//����
				{
					user_amount = *(vu16*)family_amount_addr;
					user_amount++;
					STMFLASH_Write(family_amount_addr,&user_amount,1);
				}
				else if (my_user_type == babysitter)//��ķ
				{
					user_amount = *(vu16*)babysitter_amount_addr;
					user_amount++;
					STMFLASH_Write(babysitter_amount_addr,&user_amount,1);
				}
				user_amount = *(vu16*)alluser_amount_addr;//�û�������1
				user_amount++;
				STMFLASH_Write(alluser_amount_addr,&user_amount,1);
	//				Disp_sentence(0,2,"¼��ɹ�",1);
				return USER_LOGIN_SUCCESS;

			}
		}

	}

}

//my_user_type:�û�����
//user_num���û����
//����ֵ��
//USER_LOGIN_TIMEOUT ��ʱ
//USER_LOGIN_SUCCESS �޸ĳɹ�
uint8_t user_modify(uint16_t user_num){
	uint8_t temp_u8 = 0x00;
	uint8_t key;
	uint8_t ID;
	uint8_t ID2;//�����޸�֮ǰ��ָ��ID
	uint8_t key_length;
	int i;
	int n=0;
	MY_USER user_temp;
	uint32_t addr;
	uint8_t rfcard;
	uint32_t RFCARD_ID;
	// printf("����ָ����������\n");
	// �Ȱ��û��ṹ�建�����
	for (i=0;i<MY_USER_MAX_NUM;i++)
	{
		addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
		STMFLASH_Read(addr,(uint16_t*)&user_temp,MY_USER_length/2); //ע������������ǰ���������ܹؼ�
		if (user_temp.flag == 0xAA)//���û�
			if(user_temp.number == user_num)//�ҵ��˵�ǰ���
				break;//������������user_temp�Ͻ����޸ģ�Ȼ�󱣴漴��
			//���ﲻ���Ǳ�Ų����ڵ����⣬�����������һ���������
	}
	ID2 = user_temp.finger_number;
	QS808_Rec_Buf_refresh();
	while(1)
	{
		n++;
		if(n>WAIT_TIME_MS/90)
				return USER_LOGIN_TIMEOUT;
		if (QS808_Rec_Buf.Trans_state == reset)
		{
			QS808_Rec_Buf.Trans_state = set;
			QS808_Detect_Finger();
		}
		if(QS808_Rec_Buf.Rec_state == busy)//��˵���������������һ֡
		{
				uint8_t temp = QS808_Detect_Finger_Unpack();//������һ֡
				if(temp == ERR_FINGER_DETECT)
				{
					temp_u8 = QS808_Login(&ID);//����ָ��¼��ʱ�������Ϣ���磺ָ���������ѣ��ȴ���ʱ�ȵ�
					switch(temp_u8)
					{
						case QS808_WAIT_TIMEOUT://�ȴ���ʱ���������˵���return USER_LOGIN_TIMEOUT;
						{
							return USER_LOGIN_TIMEOUT;
						}
						case ERR_BAD_QUALITY:	//ָ�����������¼�룬return LOGIN_ERROR;
						{
							Disp_sentence(0,2,"ָ��������",1);
							Disp_sentence(0,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();
							return LOGIN_ERROR;
						}
						case ERR_MERGE_FAIL:	//ָ�ƺϳ�ʧ�ܣ�����¼�룬return LOGIN_ERROR;
						{
							Disp_sentence(0,2,"¼��ʧ��",1);
							Disp_sentence(0,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//����
							return LOGIN_ERROR;
						}
						case ERR_DUPLICATION_ID://�Ѵ��ڴ���ָ������¼��return LOGIN_ERROR;
						{
							Disp_sentence(0,2,"ָ���Ѵ���",1);
							Disp_sentence(0,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();
							return LOGIN_ERROR;
						}
						case QS808_DATA_ERR:
						{
							Disp_sentence(0,2,"¼��ʧ��",1);
							Disp_sentence(0,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//����
							return LOGIN_ERROR;
						}
						case ERR_SUCCESS://�ɹ����洢�û���return USER_LOGIN_SUCCESS;
						{
							user_temp.finger_number = ID;//ֻ�޸�ָ��ID ��������
							STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//�ṹ�屣��

							//���ø����û�����
							//��QS808��ɾ��ԭ��ָ��
							if (ID2!=0)//��˵������û�֮ǰ��ָ�ƣ���Ҫ��qs808��ɾ������û���ָ��
								QS808_CMD_DEL_NUM2(ID2);
							return USER_LOGIN_SUCCESS;
						}
						default:
						{
							Disp_sentence(0,2,"¼��ʧ��",1);
							Disp_sentence(0,4,"������¼��",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//����
							return LOGIN_ERROR;
						}
					}
				}
		}
		//ɨ����Ƶ��
		uint8_t rfcard_repeat_flag=0;
		rfcard = IC_test (&RFCARD_ID);
		if (rfcard == RFCARD_DETECED)
		{
			//����Ƿ��Ѿ�����Ƶ��
			MY_USER my_user1;
			for (i=0;i<MY_USER_MAX_NUM;i++)
			{
				addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
				STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2);
				if (my_user1.flag == 0xAA)//�ж��Ƿ�����Ч����
				{
					if (my_user1.rfcard_id == RFCARD_ID)
						rfcard_repeat_flag = 1;
				}
			}
			if (rfcard_repeat_flag)//�ظ�
			{
				Disp_sentence(0,2,"���Ѵ���",1);
				Disp_sentence(0,4,"������¼��",0);
				if(!SPEAK_flag) SPEAK_OPT_FAIL();
				delay_ms(1000);
				return LOGIN_ERROR;
			}
			user_temp.rfcard_id = RFCARD_ID;//�޸����볤�ȣ�����������
			STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//�ṹ�屣��
			return USER_LOGIN_SUCCESS;
		}
		//��������
		key = IsKey();//ʱ�䲻��
		if (key == NO_KEY)
		{
			continue;
		}
		else if (key == 0x0a)//�Ǻ�,����
		{
	//		printf("����\n");
			return USER_BACK;
		}
		else if (key == 0x0b)//���ţ�����Ӧ
		{
			continue;
		}
		else
		{
			temp_u8 = Key_Cap_6bits(key,&key_length,0);//���밴����׽����¼����
			if(temp_u8 == KEY_CANCEL)
			{
	//			printf("����\n");
				return USER_BACK;
			}
			else if (temp_u8 == KEY_TIMEOUT)
			{
	//					Disp_sentence(0,2,"��ʱ",1);
				return USER_LOGIN_TIMEOUT;
			}
			else if (temp_u8 == KEY_TOO_LONG)//�������
			{
				Disp_sentence(0,2,"����̫��",1);
				Disp_sentence(0,4,"������¼��",0);
				if(!SPEAK_flag) SPEAK_OPT_FAIL();
				delay_ms(1000);
				return LOGIN_ERROR;
			}
			else if (temp_u8 == KEY_CONFIRM)//�����������
			{
				//����Ƿ��Ѿ��д����룬�����û���������ͬ�����룬��Ϊ����ķ�ͼ���������ͬʱ�����±�ķ��ʱ��ʧЧ
				user_type user_type1;
				temp_u8 = password_verify_2unlock(key_length,&user_type1);
				if (temp_u8 == VERIFF_SUCCESS)//˵�������Ѿ�����
				{
					Disp_sentence(0,2,"�����Ѵ���",1);
					Disp_sentence(0,4,"������¼��",0);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					delay_ms(1000);
					return LOGIN_ERROR;
				}
				user_temp.password_length = key_length;//�޸����볤�ȣ�����������
				for (i=0;i<key_length;i++)
					user_temp.password[i] = Key_Buffer[i];
				STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//�ṹ�屣��
				return USER_LOGIN_SUCCESS;
			}
		}

	}
}

// �����ܡ�ɾ���û�
void user_delete(user_type my_user_type,uint16_t user_num){
	//��user_numΪ0xFFFFʱ��ɾ��ȫ�� my_user_type �����û� ����ɾ�����Ϊuser_num���û�
	MY_USER user_temp;
	uint32_t addr;
	uint16_t user_amount,user_amount2;//�û�����
	int i;
	if (user_num == 0xffff)//ȫ��ɾ��
	{
		for (i=0;i<MY_USER_MAX_NUM;i++)
		{
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr,(uint16_t*)&user_temp,MY_USER_length/2); //ע������������ǰ���������ܹؼ�
			if (user_temp.flag == 0xAA)
			{
				if (user_temp.my_user_type == my_user_type)
				{
					if(user_temp.finger_number != 0x00)//˵���û���ָ�ƣ�Ҫ��qs808��ɾ��
					{
						QS808_CMD_DEL_NUM2(user_temp.finger_number);
					}
					user_temp.flag = 0x00;//��ʵֻ��Ҫflag��������ˣ��������Է���һ
					user_temp.number = 0;
					user_temp.finger_number = 0;
					user_temp.password_length = 0;
					user_temp.rfcard_id = 0x00000000;
					STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//�ṹ�屣��

					//�û������޸�
					if (my_user_type == admin)
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//����
						user_amount2 =  *(uint16_t*)admin_amount_addr;//����Ա��
						user_amount = user_amount - user_amount2;//������
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//��������
						user_amount = 0;//�¹���Ա��
						STMFLASH_Write((uint32_t)admin_amount_addr,(uint16_t*)&user_amount,1);
					}
					else if (my_user_type == family)//�������� ���༭
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//����
						user_amount2 =  *(uint16_t*)family_amount_addr;
						user_amount = user_amount - user_amount2;//������
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//��������
						user_amount = 0;
						STMFLASH_Write((uint32_t)family_amount_addr,(uint16_t*)&user_amount,1);
					}
					else if (my_user_type == babysitter)
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//����
						user_amount2 =  *(uint16_t*)babysitter_amount_addr;
						user_amount = user_amount - user_amount2;//������
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//��������
						user_amount = 0;
						STMFLASH_Write((uint32_t)babysitter_amount_addr,(uint16_t*)&user_amount,1);
					}

				}
			}
		}

	}
	else
	{
		for (i=0;i<MY_USER_MAX_NUM;i++)
		{
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr,(uint16_t*)&user_temp,MY_USER_length/2); //ע������������ǰ���������ܹؼ�
			if (user_temp.flag == 0xAA)
			{
				if (user_temp.number == user_num)
				{
					if(user_temp.finger_number != 0x00)//˵���û���ָ�ƣ�Ҫ��qs808��ɾ��
					{
						QS808_CMD_DEL_NUM2(user_temp.finger_number);
					}
					user_temp.flag = 0x00;//��ʵֻ��Ҫflag��������ˣ������������Է���һ
					user_temp.number = 0;
					user_temp.finger_number = 0;
					user_temp.password_length = 0;
					user_temp.rfcard_id = 0x00000000;
					STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//�ṹ�屣��
					//����-1
					if (my_user_type == admin)
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//����
						user_amount2 =  *(uint16_t*)admin_amount_addr;//����Ա��
						user_amount--;
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//��������
						user_amount2--;//�¹���Ա��
						STMFLASH_Write((uint32_t)admin_amount_addr,(uint16_t*)&user_amount2,1);
					}
					else if (my_user_type == family)//�������� ���༭
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//����
						user_amount2 =  *(uint16_t*)family_amount_addr;//
						user_amount--;
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//��������
						user_amount2--;//
						STMFLASH_Write((uint32_t)family_amount_addr,(uint16_t*)&user_amount2,1);
					}
					else if (my_user_type == babysitter)
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//����
						user_amount2 =  *(uint16_t*)babysitter_amount_addr;//
						user_amount--;
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//��������
						user_amount2--;
						STMFLASH_Write((uint32_t)babysitter_amount_addr,(uint16_t*)&user_amount2,1);
					}

				}
			}
		}
	}


}

void user_struct_store(uint16_t user_num,uint16_t * user_struct){
	//�洢��ʽ���������洢�����û��ṹ��洢��ַ��һ����ʼ�������ҵ����Դ��λ�ã�ֱ�Ӵ洢
	//���ﲻ����flash�洢���µ����⣬�����ĺ����������
	int i;
	MY_USER my_user1;
	uint32_t addr;//flash��ַ
	for (i=0;i<MY_USER_MAX_NUM;i++)
	{
		addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
		STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2); //ע������������ǰ���������ܹؼ�������д������my_user1Խ��
		if (my_user1.flag != 0xAA)//���λ��û�д洢�û������Դ�������
			break;
	}
	STMFLASH_Write(addr,user_struct,MY_USER_length/2);
}

//����ʹ�ó�ʼ����������
//1.����Ƿ��ǳ���ʹ�ã�INIT_WORD��
//2.��ʼ���û�����user_amount_addr��user_amount_addr��
int first_time_init(void){
	UNLOCK_TIME unlocktime;
	uint16_t init_word0;
	uint16_t user_amount = 0;//��ʼ��Ϊ0���û�
	uint16_t initword1 = INIT_WORD;
	init_word0 = *(uint16_t*)INIT_ADDR;
	if (init_word0 == INIT_WORD)//���ǵ�һ��ʹ�ã���ȡ��Ƶ���ã�Ȼ��return 0
	{
		STMFLASH_Read(speak_flag_addr,&SPEAK_flag,1);
		return 0;
	}
	//��һ��ʹ��
	SPEAK_flag=0;
	STMFLASH_Write(INIT_ADDR,&initword1,1);	//��һ��ʹ�ã�д��ؼ���
	STMFLASH_Write(admin_amount_addr,&user_amount,1);	//��һ��ʹ�ã����û�������ʼ��Ϊ0
	STMFLASH_Write(family_amount_addr,&user_amount,1);	//��һ��ʹ�ã����û�������ʼ��Ϊ0
	STMFLASH_Write(babysitter_amount_addr,&user_amount,1);
	STMFLASH_Write(alluser_amount_addr,&user_amount,1);
	STMFLASH_Write(speak_flag_addr,&SPEAK_flag,1);//��ʼ��Ϊ������


	unlocktime.hour = 0;
	unlocktime.hour2 = 23;
	unlocktime.minute = 0;
	unlocktime.minute2 = 59;

	STMFLASH_Write(UNLOCK_TIME_addr,(uint16_t*)&unlocktime,UNLOCK_TIME_length/2);

	uint32_t temp;
	temp = sizeof(MY_USER);//����һ�½ṹ�峤�� ����128�Ļ� ��ȥ�Ľṹ��
	if (temp != MY_USER_length)//�û��ṹ�峤�Ȳ���
	{
		Disp_sentence(0,0,"error!",1);
		while(1);
	}

	return 1;

}

// �жϵ�ǰʱ���Ƿ����ڿɿ���ʱ�Σ����ڱ�ķ�û�
uint8_t time_verify(void){
	//����ֵ 0 ���ܿ��� 1 ���Կ���
	UNLOCK_TIME unlocktime;
	u8 hour_now = calendar.hour;
	u8 min_now = calendar.min;

	STMFLASH_Read(UNLOCK_TIME_addr,(uint16_t*)&unlocktime,UNLOCK_TIME_length/2);
	uint32_t time1 = unlocktime.hour*60+unlocktime.minute;//��ʱ�任�㵽���ӣ��������ж�
	uint32_t time2 = unlocktime.hour2*60+unlocktime.minute2;
	uint32_t time_now = hour_now*60+min_now;

	// ���������Ԥ��ʱ�����޴�������
	if (time2 >= time1){
		if ((time_now>=time1) && (time_now<=time2))
			return 1;
		else
			return 0;
	}
	// Ԥ��ʱ������С�����ޣ�˵������ʱ������0��
	else {
		if (((time_now>=time1)&&(time_now<=1439))||(time_now<=time2))//1439=23*60+59
			return 1;
		else
			return 0;
	}
}

// ������ʱ�����á�����
uint8_t unlock_time_fun(void){
	uint8_t key;
	u8 hour1,min1;
	u8 hour2,min2;
	int i=0;
	int j=0;
	UNLOCK_TIME unlocktime;
	Disp_sentence(0,0,"�����뿪��ʱ��",1);
	while(1)
	{
		i=0;
		j=0;

		uint8_t arr[8]={0,0,0,0,2,3,5,9};//ʱ��
		uint8_t site[8] = {0,8,24,32,48,56,72,80};//��ʾ����
		hour1 = 10*arr[0]+arr[1];
		min1 = 10*arr[2]+arr[3];
		hour2 = 10*arr[4]+arr[5];
		min2 = 10*arr[6]+arr[7];
		//��ʼ����ʾ
		for (j=0;j<8;j++)
		{
			if (j==0)
				OLED_Show_inverse(site[j],2,arr[j]+48);
			else
				OLED_Show(site[j],2,arr[j]+48,0,1);
		}
		Disp_sentence(16,2,":",0);
		Disp_sentence(40,2,"~",0);
		Disp_sentence(64,2,":",0);
		while(1)
		{
			key = Wait_Key();
			if (key == NO_KEY)
				return USER_LOGIN_TIMEOUT;
			else if (key==0x0a)//�Ǻ�
			{
				if(i==0)
					return USER_BACK;
				else
					i--;
			}
			else if (key==0x0b)//ȷ��
			{
				//���Ϸ���
				if ((hour1>23)||(hour2>23)||(min1>59)||(min1>59))
				{
					Disp_sentence(25,2,"��������",1);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					delay_ms(1000);
					OLED_CLS();
					break;
				}
				//����
				unlocktime.hour = hour1;
				unlocktime.hour2 = hour2;
				unlocktime.minute = min1;
				unlocktime.minute2 = min2;
				STMFLASH_Write(UNLOCK_TIME_addr,(uint16_t*)&unlocktime,UNLOCK_TIME_length/2);//�洢
				return USER_CONFIRM;
			}
			else //�û��������֣�����
			{
				arr[i] = key;
				if(i<7)//��겻�����ұߣ��������
					i++;
				hour1 = 10*arr[0]+arr[1];
				min1 = 10*arr[2]+arr[3];
				hour2 = 10*arr[4]+arr[5];
				min2 = 10*arr[6]+arr[7];
			}
			//���»��ƽ���
			for (j=0;j<8;j++)
			{
				if (j==i)//��ɫ��ʾ
					OLED_Show_inverse(site[j],2,arr[j]+48);
				else //��ɫ��ʾ
					OLED_Show(site[j],2,arr[j]+48,0,1);
			}
		}
	}

}

// ��ϵͳʱ�����á�����
uint8_t time_settings(void){
	int j=0;
	int i=0;
	uint8_t key;
	uint8_t leap_year;

	while(1){
		uint8_t arr[12]={2,0,1,5,0,1,0,1,0,0,0,0};	//ʱ��
		uint8_t sitex[12] = {0,8,16,24,48,56,80,88,0,8,32,40};	//��ʾ���ֵĺ��������
		uint8_t sitey[12] = {2,2,2,2,2,2,2,2,4,4,4,4};	//��ʾ���ֵ����������
		u8 mon_table[12]={31,29,31,30,31,30,31,31,30,31,30,31};	//�·ݱ�
		u16 year;
		u8 smon,sday,hour,min;
		year = arr[0]*1000+ arr[1]*100+ arr[2]*10+ arr[3];	//��
		smon = arr[4]*10+ arr[5];
		sday = arr[6]*10+ arr[7];
		hour = arr[8]*10+ arr[9];
		min = arr[10]*10+ arr[11];
		// RTC_Set(2015,1,1,0,0,0);  //����ʱ��

		// ��ʼ����ʾ
		OLED_CLS();
		for (j=0;j<12;j++){
			// ��ɫ��ʾ
			if (j==0)
				OLED_Show_inverse(sitex[j],sitey[j],arr[j]+48);
			// ��ɫ��ʾ
			else
				OLED_Show(sitex[j],sitey[j],arr[j]+48,0,1);
		}
		Disp_sentence(32,2,"��",0);
		Disp_sentence(64,2,"��",0);
		Disp_sentence(96,2,"��",0);
		Disp_sentence(16,4,"ʱ",0);
		Disp_sentence(48,4,"��",0);

		// ʱ������
		i=0;
		while(1){
			key = Wait_Key();

			// û�а��°���
			if (key == NO_KEY){
				return USER_LOGIN_TIMEOUT;
			}

			// *������
			else if (key==0x0a){
				if(i==0)
					return USER_BACK;
				else
					i--;
			}

			// #��ȷ��
			else if (key==0x0b){
				// ���Ϸ���
				if ((smon>12)||(hour>23)||(min>59))//�� ʱ �ֵĲ��Ϸ����
				{
					Disp_sentence(0,0,"��������",1);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					// flag = 1;
					delay_ms(1000);
					break;
				}
				if (sday>mon_table[smon-1])//�� ���Ϸ�
				{
					Disp_sentence(0,0,"��������",1);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					// flag = 1;
					delay_ms(1000);
					break;
				}
				if((!leap_year)&&(smon==2)&&(sday>28))//ƽ��2�²��Ϸ�
				{
					Disp_sentence(0,0,"��������",1);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					// flag = 1;
					delay_ms(1000);
					break;
				}
				RTC_Set(year,smon,sday,hour,min,0);
				return USER_CONFIRM;
			}

			// �û��������֣�����arr
			else {
				arr[i] = key;
				if(i<11)//��겻�����ұߣ��������
					i++;
				year = arr[0]*1000+ arr[1]*100+ arr[2]*10+ arr[3];//��
				smon = arr[4]*10+ arr[5];
				sday = arr[6]*10+ arr[7];
				hour = arr[8]*10+ arr[9];
				min = arr[10]*10+ arr[11];
				leap_year = Is_Leap_Year(year);
			}

			// ���»��ƽ���
			for (j=0;j<12;j++){
				if (j==i)//��ɫ��ʾ
					OLED_Show_inverse(sitex[j],sitey[j],arr[j]+48);
				else //��ɫ��ʾ
					OLED_Show(sitex[j],sitey[j],arr[j]+48,0,1);
			}
		}

	}
}

// ���°桿��֤����Ա����
uint8_t admin_verify(void){
	uint8_t temp_u8;
	uint8_t key_length;
	uint8_t finger_num;
	uint8_t key;
	uint32_t RFCARD_ID;
	uint32_t while_cnt=0;
	uint32_t addr;
	int i;
	MY_USER my_user1;
	QS808_Rec_Buf_refresh();
	while(1){
		while_cnt++;
		// �����ʱ���� return USER_LOGIN_TIMEOUT
		if (while_cnt == WAIT_TIME_MS/100){
			//���while��1��ÿȦ��ʱ100ms
			return USER_LOGIN_TIMEOUT;
		}
		// �����û����ָ��ͷ���͹����ݣ��ͷ���Ѱ����ָָ��
		if (QS808_Rec_Buf.Trans_state == reset){
			QS808_Rec_Buf.Trans_state = set;
			// ����Ѱ����ָָ�ֻ���Ͳ�����
			QS808_Detect_Finger();
		}

		// ɨ�谴��
		key = IsKey();
		// ���û�а������£��Ϳ�ʼ���ָ�ƻ�����Ƶ��
		if (key == NO_KEY){
			// ���busy����˵��ָ��ͷ�������������һ֡
			if (QS808_Rec_Buf.Rec_state == busy){
				// ������һ֡
				uint8_t temp = QS808_Detect_Finger_Unpack();
				// ���ָ�ƻ�ȡ�ɹ����ͽ���ָ�Ʋ�ѯ
				if (temp == ERR_FINGER_DETECT){
					// ����ָ�Ʋ�ѯ
					temp_u8 = QS808_SEARCH(&finger_num);
					// ���ָ�Ʋ�ѯ��ʱ���Ͳ�����������return
					if (temp_u8 == QS808_WAIT_TIMEOUT){
						Disp_sentence(48,4,"��ʱ",1);
						delay_ms(1000);
						OLED_CLS();
						return USER_LOGIN_TIMEOUT;
					}
					// ���ָ�Ʋ�ѯ�ɹ�
					else if (temp_u8 == ERR_SUCCESS){
						// ��ָ֤���Ƿ����ڹ���Ա
						for (i=0; i<MY_USER_MAX_NUM; i++){
							addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
							STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2);
							// ���������¼����û�
							if (my_user1.flag == 0xAA){
								// �����֤��
								if (my_user1.finger_number ==  finger_num){
									// ���������ǹ���Ա��������������return VERIFF_SUCCESS
									if (my_user1.my_user_type == admin){
										Disp_sentence(32,2,"��֤�ɹ�",1);
										if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
										delay_ms(1000);
										OLED_CLS();
										return VERIFF_SUCCESS;
									}
									// �������˲��ǹ���Ա��������������return VERIFF_FAIL
									else{
										Disp_sentence(32,2,"��֤ʧ��",1);
										Disp_sentence(24,4,"��������֤",0);
										if(!SPEAK_flag) SPEAK_OPT_FAIL();
										delay_ms(1000);
										OLED_CLS();
										return VERIFF_FAIL;
									}
								}
							}
						}
					}
					// ������ָ�ƿ�Ϊ�գ�ָ�Ʋ����ڣ�ָ���������ö���Ϊ��֤ʧ�ܣ���Ȼ���������
					else{
						Disp_sentence(32,2,"��֤ʧ��",1);
						Disp_sentence(24,4,"��������֤",0);
						delay_ms(1000);
						OLED_CLS();
						return VERIFF_FAIL;;
					}
				}
			}

			// �����Ƶ��
			temp_u8 = IC_test (&RFCARD_ID);
			// ��⵽��Ƶ��
			if (temp_u8 == RFCARD_DETECED)
			{
					for (i=0;i<MY_USER_MAX_NUM;i++)
					{
						addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
						STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2);
						if (my_user1.flag == 0xAA)
						{
							if (my_user1.rfcard_id ==  RFCARD_ID)//��֤������Ƶ��
							{
								if (my_user1.my_user_type == admin)//����Ա���ܽ�������ģʽ
								{
									Disp_sentence(32,2,"��֤�ɹ�",1);
									delay_ms(1000);
									return VERIFF_SUCCESS;
								}
								else //�ǹ���Ա
								{
										Disp_sentence(32,2,"��֤ʧ��",1);
										Disp_sentence(24,4,"��������֤",0);
										delay_ms(1000);
										OLED_CLS();
										return VERIFF_FAIL;;
								}

							}
						}
					}
					//���е����� ˵���˿�û��¼��� ���߲��ǹ���Ա
					Disp_sentence(32,2,"��֤ʧ��",1);
					Disp_sentence(24,4,"��������֤",0);
					delay_ms(1000);
					OLED_CLS();
					return VERIFF_FAIL;
				}
		}
		// ������� * ���� return USER_BACK
		else if (key == 0x0a){
			return USER_BACK;
		}
		// ������� # ��ʲô������
		else if (key == 0x0b){
		}
		// ����������ּ����Ϳ�ʼ����������֤
		else {
			// ���밴����׽����¼���룬
			// temp_u8 = Key_Cap(key,&key_length,1);
			temp_u8 = admin_verify3_key_cap(key,&key_length);
			// ������� * ���� return USER_BACK
			if(temp_u8 == KEY_CANCEL){
				return USER_BACK;
			}
			// �����ʱ���Ͳ�����������return USER_LOGIN_TIMEOUT
			else if (temp_u8 == KEY_TIMEOUT){
				Disp_sentence(48,2,"��ʱ",1);
				delay_ms(1000);
				OLED_CLS();
				return USER_LOGIN_TIMEOUT;
			}
			// �������������Ͳ�����������continue
			else if (temp_u8 == KEY_TOO_LONG){
				Disp_sentence(32,2,"����̫��",1);
				Disp_sentence(24,4,"������¼��",0);
				delay_ms(1000);
				OLED_CLS();
				continue;
			}
			// �������������ɣ��Ϳ�ʼ����������֤
			else if (temp_u8 == KEY_CONFIRM){
				// ������֤����
				temp_u8 = password_verify_2admin(key_length);
				// �����֤ʧ�ܣ��Ͳ�����������return VERIFF_FAIL
				if (temp_u8 == VERIFF_FAIL){
					Disp_sentence(32,2,"��֤ʧ��",1);
					Disp_sentence(24,4,"��������֤",0);
					delay_ms(1000);
					OLED_CLS();
					return VERIFF_FAIL;
				}
				// �����֤�ɹ����Ͳ�����return VERIFF_SUCCESS
				else {
					Disp_sentence(32,2,"��֤�ɹ�",1);
					delay_ms(1000);
					return VERIFF_SUCCESS;
				}
			}
		}
	}
}

// �������������Ƿ���ȷ�������ڽ������Աģʽ��
// key_length	��������ǵ�ǰ����ĳ���
// ֧�֡����롯����
uint8_t password_verify_2admin(uint8_t key_length){
	char ans = VERIFF_FAIL;
	uint8_t key_temp[6]={NO_KEY,NO_KEY,NO_KEY,NO_KEY,NO_KEY,NO_KEY};

	// ������������Ϊ6λ���Ǿ�ֱ�ӿ�ʼ����������ȷ��
	if (key_length==6){
		ans = password_verify_only6_admin(6, key_temp);
	}
	// �����������볤��6λ���ǾͿ�ʼѭ������������ȷ��
	else if (key_length>6){
		for (u8 i=0; i<=key_length-6; i++){
			// ��ȡ6λ����� key_temp
			for (u8 j=0; j<6; j++){
				key_temp[j] = Key_Buffer[j+i];
			}
			// �� key_temp ����������ȷ����֤
			ans = password_verify_only6_admin(6, key_temp);
			if (VERIFF_SUCCESS == ans){
				break;
			}
		}
	}
	return ans;
}

// �������������Ƿ���ȷ�������ڽ�����
// key_length	��������ǵ�ǰ����ĳ���
// ֧�֡����롯����
uint8_t password_verify_2unlock(uint8_t key_length, user_type *user_type_o){
	*user_type_o = error;
	char ans = VERIFF_FAIL;
	uint8_t key_temp[6]={NO_KEY,NO_KEY,NO_KEY,NO_KEY,NO_KEY,NO_KEY};

	// ������������Ϊ6λ���Ǿ�ֱ�ӿ�ʼ����������ȷ��
	if (key_length==6){
		ans = password_verify_only6(6, user_type_o, Key_Buffer);
	}
	// �����������볤��6λ���ǾͿ�ʼѭ������������ȷ��
	else if (key_length>6){
		for (u8 i=0; i<=key_length-6; i++){
			// ��ȡ6λ����� key_temp
			for (u8 j=0; j<6; j++){
				key_temp[j] = Key_Buffer[j+i];
			}
			// �� key_temp ����������ȷ����֤
			ans = password_verify_only6(6, user_type_o, key_temp);
			if (VERIFF_SUCCESS == ans){
				break;
			}
		}
	}
	return ans;
}

// �������������Ƿ���ȷ������֧��6λ������֤
// �����Ƚ�ȫ�ֱ���Key_Buffer��flash�д洢�������Ƿ���ͬ
// �����֤��������û�����
uint8_t password_verify_only6(uint8_t key_length, user_type *user_type_o, uint8_t *key_ans){
	*user_type_o = error;
	int i=0;
	int j=0;
	int flag=0;	//����ƥ���־��0��ʾʧ�� 1��ʾ�ɹ�


	// ����flash�ж������û��ṹ��
	uint32_t addr = (uint32_t)MY_USER_addr_base;
	MY_USER * my_user1;
	my_user1 = (MY_USER*)malloc(sizeof(MY_USER));

	// �����������볤�ȴ���6λ���� return VERIFF_FAIL
	if (key_length != 6){
		return VERIFF_FAIL;
	}

	// ������볤�ȵ���6λ���ǾͿ�ʼ��������Ƿ���ȷ
	else {
		for (i=0; i<MY_USER_MAX_NUM; i++){
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr, (uint16_t*)my_user1, MY_USER_length/2);
			// �ж��Ƿ�����Ч����
			if (my_user1->flag != 0xAA)
				continue;
			// ��ʼ�Ƚ�
			while (j<6){
				if (my_user1->password[j] != key_ans[j]){
					flag = 0;
					break;
				}
				else {
					flag = 1;
				}
				j++;
			}
			// ������ȷ
			if (flag){
				// *user_type_o = my_user1->my_user_type;
				free(my_user1);
				my_user1 = NULL;
				return VERIFF_SUCCESS;
			}
		}
		free(my_user1);
		my_user1 = NULL;
		return VERIFF_FAIL;
	}
}
// ���溯���ġ�ֻ�й���Ա�ſ���ͨ�����汾
uint8_t password_verify_only6_admin(uint8_t key_length, uint8_t *key_ans){
	int i=0;
	int j=0;
	int flag=0;	//����ƥ���־��0��ʾʧ�� 1��ʾ�ɹ�


	// ����flash�ж������û��ṹ��
	uint32_t addr = (uint32_t)MY_USER_addr_base;
	MY_USER * my_user1;
	my_user1 = (MY_USER*)malloc(sizeof(MY_USER));

	// �����������볤�ȴ���6λ���� return VERIFF_FAIL
	if (key_length != 6){
		return VERIFF_FAIL;
	}

	// ������볤�ȵ���6λ���ǾͿ�ʼ��������Ƿ���ȷ
	else {
		for (i=0; i<MY_USER_MAX_NUM; i++){
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr, (uint16_t*)my_user1, MY_USER_length/2);
			// �ж��Ƿ�����Ч����
			if (my_user1->flag != 0xAA)
				continue;
			if (my_user1->my_user_type != admin)//�û����ǹ���Ա����ֹ�û���������ͨ�������������ģʽ��
				continue;
			// ��ʼ�Ƚ�
			while (j<6){
				if (my_user1->password[j] != key_ans[j]){
					flag = 0;
					break;
				}
				else {
					flag = 1;
				}
				j++;
			}
			// ������ȷ
			if (flag){
				// *user_type_o = my_user1->my_user_type;
				free(my_user1);
				my_user1 = NULL;
				return VERIFF_SUCCESS;
			}
		}
		free(my_user1);
		my_user1 = NULL;
		return VERIFF_FAIL;
	}
}

// �����һ������ʾ 2018-02-28 18:31 ������Ϊ16��
// ֱ�Ӵ�ӡ���������flag���������ֻ��һ��Ϳ��ԣ����ʱ�����ʾ�ò���ʵʱ��
void time_disp_bottom(void){
	char disp1[17];
	// ��ʽ�磺2018-02-28
	disp1[0] = 48 + calendar.w_year/1000;
	disp1[1] = 48 + (calendar.w_year/100)%10;
	disp1[2] = 48 + (calendar.w_year/10)%10;
	disp1[3] = 48 + (calendar.w_year)%10;
	disp1[4] = '-';
	disp1[5] = 48 + (calendar.w_month)/10;
	disp1[6] = 48 + (calendar.w_month)%10;
	disp1[7] = '-';
	disp1[8] = 48 + (calendar.w_date)/10;
	disp1[9] = 48 + (calendar.w_date)%10;
	disp1[10] = ' ';
	// 18:31
	disp1[11] = 48 + (calendar.hour)/10;
	disp1[12] = 48 + (calendar.hour)%10;
	disp1[13] = ':';
	disp1[14] = 48 + (calendar.min)/10;
	disp1[15] = 48 + (calendar.min)%10;
	// ���������
	disp1[16] = 0;

	// ��ӡ�� OLED ��
	Disp_sentence_singleline(0,6,disp1,1);
}

// ����Ļ�м���ʾʱ�䣬�� 18:31��������д��
u8 time_disp_big_flag=0;
void time_disp_big(void){
	char disp1[4];
	// �����м������Բ��
	if (timenew){
		disp1[0] = (calendar.hour)/10;
		disp1[1] = (calendar.hour)%10;
		disp1[2] = (calendar.min)/10;
		disp1[3] = (calendar.min)%10;

		// ��ӡ����
		show_time_big(4, 1, disp1[0]);
		show_time_big(28, 1, disp1[1]);
		show_time_big(76, 1, disp1[2]);
		show_time_big(100, 1, disp1[3]);

		if (time_disp_big_flag == 0){
			show_time_pot();
			time_disp_big_flag = 1;
		}
		else {
			close_time_pot();
			time_disp_big_flag = 0;
		}
	}
	timenew=0;
}

// ����ͼ����ʾ
void key_disp(uint8_t x,uint8_t y,uint8_t key,uint8_t clr){
	//clr 0��ʾ����� 1��ʾ���
	char disp[2];
	if (key == 0xa)
		disp[0] = '*';
	else if (key == 0xb)
		disp[0] = '#';
	else
		disp[0] = key+48;
	disp[1] = 0;//�ַ�����β
	Disp_sentence(x,y,disp,clr);
}

// �û���Ųɼ�������һ���������棬�û����ñ��
uint8_t user_id_cap(uint32_t * user_id,user_type * user_type_o){
	//user_id �ɼ�����id
	//user_type_o ���id��Ӧ���û����ͣ�����ǿ�id��������������ù�
	//�ɼ��û�����ı�Ŵ�
	*user_type_o = admin;
	int i=0;
	uint8_t key;
	uint8_t arr[3]={0,0,0};//����洢�û�����ı��
	MY_USER my_user1;
	uint32_t addr;
	Disp_sentence(20,2,"��������",1);
	OLED_Show_inverse(40,4,'0');
	OLED_Show(48,4,'0',0,1);
	OLED_Show(56,4,'0',0,1);
	*user_id = 0;
	while(1)
	{
		key = Wait_Key();
		if (key == NO_KEY)
			return USER_LOGIN_TIMEOUT;
		else if (key==0x0a)//�Ǻ�
		{
			if(i==0)
				return USER_BACK;
			else
				i--;
		}
		else if (key==0x0b)//ȷ��
		{
			//�ж��û�id�Ƿ��ظ�
			for (i=0;i<MY_USER_MAX_NUM;i++)
			{
				addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
				STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2);
				if (my_user1.flag == 0xAA)
				{
					if (my_user1.number ==  *user_id)
					{
						*user_type_o = my_user1.my_user_type;
						 return USER_ID_REDUP;
					}
				}
			}
			return USER_CONFIRM;
		}
		else //�û��������֣�����arr
		{
			arr[i] = key;
			*user_id = 100*arr[0]+10*arr[1]+arr[2];
			if(i<2)//��겻�����ұߣ��������
				i++;
		}
		//���»��ƽ���
		if(i==0)
		{
			OLED_Show_inverse(40,4,arr[0]+48);
			OLED_Show(48,4,arr[1]+48,0,1);
			OLED_Show(56,4,arr[2]+48,0,1);
		}
		else if (i==1)
		{
			OLED_Show(40,4,arr[0]+48,0,1);
			OLED_Show_inverse(48,4,arr[1]+48);
			OLED_Show(56,4,arr[2]+48,0,1);
		}
		else
		{
			OLED_Show(40,4,arr[0]+48,0,1);
			OLED_Show(48,4,arr[1]+48,0,1);
			OLED_Show_inverse(56,4,arr[2]+48);
		}

	}
}

// �ָ���������
void back2factory(void){
	int i;
	QS808_CMD_DEL_ALL();//ɾ��ȫ��ָ��
	FLASH_Unlock();
	for (i=48;i<64;i++)
		FLASH_ErasePage(i*1024+STM32_FLASH_BASE);//�����û���������
	FLASH_Lock();//����
}

// // ʱ����ʾ���ɰ汾�ĺ����������Ѿ�����ʹ��
// void time_disp(void){
// 	// int m,n;
// 	char disp1[15];
// 	char disp2[9];
// 	if (timenew){
// 		disp1[0] = 48 + calendar.w_year/1000;
// 		disp1[1] = 48 + (calendar.w_year/100)%10;
// 		disp1[2] = 48 + (calendar.w_year/10)%10;
// 		disp1[3] = 48 + (calendar.w_year)%10;
// 		disp1[4] = '��'>>8;
// 		disp1[5] = '��';
// 		disp1[6] = 48 + (calendar.w_month)/10;
// 		disp1[7] = 48 + (calendar.w_month)%10;
// 		disp1[8] = '��'>>8;
// 		disp1[9] = '��';
// 		disp1[10] = 48 + (calendar.w_date)/10;
// 		disp1[11] = 48 + (calendar.w_date)%10;
// 		disp1[12] = '��'>>8;
// 		disp1[13] = '��';
// 		disp1[14] = 0;//����
// 		disp2[0] = 48 + (calendar.hour)/10;
// 		disp2[1] = 48 + (calendar.hour)%10;
// 		disp2[2] = ':';
// 		disp2[3] = 48 + (calendar.min)/10;
// 		disp2[4] = 48 + (calendar.min)%10;
// 		disp2[5] =  ':';
// 		disp2[6] = 48 + (calendar.sec)/10;
// 		disp2[7] = 48 + (calendar.sec)%10;
// 		disp2[8] = 0;//����
// 		Disp_sentence(2,2,disp1,0);
// 		Disp_sentence(25,4,disp2,0);
// 		Disp_sentence(30,6,"����",0);
// 		switch(calendar.week){
// 			case 1:Disp_sentence(62,6,"һ",0);break;
// 			case 2:Disp_sentence(62,6,"��",0);break;
// 			case 3:Disp_sentence(62,6,"��",0);break;
// 			case 4:Disp_sentence(62,6,"��",0);break;
// 			case 5:Disp_sentence(62,6,"��",0);break;
// 			case 6:Disp_sentence(62,6,"��",0);break;
// 			case 0:Disp_sentence(62,6,"��",0);break;
// 			default:Disp_sentence(62,6,"??",0);break;
// 		}
// 		timenew = 0;
// 		//��ʾ����
// 		if (Battery_quantity<3.7)
// 			OLED_Show_Power(0);
// 		else if (Battery_quantity<4.2)
// 			OLED_Show_Power(1);
// 		else if (Battery_quantity<4.7)
// 			OLED_Show_Power(2);
// 		else if (Battery_quantity<5.2)
// 			OLED_Show_Power(3);
// 		else
// 			OLED_Show_Power(4);
// 	}
// }
