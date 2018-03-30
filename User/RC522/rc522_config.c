#include "./rc522/rc522_config.h"
#include "./Delay/delay.h"


static void             RC522_SPI_Config             ( void );

uint8_t RC522_usart_flag=0;
uint8_t RC522_rec;

void RC522_Init ( void )
{
	RC522_SPI_Config ();
	
	macRC522_Reset_Disable();
	delay_ms(10);	
	//macRC522_CS_Disable();
	PcdReset();
	PcdAntennaOff(); 
	M500PcdConfigISOType ( 'A' );//���ù�����ʽ//	

	WriteRawRC(SerialSpeedReg,0x7A);//�޸Ĳ�����
	delay_ms(1);//ע��������Ҫ��ʱ���޸Ĳ����ʺ�rc522��Ҫ��Ӧʱ��
//	USART_Cmd(RC522_USART, DISABLE);	
//	RCC_APB1PeriphClockCmd(RC522_USART_CLK, DISABLE);	
	RC522_USART_Init(115200);
//	u8 asd = ReadRawRC(SerialSpeedReg);

}

void RC522_SLEEP(void)
{
	uint8_t temp = ReadRawRC(CommandReg);
	WriteRawRC ( CommandReg, temp|0x10  );  
	
}

/***************************** macRC522_uart_io_SEL *****************************************/

//static void NVIC_Configuration(void)
//{
//  NVIC_InitTypeDef NVIC_InitStructure;
//  
//  /* Ƕ�������жϿ�������ѡ�� */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//  
//  /* ����USARTΪ�ж�Դ */
//  NVIC_InitStructure.NVIC_IRQChannel = RC522_IRQ_IRQChannel;
//  /* �������ȼ�Ϊ1 */
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//  /* �����ȼ�Ϊ1 */
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//  /* ʹ���ж� */
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  /* ��ʼ������NVIC */
//  NVIC_Init(&NVIC_InitStructure);
//}
void RC522_USART_Init(uint32_t BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_DeInit(RC522_USART);
/*uart1 rx*/
	GPIO_InitStructure.GPIO_Pin = RC522_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RC522_RX_GPIO_PORT, &GPIO_InitStructure);
/*uart1 tx*/	
	GPIO_InitStructure.GPIO_Pin = RC522_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RC522_TX_GPIO_PORT, &GPIO_InitStructure);
	

	/* ʹ�� UART ʱ�� */
  RCC_APB1PeriphClockCmd(RC522_USART_CLK, ENABLE);	
			
  /* ���ô��� ģʽ */
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(RC522_USART, &USART_InitStructure); 
	
//	NVIC_Configuration();
//	USART_ITConfig(RC522_USART, USART_IT_RXNE, ENABLE);	
  USART_Cmd(RC522_USART, ENABLE);	 

}

static void RC522_SPI_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*!< Configure uart_io pins: RST */
	RCC_APB2PeriphClockCmd (RC522_RST_GPIO_CLK, ENABLE);
  GPIO_InitStructure.GPIO_Pin = RC522_RST_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(RC522_RST_GPIO_PORT, &GPIO_InitStructure);
	macRC522_Reset_Disable();

	GPIO_InitStructure.GPIO_Pin = RC522_INT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(RC522_INT_GPIO_PORT, &GPIO_InitStructure);
  
	 
	RC522_USART_Init(9600u);
		
	
}
 
void RC522_USART_IRQHandler(void)                
	{
	u8 Res;   
	if(USART_GetITStatus(RC522_USART, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		Res =USART_ReceiveData(RC522_USART);	//��ȡ���յ�������
		RC522_usart_flag = 0xff;
		RC522_rec = Res;
//		printf("%x\n",Res);
		}
		
} 
