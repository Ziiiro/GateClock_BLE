#include "./usart/debug_usart.h"
#include <stdio.h>

static volatile uint32_t  UARTTimeout = UART_WAIT_TIME;

static void NVIC_Configuration(void){
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	/* �������ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* �����ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void debug_usart_init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd( UART_1_RX_GPIO_CLK|UART_1_TX_GPIO_CLK, ENABLE);

	// ʧ�� UART ʱ��
	USART_DeInit(UART_1);

	// ���� Rx ����Ϊ���ù���
	GPIO_InitStructure.GPIO_Pin = UART_1_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART_1_RX_GPIO_PORT, &GPIO_InitStructure);

	// ���� Tx ����Ϊ���ù���
	GPIO_InitStructure.GPIO_Pin = UART_1_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		// ���ÿ�©
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART_1_TX_GPIO_PORT, &GPIO_InitStructure);

	// ʹ�� UART ʱ��
	RCC_APB2PeriphClockCmd(UART_1_CLK, ENABLE);

	// ���ô���ģʽ
	USART_InitStructure.USART_BaudRate = UART_1_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART_1, &USART_InitStructure);

	// Ƕ�������жϿ�����NVIC����
	NVIC_Configuration();

	// ʹ�ܴ��ڽ����ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	// ��ע�⣡������������������������������������������������֡�жϣ���λ���Ͳ���ʵʱ���յ������ˣ�������������������
	// �������к��������ʼ�İ汾�����еģ������� ucos �汾��û�С�
	// ����������֡�жϡ�
	// USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

	// ʹ�ܴ���
	USART_Cmd(UART_1, ENABLE);

}



// ����һ���ֽ�
uint16_t Usart_SendByte( USART_TypeDef * pUSARTx, u8 ch ){
	UARTTimeout = UART_WAIT_TIME;
	// ����һ���ֽ����ݵ�USART1
	USART_SendData(pUSARTx,ch);

	// �ȴ��������
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET){
		if(UARTTimeout-- == 0) return uart_error();
	}
	return 0;
}

// ����һ���ֽڣ�ʹ�õ�ַ����
void pUsart_SendByte( USART_TypeDef * pUSARTx, u8* ch ){
	// ����һ���ֽ����ݵ�USART1
	USART_SendData(pUSARTx,*ch);
	// ����
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}
// һ���Է��������ֽڣ�Ҳ����һ����Ϣ��,ʹ�õ�ַ����
// pUSARTx:		���ں�
// message:		��Ҫ���͵�16λ��Ϣ
void pUsart_SentMessage( USART_TypeDef * pUSARTx, u16 * message){
	u8 temp_h, temp_l;

	// ȡ���߰�λ
	temp_h = ((*message)&0XFF00)>>8;
	// ȡ���Ͱ�λ
	temp_l = (*message)&0XFF;
	// ���͸߰�λ
	USART_SendData(pUSARTx,temp_h);
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	// ���͵Ͱ�λ
	USART_SendData(pUSARTx,temp_l);
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}


// ����һ���ֽ�
uint16_t Usart_RecvByte( USART_TypeDef * pUSARTx){
	// UARTTimeout = UART_WAIT_TIME;
	UARTTimeout = 1000;
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_RXNE) == RESET){
		if(UARTTimeout-- == 0) return 0xFFFF;
	}
	return USART_ReceiveData(pUSARTx)&0x00FF;
}

// Ԥ�������ڵȴ���ʱ
uint16_t uart_error(void){
	return 0xFF;
}


// �ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
int fputc(int ch, FILE *f){
	// ����һ���ֽ����ݵ�����
	USART_SendData(UART_1, (uint8_t) ch);

	// �ȴ��������
	while (USART_GetFlagStatus(UART_1, USART_FLAG_TXE) == RESET);

	return (ch);
}


// �ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f){
	// �ȴ�������������
	while (USART_GetFlagStatus(UART_1, USART_FLAG_RXNE) == RESET);
	return (int)USART_ReceiveData(UART_1);
}
