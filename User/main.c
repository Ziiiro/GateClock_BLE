#include "./my_board.h"


extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern u8 USART1_RecvBuf_Length;


int main(void) {
	delay_init();			// ��ϵͳʱ�ӡ���ʼ��
	RTC_Init();				// ��RTCʱ�ӡ���ʼ��
	TSM12_Init();			// ����������оƬ����ʼ��
	BLE_init();				// ������ʼ��
	debug_usart_init();		// ���ڳ�ʼ��
	power_ctrl_init();		// ����Դ���ơ���ʼ��
	Power_ctrl_on();		// �򿪵�Դ����
	RC522_Init();			// ����Ƶ��оƬ����ʼ��
	TIM3_Int_Init(99,7199);	// ��ʱ��3�����Ժ�������ʱ����,10KƵ�ʼ���,ÿ10msһ���ж�
	QS808_Init();			// ��ָ�Ʋɼ�ͷ����ʼ��
	Gate_Init();			// ��������е���ơ���ʼ��
	OLED_Init();			// ��OLED����ʼ��
	// delay_ms(100);
	// VCC_Adc_Init();			// ��ADC��ͨ����ʼ��
	UT588C_init();			// ������оƬ����ʼ��
	QS808_CMD_DEL_ALL();	// ɾ��ȫ��ָ��

	u16 temp_cmdid,temp_userid,temp_return;
	u32 temp_RFCARD_ID;
	while(1) {
		// show_clock_close_big();
		// ������յ������ݴ��룬˵���ֻ��˷�������Ϣ������Ҫ������Ϣ¼�����һ������
		if ( Usart_RecvOrder(USART1)==SYS_RECV_ORDER ) {
			// ���� temp_cmdid �����з�֧�ж�
			temp_cmdid = RecvBuf2Cmdid();
			switch( temp_cmdid ) {
				/************************* ���յ������ָ�ơ�ָ�� *************************/
				case CMDID_ADD_FINGER:
					SPEAK_DUDUDU();
					if(Add_Finger(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_ADD_Success(USART1, 3, temp_userid);
					// ��ʱһ��ʱ�䣬��ֱֹ�ӽ���ָ�ƽ�������
					delay_ms(1000);
					break;


				/************************* ���յ���ɾ��ָ�ơ�ָ�� *************************/
				case CMDID_DEL_FINGER:
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid(); 
					if (Delete_Finger(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_DEL_Success(USART1);
					else
						Usart_SendFinger_DEL_Error(USART1);
					break;


				/************************ ���յ��������Ƶ����ָ�� ************************/
				case CMDID_ADD_RFCARD:
					SPEAK_DUDUDU();
					temp_return = Add_RFCard(&temp_userid);
					if(temp_return== ERROR_CODE_SUCCESS)
						Usart_SendRFCard_ADD_Success(USART1, temp_userid);
					else
						Usart_SendRFCard_ADD_Error(USART1, temp_return);
					// ��ʱһ��ʱ�䣬��ֱֹ�ӽ�����Ƶ����������
					delay_ms(1000);
					break;


				/************************ ���յ���ɾ����Ƶ����ָ�� ************************/
				case CMDID_DEL_RFCARD:
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_RFCard(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendRFCard_DEL_Success(USART1);
					else
						Usart_SendRFCard_DEL_Error(USART1);
					break;


				/************************* ���յ���������롿ָ�� *************************/
				case CMDID_ADD_PASSWORD:
					SPEAK_DUDUDU();
					temp_return = Add_Password(&temp_userid);
					if(temp_return== ERROR_CODE_SUCCESS)
						Usart_SendPassword_ADD_Success(USART1, temp_userid);
					else
						Usart_SendPassword_ADD_Error(USART1, temp_return);
					break;


				/************************* ���յ���ɾ�����롿ָ�� *************************/
				case CMDID_DEL_PASSWORD:
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_Password(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendPassword_DEL_Success(USART1);
					else
						Usart_SendPassword_DEL_Error(USART1);
					break;
				/************************************************************************/
				default:
					break;
			}
		}

		// ���û�н��յ��ֻ��˷�������Ϣ���Ǿ�ʱ��׼������
		else {
			// �����⵽����ָ���£��Ϳ�ʼ���ָ����ȷ�ԣ�׼������
			if (QS808_CMD_FINGER_DETECT()==ERR_FINGER_DETECT) {
				temp_return = Confirm_Finger();
				if (temp_return==ERROR_CODE_SUCCESS)	SPEAK_OPEN_THE_DOOR();
				else									SPEAK_DUDUDU();
				// ��ֹ��ʱ�����ٴν���ָ�Ƽ��
				delay_ms(1000);
			}
			
			// �����⵽����Ƶ���������Ϳ�ʼ�����Ƶ������ȷ�ԣ�׼������
			if (RFCard_test(&temp_RFCARD_ID) == RFCARD_DETECED) {
				temp_return = Confirm_RFCard(temp_RFCARD_ID);
				if (temp_return==ERROR_CODE_SUCCESS)	SPEAK_OPEN_THE_DOOR();
				else									SPEAK_DUDUDU();
				// ��ֹ��ʱ�����ٴν�����Ƶ�����
				delay_ms(1000);
			}

			// �����⵽�а������£��ͽ��������������
			if (TMS12_ReadOnKey() != KEY_NULL) {
				u8 password_buf[LENGTH_KEY_BUF], buf_length=0, last_press, temp;
				SPEAK_DUDUDU();
				Disp_sentence(24,0,"����������",1);
				Create_NewPasswordBuf(password_buf);
				while(1) {
					// ���°���������
					temp = Update_KeyBuf(password_buf, &buf_length, &last_press);
					// ������������������룬����һ��
					if (temp==PASSWORD_CODE_INPUTTING) {
						SPEAK_DUDUDU();
						Interface_Password(buf_length);
					}
					// ����������볬ʱ���������˳�����ֱ��break
					else if (temp==PASSWORD_CODE_TIMEOUT || temp==PASSWORD_CODE_QUIT) {
						SPEAK_DUDUDU();
						OLED_CLS();
						break;
					}
					// �������������ɣ��Ϳ�ʼ��֤�����׼ȷ��
					else if (temp==PASSWORD_CODE_COMPLETE) {
						SPEAK_DUDUDU();
						if (Confirm_Password(password_buf, buf_length)==ERROR_CODE_SUCCESS) {
							show_clock_open_big();
						}
						else {
							show_clock_close_big();
						}
						OLED_CLS();
						break;
					}
					else {
						OLED_CLS();
						break;
					}
				}
			}
		}
	}
}
