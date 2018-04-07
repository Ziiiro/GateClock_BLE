#include "./OLED/oled.h"
#include <stdlib.h>
#include "./OLED/FONT_TAB.h"
//sda   PA15
//scl   PB3
//IO��������
#define SDA_IN()  {GPIOA->CRH&=0X0FFFFFFF;GPIOA->CRH|=(u32)8<<28;}
#define SDA_OUT() {GPIOA->CRH&=0X0FFFFFFF;GPIOA->CRH|=(u32)3<<28;}

//IO��������
#define IIC_SCL    PBout(3) //SCL
#define IIC_SDA    PAout(15) //SDA
#define READ_SDA   PAin(15)  //����SDA

static void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	IIC_SDA=1;
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low
	delay_us(4);
	IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ��������
}
static void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1;
	IIC_SDA=1;//����I2C���߽����ź�
	delay_us(4);
}
static u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����
	IIC_SDA=1;delay_us(1);
	IIC_SCL=1;delay_us(1);
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
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		IIC_SCL=1;
		delay_us(2);
		IIC_SCL=0;
		delay_us(2);
    }
	IIC_Wait_Ack();
}


static void I2C_Write_Data(u8 addr,u8 data)
{
    IIC_Start();
		IIC_Send_Byte(0x78);
    IIC_Send_Byte(addr);
		IIC_Send_Byte(data);
    IIC_Stop();
}

void WriteCmd(unsigned char I2C_Command)//д����
{
	I2C_Write_Data(0x00, I2C_Command);
}

void WriteDat(unsigned char I2C_Data)//д����
{
	I2C_Write_Data(0x40, I2C_Data);
}

void OLED_IIC_Init(void)//��ʼ��,ģ��II2C��ʹ�����ŵĸ��á�
{
	//��Ļ����
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(OLED_SCL_GPIO_CLK|OLED_SDA_GPIO_CLK, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

	GPIO_InitStructure.GPIO_Pin = OLED_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(OLED_SCL_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = OLED_SDA_PIN;
	GPIO_Init(OLED_SDA_GPIO_PORT, &GPIO_InitStructure);
	IIC_SDA=1;
	IIC_SCL=1;
}

void OLED_Init(void)
{
	OLED_IIC_Init();
	int i;
	for (i=0;i<=1000;i++);

	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //���ȵ��� 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	OLED_CLS();
	delay_ms(100);
}

// ������ʼ������
void OLED_SetPos(unsigned char x, unsigned char y){
	WriteCmd(0xb0+y);
	WriteCmd( ((x&0xf0)>>4)|0x10 );				//�еĸߵ�ַ
	WriteCmd( (x&0x0f)|0x00 );					//�еĵ͵�ַ
}

void OLED_Fill(unsigned char fill_Data)//ȫ�����
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);	//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
		{
			WriteDat(fill_Data);
		}
	}
}

void OLED_CLS(void)//����
{
	OLED_Fill(0x00);
}

