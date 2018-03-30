#include "./my_board.h"



// extern _calendar_obj calendar;//ʱ�ӽṹ��
// extern uint32_t ms10_cnt;

extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[12];
extern u8 USART1_RecvBuf_Length;

// float Battery_quantity;

int main(void)
{
	delay_init();			// ��ϵͳʱ�ӡ���ʼ��
	RTC_Init();				// ��RTCʱ�ӡ���ʼ��
	//TSM12_Init();			// ����������оƬ����ʼ��
	BLE_init();				// ������ʼ��
	debug_usart_init();		// ���ڳ�ʼ��

	power_ctrl_init();		// ����Դ���ơ���ʼ��
	Power_ctrl_on();		// �򿪵�Դ����
	RC522_Init();			// ����Ƶ��оƬ����ʼ��
	TIM3_Int_Init(99,7199);	// ��ʱ��3�����Ժ�������ʱ����,10KƵ�ʼ���,ÿ10msһ���ж�
	
	
	// OLED_Init();			// ��OLED����ʼ��
	// QS808_Init();			// ��ָ�Ʋɼ�ͷ����ʼ��
	// Gate_Init();			// ��������е���ơ���ʼ��
	// delay_ms(100);
	// VCC_Adc_Init();			// ��ADC��ͨ����ʼ��
	// UT588C_init();			// ������оƬ����ʼ��




	u16 a,b,c,d;
	u32 RFCARD_ID;
	
	while(1){
		// // ɨ�谴��
		// key = IsKey();
		
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
		

		// order = Usart_RecvOrder(USART1);
		// if(order!=0x0000) {
		// 	pUsart_SentMessage(USART1, &order);
		// }

		if(Add_RFCard() == ERROR_CODE_SUCCESS) {
			Usart_SendUserId(USART1, 1);
		}

		
		// Usart_SendUserId(USART1, 1);
		// delay_ms(1000);
	}





	// // �������ݲ���
	// while(1){
	// 	;
	// }


	// back2factory();

	// ������Ʋ���
	// start_interface();

}  // end main()
