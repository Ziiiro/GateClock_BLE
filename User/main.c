#include "my_board.h"
#include "./data/data_def.h"
#include "./lock_ctrl/lock_ctrl.h"
#include "./OLED/oled.h"
#include "./vcc_adc/vcc_adc.h"
#include "./dac/dac.h"
#include "./TSM12/TSM12.h"
// #include "./BLE/BLE.h"

extern _calendar_obj calendar;//ʱ�ӽṹ��
extern uint32_t ms10_cnt;
float Battery_quantity;


int main(void)
{
	delay_init();			// ��ϵͳʱ�ӡ���ʼ��
	RTC_Init();				// ��RTCʱ�ӡ���ʼ��
	TIM3_Int_Init(99,7199);	// ��ʱ��3�����Ժ�������ʱ����,10KƵ�ʼ���,ÿ10msһ���ж�


	power_ctrl_init();		// ����Դ���ơ���ʼ��
	Power_ctrl_on();		// �򿪵�Դ����
	OLED_Init();			// ��OLED����ʼ��
	QS808_Init();			// ��ָ�Ʋɼ�ͷ����ʼ��
	TSM12_Init();			// ����������оƬ����ʼ��
	RC522_Init();			// ����Ƶ��оƬ����ʼ��
	Gate_Init();			// ��������е���ơ���ʼ��
	delay_ms(100);
	VCC_Adc_Init();			// ��ADC��ͨ����ʼ��
	UT588C_init();			// ������оƬ����ʼ��
	first_time_init();		// �ϵ��ʼ��


	// BLE_init();				// ������ʼ��
	// debug_usart_init();		// ���ڳ�ʼ��


	// // �������ݲ���
	// u8 key;	
	// BleDataHead blue;
	// blue.m_magicCode = magicCode;
	// blue.m_version = version;
	// blue.m_totalLength = 12;
	// blue.m_cmdId = cmdId_IC;
	// blue.m_seq = 0x0000;
	// blue.m_errorCode = errorCode0;

	// int box = 11;
	// while(1){
	// 	// ɨ�谴��
	// 	key = IsKey();

	// 	// *
	// 	if (key==0x0a){
	// 		// Usart_SentMessage(UART_1, (u16*)&blue);
	// 		// Usart_SentMessage(UART_1, (u16*)&blue+1);
			
	// 		// for (u8 i=0; i<5; i++){
	// 		// 	pUsart_SentMessage(UART_1, (u16*)&blue+i);
	// 		// }

	// 		// for (u8 i=0; i<3; i++){
	// 		// 	pUsart_SendByte(UART_1, (u8*)&box+i);
	// 		// }
			
	// 		Usart_SendByte(UART_1, 0x31);
	// 		Usart_SendByte(UART_1, 0x32);
	// 		Usart_SendByte(UART_1, 0x33);


	// 		SPEAK_DUDUDU();
	// 	}
	// }

	// // �������ݲ���
	// while(1){
	// 	;
	// }


	// back2factory();


	// ������Ʋ���
	start_interface();

}  // end main()
