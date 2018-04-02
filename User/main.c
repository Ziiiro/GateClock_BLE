#include "./my_board.h"



// extern _calendar_obj calendar;//ʱ�ӽṹ��
// extern uint32_t ms10_cnt;

extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern u8 USART1_RecvBuf_Length;

// float Battery_quantity;

int main(void) {
	delay_init();			// ��ϵͳʱ�ӡ���ʼ��
	RTC_Init();				// ��RTCʱ�ӡ���ʼ��
	// TSM12_Init();			// ����������оƬ����ʼ��
	BLE_init();				// ������ʼ��
	debug_usart_init();		// ���ڳ�ʼ��
	power_ctrl_init();		// ����Դ���ơ���ʼ��
	Power_ctrl_on();		// �򿪵�Դ����
	RC522_Init();			// ����Ƶ��оƬ����ʼ��
	TIM3_Int_Init(99,7199);	// ��ʱ��3�����Ժ�������ʱ����,10KƵ�ʼ���,ÿ10msһ���ж�
	QS808_Init();			// ��ָ�Ʋɼ�ͷ����ʼ��
	// OLED_Init();			// ��OLED����ʼ��
	// Gate_Init();			// ��������е���ơ���ʼ��
	// delay_ms(100);
	// VCC_Adc_Init();			// ��ADC��ͨ����ʼ��
	UT588C_init();			// ������оƬ����ʼ��
	QS808_CMD_DEL_ALL();	// ɾ��ȫ��ָ��


	u16 temp_cmdid,temp_userid;
	while(1) {
		// ������յ������ݴ���
		if ( Usart_RecvOrder(USART1)==SYS_RECV_ORDER  ) {
			// ���� temp_cmdid �����з�֧�ж�
			temp_cmdid = RecvBuf2Cmdid();
			switch( temp_cmdid ) {
				/************************ ���յ��������Ƶ����ָ�� ************************/
				case CMDID_ADD_RFCARD:
					SPEAK_DUDUDU();
					if(Add_RFCard(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendRFCard_ADD_Success(USART1, temp_userid);
					else
						Usart_SendRFCard_ADD_Error(USART1);
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


				/************************* ���յ������ָ�ơ�ָ�� *************************/
				case CMDID_ADD_FINGER:
					SPEAK_DUDUDU();
					if(Add_Finger(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_ADD_Success(USART1, 3, temp_userid);
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

				/************************************************************************/
				default:
					break;
			}
		}
	}
}





// if(USART_Recv_Flag==1) {
// 	USART_Recv_Flag = 0;
// 	i = 0;
// 	while(USART1_RecvBuf_Length--) {
// 		// ��������
// 		USART_SendData(USART1, USART_RecvBuf[i++]);

// 		// �ȴ����ͽ���
// 		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
// 	}
// 	USART1_RecvBuf_Length = 0;
// }
