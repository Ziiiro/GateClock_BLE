#include "./my_board.h"


extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern u8 USART1_RecvBuf_Length;
extern u8 WAKEUP_SOURCE;
extern QS808_Rec_Buf_type QS808_Rec_Buf;

float Battery_quantity;
u8 SLEEP_MAX = 30;

int main(void) {
	delay_init();			// ��ϵͳʱ�ӡ���ʼ��
	RTC_Init();				// ��RTCʱ�ӡ���ʼ��
	TSM12_Init();			// ����������оƬ����ʼ��
	BLE_init();				// ����������ʼ��
	debug_usart_init();		// �����ڡ���ʼ��
	power_ctrl_init();		// ����Դ���ơ���ʼ��
	Power_ctrl_on();		// �򿪵�Դ����
	RC522_Init();			// ����Ƶ��оƬ����ʼ��
	TIM3_Int_Init(99,7199);	// ��ʱ��3�����Ժ�������ʱ����,10KƵ�ʼ���,ÿ10msһ���ж�
	QS808_Init();			// ��ָ�Ʋɼ�ͷ����ʼ��
	Gate_Init();			// ��������е���ơ���ʼ��
	OLED_Init();			// ��OLED����ʼ��
	delay_ms(100);
	VCC_Adc_Init();			// ��ADC��ͨ����ʼ��
	UT588C_init();			// ������оƬ����ʼ��
	// QS808_CMD_DEL_ALL();	// ɾ��ȫ��ָ��
	
	u8 work_flag;
	u16 temp_cmdid,temp_userid,temp_return,sleep_count;
	u32 temp_RFCARD_ID;

	sleep_count = 0;
	show_clock_close_big();
	while(1) {
		// ˯�߼���++
		sleep_count++;
		
		// ��ʾ������
		if (work_flag==1) {
			work_flag = 0;
			show_clock_close_big();
		}

		// �ж��Ƿ�ý���˯��ģʽ
		if (sleep_count>=SLEEP_MAX && QS808_Rec_Buf.Trans_state==reset) {
			sleep_count = 0;
			QS808_Rec_Buf_refresh();
			Disp_sentence(48,2,"����",1);
			delay_ms(500);
			PWR_Standby_Mode();

			// ����֮������￪ʼִ�г�����ִ��һ��ָ��ɨ�裬���ٿ�������
			show_clock_close_big();
			// �����ָ��ͷ����
			if(WAKEUP_SOURCE==0) {
				// �����⵽����ָ���£��Ϳ�ʼ���ָ����ȷ�ԣ�׼������
				if (QS808_CMD_FINGER_DETECT()==ERR_FINGER_DETECT) {
					sleep_count = SLEEP_MAX;
					temp_return = Confirm_Finger();
					if (temp_return==ERROR_CODE_SUCCESS) {
						show_clock_open_big();
						SPEAK_OPEN_THE_DOOR();
						Gate_Unlock();
					}
					else {
						Disp_sentence(28,2,"��֤ʧ��",1);
						SPEAK_OPEN_THE_DOOR_FAIL();
						delay_ms(1000);	// ��֤ʧ�ܵ���ʾʱ��
					}
				}
			}
		}

		// ������յ������ݴ��룬˵���ֻ��˷�������Ϣ������Ҫ������Ϣ¼�����һ������
		if ( Usart_RecvOrder(USART1)==SYS_RECV_ORDER ) {
			// ���� temp_cmdid �����з�֧�ж�
			temp_cmdid = RecvBuf2Cmdid();
			switch( temp_cmdid ) {
				/************************* ���յ������ָ�ơ�ָ�� *************************/
				case CMDID_ADD_FINGER:
					sleep_count = 0;
					SPEAK_DUDUDU();
					if(Add_Finger(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_ADD_Success(USART1, 3, temp_userid);
					// ��ʱһ��ʱ�䣬��ֱֹ�ӽ���ָ�ƽ�������
					delay_ms(1000);
					break;


				/************************* ���յ���ɾ��ָ�ơ�ָ�� *************************/
				case CMDID_DEL_FINGER:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_Finger(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_DEL_Success(USART1);
					else
						Usart_SendFinger_DEL_Error(USART1);
					break;


				/************************ ���յ��������Ƶ����ָ�� ************************/
				case CMDID_ADD_RFCARD:
					sleep_count = 0;
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
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_RFCard(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendRFCard_DEL_Success(USART1);
					else
						Usart_SendRFCard_DEL_Error(USART1);
					break;


				/************************* ���յ���������롿ָ�� *************************/
				case CMDID_ADD_PASSWORD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_return = Add_Password(&temp_userid);
					if(temp_return== ERROR_CODE_SUCCESS)
						Usart_SendPassword_ADD_Success(USART1, temp_userid);
					else
						Usart_SendPassword_ADD_Error(USART1, temp_return);
					break;


				/************************* ���յ���ɾ�����롿ָ�� *************************/
				case CMDID_DEL_PASSWORD:
					sleep_count = 0;
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
				sleep_count = 0;
				work_flag = 1;
				temp_return = Confirm_Finger();
				if (temp_return==ERROR_CODE_SUCCESS) {
					show_clock_open_big();
					SPEAK_OPEN_THE_DOOR();
					Gate_Unlock();
				}
				else {
					Disp_sentence(28,2,"��֤ʧ��",1);
					SPEAK_OPEN_THE_DOOR_FAIL();
				}
				// ��ֹ��ʱ�����ٴν���ָ�Ƽ��
				delay_ms(1000);
			}

			// �����⵽����Ƶ���������Ϳ�ʼ�����Ƶ������ȷ�ԣ�׼������
			if (RFCard_test(&temp_RFCARD_ID) == RFCARD_DETECED) {
				sleep_count = 0;
				work_flag = 1;
				temp_return = Confirm_RFCard(temp_RFCARD_ID);
				if (temp_return==ERROR_CODE_SUCCESS) {
					show_clock_open_big();
					SPEAK_OPEN_THE_DOOR();
					Gate_Unlock();
				}
				else {
					Disp_sentence(28,2,"��֤ʧ��",1);
					SPEAK_OPEN_THE_DOOR_FAIL();
				}
				// ��֤ʧ����ʾʱ��
				delay_ms(1000);
			}

			// �����⵽�а������£��ͽ�������������棬׼������
			if (TMS12_ReadOnKey() != KEY_NULL) {
				sleep_count = 0;
				work_flag = 1;
				u8 password_buf[LENGTH_KEY_BUF], buf_length=0, last_press, temp;
				SPEAK_DUDUDU();
				OLED_CLS();

				// �����������
				Battery_quantity = Get_Battery();
				if (Battery_quantity<3.7)		OLED_Show_Power(0);
				else if (Battery_quantity<4.2)	OLED_Show_Power(1);
				else if (Battery_quantity<4.7)	OLED_Show_Power(2);
				else if (Battery_quantity<5.2)	OLED_Show_Power(3);
				else 							OLED_Show_Power(4);
				Disp_sentence(24,2,"����������",0);

				Create_NewPasswordBuf(password_buf);
				while(1) {
					// ���°���������X
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
							SPEAK_OPEN_THE_DOOR();
							Gate_Unlock();
						}
						else {
							Disp_sentence(28,2,"��֤ʧ��",1);
							SPEAK_OPEN_THE_DOOR_FAIL();
						}
						// ����������ʾ
						delay_ms(1000);
						break;
					}
					// ���볤�Ȳ���6λ
					else if (temp==PASSWORD_CODE_SHORT) {
						SPEAK_DUDUDU();
						Disp_sentence(28,2,"��֤ʧ��",1);
						SPEAK_OPEN_THE_DOOR_FAIL();
						// ����������ʾ
						delay_ms(1000);
						break;
					}
				}
			}
		}
	}
}

// // �������
// int* twoSum(int* nums, int numsSize, int target) {
//     int min = 2147483647;
//     int i = 0;
//     for (i = 0; i < numsSize; i++) {
//         if (nums[i] < min)
//             min = nums[i];
//     }
//     int max = target - min;
//     int len = max - min + 1;   // ȷ��hash����
//     int *table = (int*)malloc(len*sizeof(int));
//     int *indice = (int*)malloc(2*sizeof(int));


//     for (i = 0; i < len; i++) {
//         table[i] = -1;         // hash��ֵ��ȡһ�������ܳ��ֵ���ֵ��Ҳ����С��0����
//     }
//     for (i = 0; i < numsSize; i++) {
//         if (nums[i]-min < len) {
//             if (table[target-nums[i]-min] != -1) {        //�������Ϊtarget
//                 indice[0] = table[target-nums[i] - min];
//                 indice[1] = i;
//                 return indice;
//             }
//             table[nums[i]-min] = i;
//         }
//     }
//     free(table);
//     return indice;
// }
