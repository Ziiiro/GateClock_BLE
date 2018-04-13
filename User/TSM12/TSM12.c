#include <stdio.h>
#include "./TSM12/TSM12.h"

// ��TSM12�����ݴ���оƬ

//PB13->SCL
//PB12->SDA
//��ʼ��IIC

#define SDA_IN()  {GPIOB->CRH&=0XFFF0FFFF;GPIOB->CRH|=(u32)8<<16;}
#define SDA_OUT() {GPIOB->CRH&=0XFFF0FFFF;GPIOB->CRH|=(u32)3<<16;}

//IO��������
#define IIC_SCL    PBout(13) //SCL
#define IIC_SDA    PBout(12) //SDA
#define READ_SDA   PBin(12)  //����SDA

static void TSM12_PIN_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	TSM12_SDA_GPIO_CLK|TSM12_SCL_GPIO_CLK|TSM12_RST_GPIO_CLK|TSM12_I2CEN_GPIO_CLK, ENABLE );


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = TSM12_SDA_PIN;
	GPIO_Init(TSM12_SDA_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(TSM12_SDA_GPIO_PORT,TSM12_SDA_PIN);	//sda����

	GPIO_InitStructure.GPIO_Pin = TSM12_SCL_PIN;
	GPIO_Init(TSM12_SCL_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(TSM12_SCL_GPIO_PORT,TSM12_SCL_PIN);	//scl����











	GPIO_InitStructure.GPIO_Pin = TSM12_RST_PIN;
	GPIO_Init(TSM12_RST_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(TSM12_RST_GPIO_PORT,TSM12_RST_PIN);//��λ���ͣ�����λ













	GPIO_InitStructure.GPIO_Pin = TSM12_I2CEN_PIN;
	GPIO_Init(TSM12_I2CEN_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(TSM12_I2CEN_GPIO_PORT,TSM12_I2CEN_PIN);//��TSM12 i2c����
	//�ж�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = TSM12_INT_PIN;
	GPIO_Init(TSM12_INT_GPIO_PORT, &GPIO_InitStructure);

	//�жϳ�ʼ��
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
 	GPIO_EXTILineConfig(TSM12_INT_PIN_SOURCE_PORT,TSM12_INT_PIN_SOURCE_PIN);
	EXTI_InitStructure.EXTI_Line=TSM12_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TSM12_INT_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x04;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

//����IIC��ʼ�ź�
static void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	IIC_SDA=1;
	IIC_SCL=1;
	delay_us(5);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low
	delay_us(5);
	IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ��������
}

//����IICֹͣ�ź�
static void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(5);
	IIC_SCL=1;
	IIC_SDA=1;//����I2C���߽����ź�
	delay_us(5);
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
static u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����
	IIC_SDA=1;delay_us(5);
	IIC_SCL=1;delay_us(5);
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//ʱ�����0
	return 0;
}
//����ACKӦ��
static void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(5);
	IIC_SCL=1;
	delay_us(5);
	IIC_SCL=0;
}
//������ACKӦ��
static void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(5);
	IIC_SCL=1;
	delay_us(5);
	IIC_SCL=0;
}
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
static void IIC_Send_Byte(u8 txd)
{
    u8 t;
	SDA_OUT();
    IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {
    //IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			IIC_SDA=1;
		else
			IIC_SDA=0;
		txd<<=1;
		delay_us(5);
		IIC_SCL=1;
		delay_us(5);
		IIC_SCL=0;
		delay_us(5);
    }
}
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
static u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0;
        delay_us(5);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;
		delay_us(1);
    }
    if (!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK
    return receive;
}

u16 TTP229_ReadOneByte(void)//��һ������
{
	u8 high=0,low=0;
	u16 key_value=0;
	IIC_Start();
	IIC_Send_Byte(0XAF);		//ttp259  �Ķ���ַ��ֻ���ܶ���ַ��
	IIC_Wait_Ack();


  low=IIC_Read_Byte(1);
	high=IIC_Read_Byte(0);
	key_value=(high<<8)|low;		   	//ʮ������Ҫ�����ֽ����ݣ��˼�ֻ��Ҫһ���ֽ�����
	IIC_Stop();
	return key_value;
}

