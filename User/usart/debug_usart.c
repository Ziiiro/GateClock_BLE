#include "./usart/debug_usart.h"
#include <stdio.h>

static volatile uint32_t  UARTTimeout = UART_WAIT_TIME;

extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[12];
extern u8 USART1_RecvBuf_Length;


static void NVIC_Configuration(void)
{
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

void debug_usart_init(void)
{
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
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

	// ʹ�ܴ���
	USART_Cmd(UART_1, ENABLE);
	
}

/********************************************** �û����� *************************************************/

// �����ֻ�������
// result:	�ֻ���cmdID
u16 Usart_RecvOrder(USART_TypeDef* pUSARTx) {
	u16 result=0x0000;
	// ������ڽ��յ�����
	if(USART_Recv_Flag==1) {
		// ��ձ�־λ
		USART_Recv_Flag = 0;

		// ����16λcmdID
		result = USART_RecvBuf[6];
		result = result<<8;
		result = result | USART_RecvBuf[7];

		// ���RecBuf���ݳ���
		USART1_RecvBuf_Length = 0;
	}
	return result;
}

// ���͡��û�ID��
// pUSARTx:		���ں�
// user_id:		�û�id
// notes:		12=0x303132 ��ʽ
void Usart_SendUserId(USART_TypeDef* pUSARTx, u16 user_id) {
	u8 temp[3];
	Userid2Ascii(user_id,temp);

	for (u8 i=0; i<3; i++){
		pUsart_SendByte(pUSARTx, temp+i);
	}
}

// ���͡���Ƶ����¼��ɹ����ݰ�
// pUSARTx:		���ں�
// user_id:		�û�id
void Usart_SendIC_Success(USART_TypeDef* pUSARTx, u16 user_id) {
	// �����ͷ����
	BleDataHead temp;
	temp.m_magicCode = magicCode;
	temp.m_version = version;
	temp.m_totalLength = 15;
	temp.m_cmdId = cmdId_IC;
	temp.m_seq = 0x0000;
	temp.m_errorCode = errorCode0;

	// ���Ͱ�ͷ����
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}

	// �����û� ID
	Usart_SendUserId(pUSARTx, user_id);
}

// ���͡���Ƶ����¼��ʧ�����ݰ�
// pUSARTx:		���ں�
void Usart_SendIC_Error(USART_TypeDef* pUSARTx) {
	// �����ͷ����
	BleDataHead temp;
	temp.m_magicCode = magicCode;
	temp.m_version = version;
	temp.m_totalLength = 12;
	temp.m_cmdId = cmdId_IC;
	temp.m_seq = 0x0000;
	temp.m_errorCode = errorCode9;

	// ���Ͱ�ͷ����
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}
}














/********************************************** �ײ㺯�� *************************************************/

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

// ����һ���ֽڣ��ر�ע�⣬���ڼĴ���ֻ�е�9λ��Ч����������
uint16_t Usart_RecvByte( USART_TypeDef * pUSARTx){

	// ���ر�ע�⡿������ط�һ��Ҫ��FFFFF��ô������ݣ�������1000��ôС��
	UARTTimeout = UART_WAIT_TIME;
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_RXNE) == RESET){
		if(UARTTimeout-- == 0) return uart_error();
	}
	return USART_ReceiveData(pUSARTx)&0x00FF;
}

// Ԥ�������ڵȴ���ʱ
uint16_t uart_error(void){
	return 0xFFFF;
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