void OLED_ON(void)
{
	WriteCmd(0X8D);  //���õ�ɱ�
	WriteCmd(0X14);  //������ɱ�
	WriteCmd(0XAF);  //OLED����
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          :
// Parameters     : none
// Description    : ��OLED���� -- ����ģʽ��,OLED���Ĳ���10uA
//--------------------------------------------------------------
void OLED_OFF(void){
	WriteCmd(0X8D);  //���õ�ɱ�
	WriteCmd(0X10);  //�رյ�ɱ�
	WriteCmd(0XAE);  //OLED����
}




void OLED_Show(unsigned char x, unsigned char y,char data1,char data2,uint8_t CNorEN)
/*��ʾ�����ַ����������֣�Ӣ�ģ����֣������ַ�
x:������ 0-127
y:������ 0-7
data1 ���ֵĸ�λ����Ӣ�ģ�
data2 ���ֵĵ�λ����ʾӢ��ʱ�����������Ч��
CNorEN: 0 ���� 1 Ӣ�ģ����֣������ַ�
*/
{
	unsigned char wm=0;
	OLED_SetPos(x , y);
	int addr = 0;
	int i=0;
	// ��ʾӢ��
	if (CNorEN){
		addr = (data1-32)*16;//-32����Ϊ�ֿ���ǰ32�� ������ʾ�ַ���û��
		for(wm = 0;wm < 8;wm++)
		{
			WriteDat(F8X16[addr]);
			addr += 1;
		}
		OLED_SetPos(x,y+1);
		for(wm = 0;wm < 8;wm++)
		{
			WriteDat(F8X16[addr]);
			addr += 1;
		}
	}
	// ��ʾ����
	else {
		while(i<sizeof(HZ16x16))
		{
			if((HZ16x16[i] == data1) && (HZ16x16[i+1] == data2))
			{
				addr = i+2;
				break;
			}
			i = i+34;
		}

		for(wm = 0;wm < 16;wm++)
		{
			WriteDat(HZ16x16[addr]);
			addr += 1;
		}
		OLED_SetPos(x,y+1);
		for(wm = 0;wm < 16;wm++)
		{
			WriteDat(HZ16x16[addr]);
			addr += 1;
		}
	}
}

/*��ʾ�����ַ������룬Ŀǰֻ����ʾ0~9
x:������ 0-127
y:������ 0-7
data �ַ���ʽ������
*/
void OLED_Show_inverse(unsigned char x, unsigned char y,char data)
{
	unsigned char wm=0;
	OLED_SetPos(x , y);
	int addr = 0;
	addr = (data-48)*16;
	for(wm = 0;wm < 8;wm++)
	{
		WriteDat(F8X16_inverse[addr]);
		addr += 1;
	}
	OLED_SetPos(x,y+1);
	for(wm = 0;wm < 8;wm++)
	{
		WriteDat(F8X16_inverse[addr]);
		addr += 1;
	}
}

/*��ʾһ������
����
//x��Χ 0 ~��127  y �ķ�Χ 0~7
X��Y��ʼ����
Disp_arr�����ӵ��������飬��bsp_i2c_oled.h�ļ�
length�����ӵĳ���
clr_or_not:�Ƿ�������1���� 0����
Ĭ����ʾ16*16,ֻ����ʾһ��*/
void Disp_sentence(uint8_t X,uint8_t Y,char *str,uint8_t clr_or_not)
{
	uint8_t X_now,Y_now;
	char * ptr;
	ptr = str;
	X_now = X;
	Y_now = Y;
	if (clr_or_not) OLED_CLS();
	// �ַ���β��
	while ((*ptr) != 0x00){
		// ��ʾ�Ĳ��Ǻ���
		if ((uint8_t)(*ptr) < 0x80){
			OLED_Show(X_now,Y_now,*ptr,*(ptr+1),1);
			X_now += 8;
			ptr+=1;
		}
		// ��ʾ���Ǻ���
		else {
			OLED_Show(X_now,Y_now,*ptr,*(ptr+1),0);
			X_now += 16;
			ptr += 2;
		}
	}
}
void Disp_sentence_singleline(uint8_t X,uint8_t Y,char *str,uint8_t clr_or_not)
{
	uint8_t X_now,Y_now;
	char * ptr;
	ptr = str;
	X_now = X;
	Y_now = Y;
	if (clr_or_not){
		OLED_SetPos(0,Y_now);
		for (u8 i=0; i<128; i++){
			WriteDat(0x00);
		}
		OLED_SetPos(0,Y_now+1);
		for (u8 i=0; i<128; i++){
			WriteDat(0x00);
		}
	}
	// �ַ���β��
	while ((*ptr) != 0x00){
		// ��ʾ�Ĳ��Ǻ���
		if ((uint8_t)(*ptr) < 0x80){
			OLED_Show(X_now,Y_now,*ptr,*(ptr+1),1);
			X_now += 8;
			ptr+=1;
		}
		// ��ʾ���Ǻ���
		else {
			OLED_Show(X_now,Y_now,*ptr,*(ptr+1),0);
			X_now += 16;
			ptr += 2;
		}
	}
}



static uint8_t code_111(uint8_t in)
{
	uint8_t code = 0x00;
	while(in>0)
	{
		code = (code<<1)|0x01;
		in--;
	}
	return code;
}
static uint8_t reverse(uint8_t in)
{
	int i=7;
	uint8_t code=0x00;
	while(i>0)
	{
		code  = (code|((in<<(7-i))&0x80))>>1;
		i--;
	}
	code  = (code|((in<<7)&0x80));
	return code;
}
//x ��ʼ������ 0~127
//y ��ʼ������ 0~63
//length  ���ȣ�
//hv 0��ʾ���ߣ� 1��ʾ����
void Disp_line(uint8_t xin,uint8_t yin,uint8_t length,uint8_t hv){
	uint8_t data = 0;
	int i=0;
	int j=0;
	uint8_t head,mid,tail;
	if (hv){
		if (length-1+yin > 63)
			length = 63+1-yin;
	}
	else {
		if (length-1+xin > 127)
			length = 127+1-xin;
	}
	if (hv){
		head = (yin/8+1)*8-yin;
		mid = (length - head)/8;
		tail = length - head - mid*8;
		OLED_SetPos(xin,yin/8);
		data = reverse(code_111(head));
		WriteDat(data);
		for (i=0;i<mid;i++){
			OLED_SetPos(xin,yin/8+1+i);
			WriteDat(0xff);
		}
		if (tail>0){
			OLED_SetPos(xin,yin/8+1+mid);
			data = code_111(tail);
			WriteDat(data);
		}
	}
	else{
		mid = length/8;
	  	tail = length - mid*8;
		OLED_SetPos(xin,yin/8);
		data = (yin/8+1)*8-yin;
		data = 0x80>>(data-1);

		OLED_SetPos(xin,yin/8);
		for (j=0;j<mid;j++)
			for(i=0;i<8;i++)
				WriteDat(data);
		OLED_SetPos(xin+mid*8,yin/8);
		for (i=0;i<8;i++){
			if(i>=tail)
				data = 0x00;
			WriteDat(data);
		}
	}
}

//name:		OLED��ʾ��ص�������
//func:		�����Ͻ���ʾ���״̬ͼ��
//param:	power:	��ص���[0~4],0Ϊȱ��,4Ϊ����
//return:	NONE
//brief:	NONE
void OLED_Show_Power(uint8_t power){
	OLED_SetPos(0,0);
	for (u8 i=0;i<111; i++){
		WriteDat(0x00);
	}
	OLED_SetPos(111,0);
	for(u8 i=0; i<17; i++){
		WriteDat(Logo_Power[power][i]);
	}
}

// ���ѹ��š�������ʾ��ȫ��д�룬ʹ��֮ǰ��������
void show_clock_open_big(void){
	uint16_t temp;
	// ��ʾͼ��
	for (u8 hang=0; hang<8; hang++){
		OLED_SetPos(0,hang);
		temp = hang*128;
		for (u8 lie=0; lie<128; lie++){
			if (hang<6){
				WriteDat( IMG_OPEN_CLOCK[temp+lie] );
			}
			else{
				WriteDat( 0x00 );
			}
		}
	}
}

// ���ѿ��š�������ʾ��ȫ��д�룬ʹ��֮ǰ��������
void show_clock_close_big(void){
	uint16_t temp;
	// ��ʾͼ��
	for (u8 hang=0; hang<8; hang++){
		OLED_SetPos(0,hang);
		temp = hang*128;
		for (u8 lie=0; lie<128; lie++){
			if (hang<6){
				WriteDat( IMG_CLOSE_CLOCK[temp+lie] );
			}
			else{
				WriteDat( 0x00 );
			}
		}
	}
}

// ��ָ��λ����ʾ2448��С�����֡�
// xin:	0~103
// yin: 0~2
int show_time_big(uint8_t xin,uint8_t yin,uint8_t numin){
	uint16_t temp,start;

	// ������Χ����ִ������
	if (numin>9){
		return 0;
	}
	start = numin*144;
	for (u8 i=0; i<6; i++){
		OLED_SetPos(xin,yin+i);
		temp = i*24;
		for (u8 lie=0; lie<24; lie++){
			WriteDat( IMG_NUM_2448[start+temp+lie] );
		}
	}
	return 1;
}

// ��ʾʱ������ϵ�����С��
void show_time_pot(void){
	uint16_t temp;
	for (u8 hang=0; hang<6; hang++){
		OLED_SetPos(52, hang+1);
		temp = hang*24;
		for (u8 lie=0; lie<24; lie++){
			WriteDat( IMG_POT[temp+lie] );
		}
	}
}

// ����ʾʱ������ϵ�����С��
void close_time_pot(void){
	for (u8 hang=0; hang<6; hang++){
		OLED_SetPos(52, hang+1);
		for (u8 lie=0; lie<24; lie++){
			WriteDat( 0x00 );
		}
	}
}

// ��ʾ����ͼ�� 16*16
void show_hammer(uint8_t xin,uint8_t yin){
	uint16_t temp;
	for (u8 hang=0; hang<2; hang++){
		OLED_SetPos(xin, yin+hang);
		temp = hang*16;
		for (u8 lie=0; lie<16; lie++){
			WriteDat( IMG_HAMMER[temp+lie] );
		}
	}
}

// ��ʾ*�����������
// length:		���뻺����������ĳ���
void Interface_Password(u8 length) {
	if (length>16) length = 16;
	u8 password[length+1];

	for (u8 i=0; i<length; i++) {
		password[i] = '*';
	}
	password[length] = 0x00;

	Disp_sentence_singleline(0, 2, (char*)password, 1);
}