uint8_t TSM12_reg_write(uint8_t address,uint8_t value)
{
	IIC_Start();
	IIC_Send_Byte(0xD0);//д��ַ
	if(IIC_Wait_Ack())
		return 0x01;
	IIC_Send_Byte(address);//�Ĵ�����ַ
	if(IIC_Wait_Ack())
		return 0x01;
	IIC_Send_Byte(value);//�Ĵ���ֵ
	if(IIC_Wait_Ack())
		return 0x01;
	IIC_Stop();
	return 0x00;
}

uint8_t TSM12_reg_read(uint8_t address)//i2cͨ��ʧ�� ��ֱ�ӷ���0x00 ģ��û�а��¼���
{
	uint8_t value;
	IIC_Start();
	IIC_Send_Byte(0xD0);//д��ַ
	if(IIC_Wait_Ack())
		return 0x00;
	IIC_Send_Byte(address);//�Ĵ�����ַ
	if(IIC_Wait_Ack())
		return 0x00;
	IIC_Stop();

	IIC_Start();
	IIC_Send_Byte(0xD1);//����ַ
	if(IIC_Wait_Ack())
		return 0x00;
	value = IIC_Read_Byte(0);
	IIC_Stop();
	return value;

}
void TSM12_SLEEP(void)
{
	//���� ���� �ر�ͨ�� �������� �����жϲ��ܲ��� ������
	TSM12_reg_write(CTR1,0x23);
	TSM12_reg_write(CTR2,0x07);//�����߹���
//	TSM12_reg_write(Ch_hold1,0xff);//ͨ��ȫ���ر�
//	TSM12_reg_write(Ch_hold2,0x0f);
	GPIO_SetBits(TSM12_I2CEN_GPIO_PORT,TSM12_I2CEN_PIN);//�ر�i2c����
}
void TSM12_Wakeup(void)
{
	GPIO_ResetBits(TSM12_I2CEN_GPIO_PORT,TSM12_I2CEN_PIN);//��i2c
	TSM12_reg_write(CTR2,0x03);//�ر����߹���
	TSM12_reg_write(CTR1,0x80);
}
void TSM12_Init(void)
{
	TSM12_PIN_Init();
	//rst
	delay_ms(100);
	GPIO_SetBits(TSM12_RST_GPIO_PORT,TSM12_RST_PIN);//��λ
	delay_ms(100);
	GPIO_ResetBits(TSM12_RST_GPIO_PORT,TSM12_RST_PIN);//��λ����
	delay_ms(100);
	//reg config
	TSM12_reg_write(CTR2,0x0b);//��λ,�������ߣ�������Ӧ�ٶȿ�
	TSM12_reg_write(CTR2,0x07);


	TSM12_reg_write(Sens1,0x88);//ÿ4bitһ��ͨ�� ��λ��ʾ��Ӧ��Χ ����λ��ʾ������ ����ԽСԽ����[0x88]
	TSM12_reg_write(Sens2,0x88);
	TSM12_reg_write(Sens3,0x88);
	TSM12_reg_write(Sens4,0x88);
	TSM12_reg_write(Sens5,0x88);
	TSM12_reg_write(Sens6,0x88);



	TSM12_reg_write(CTR1,0x80);//���Ҫ���ó�80����Ȼ��Ӧ��
	TSM12_reg_write(Ref_rst1,0x00);//���òο�ֵ����������������һ�£���֮��Ҫ���ó�0 ��Ȼ������
	TSM12_reg_write(Ref_rst2,0x00);
	TSM12_reg_write(Cal_hold1,0xff);//У׼ʹ�ܣ����ÿ���������
	TSM12_reg_write(Cal_hold2,0x0f);
	TSM12_reg_write(Ch_hold1,0x00);//ͨ��ʹ�ܣ�0��ʹ�� 1��ʧ��
	TSM12_reg_write(Ch_hold2,0x00);
}

uint8_t TMS12_ReadOnKey(void) {
	uint8_t k1,k2,k3;
	uint32_t KEY;
	uint32_t mask=0x00000003;	// ���룬ȡ��KEY�а��µļ�ֵ
	uint8_t i=0;

	k1=TSM12_reg_read(Output1);
	k2=TSM12_reg_read(Output2);
	k3=TSM12_reg_read(Output3);
	KEY=(k1<<16)|(k2<<8)|(k3);

	// ɨ�����ް���������
	for (i=0;i<12;i++) {
		if(KEY == mask)	return i;
		mask=mask<<2;
	}

	// ���ܵ�����˵��û�а��������£���ֱ�� return KEY_NULL
	return KEY_NULL;
}
