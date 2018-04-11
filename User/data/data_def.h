#ifndef __DATA_DEF_H
#define __DATA_DEF_H
#include "stm32f10x.h"


// // �û���ţ���1��ʼ�������û�ʹ��
// // ��Ҫ����
// // flash���ޣ�Ŀǰֻ�ܴ洢128���û�������������û����ʱ��ֻ����λ��Ҳ���Ǳ�Ŵ�01~99
// typedef struct//�û�׼��ʱ��
// {
// 	uint16_t year;
// 	uint8_t month;
// 	uint8_t date;
// 	uint8_t hour;
// 	uint8_t minute;
// 	uint8_t second;
// }USER_TIME;//�����ʱû�� �����Ϊ���Ժ���ÿ���˿���ʱ�䲻һ����Ƶ� Ŀǰ���б�ķ��ͳһ�Ŀ���ʱ�� �����Ǹ��ṹ���������õ�

// // �û�׼��ʱ��
// typedef struct
// {
// 	// ����
// 	uint8_t hour;
// 	uint8_t minute;
// 	// ����
// 	uint8_t hour2;
// 	uint8_t minute2;
// }UNLOCK_TIME;

// // �����û���ʷ��¼�ṹ��
// typedef struct{
// 	u16 flag;
// 	u16 year;
// 	u16 month_date;
// 	u16 hour_minute;
// 	u16 user_number;
// }UNLOCK_NOTES;

// // �����û���ʷ��¼�ṹ��
// typedef struct{
// 	UNLOCK_NOTES note1;
// 	UNLOCK_NOTES note2;
// 	UNLOCK_NOTES note3;
// 	UNLOCK_NOTES note4;
// 	UNLOCK_NOTES note5;
// 	UNLOCK_NOTES note6;
// 	UNLOCK_NOTES note7;
// 	UNLOCK_NOTES note8;
// 	UNLOCK_NOTES note9;
// 	UNLOCK_NOTES note10;
// 	UNLOCK_NOTES note11;
// 	UNLOCK_NOTES note12;
// }UNLOCK_NOTES_AREA;



// �û������ݽṹ(����ָ�ƺ���Ƶ��)
typedef struct{
	u16 m_USER_Number;	// ��ţ���000~999��
	u16 m_USER_Type;	// ���ͣ���0x0001.ָ�ơ���0x0002.��Ƶ����
	u32 m_USER_Data;	// ���ݣ�����m_USER_Type�����ݴ洢�����
}MY_USER;

// ��������ݽṹ
typedef struct{
	u16 m_Password_ID;		// ��ţ���000~999��
	u16 m_Type;				// ���������ã����ж�׼��ʱ�䣬��0x0001 ��ʱ�����ơ���0x0002 ʱ��Ρ���0x0003 ��ķ��
	u32 m_Password;			// ������ֵ
	u16 m_Year_Start;		// ׼�뿪ʼʱ��(��)(2018)
	u16 m_MonthAndDay_Start;// ׼�뿪ʼʱ��(����)(1111)
	u16 m_HourAndMin_Start;	// ׼�뿪ʼʱ��(ʱ��)(2359)
	u16 m_Year_End;			// ׼�����ʱ��(��)(2018)
	u16 m_MonthAndDay_End;	// ׼�����ʱ��(����)(1111)
	u16 m_HourAndMin_End;	// ׼�����ʱ��(ʱ��)(2359)
}STRUCT_PASSWORD;

// BLE �� �ֻ�ͨ�����ݰ�
typedef struct{
	u16 m_magicCode;	// 0xFECF
	u16 m_version;		// 0x0001
	u16 m_totalLength;	// 12+
	u16 m_cmdId;		// ��Ƭ����Ҫ�Ϳ������Ϣ,���ж�����ָ����Ҫ��Ƭ����ʲô
	u16 m_seq;			// None
	u16 m_errorCode;
}BleDataHead;



/********************************************** �û����� *************************************************/
#define MY_USER_LENGTH			8			// һ���û����ݽṹ�Ĵ�С��ͨ�� sizeof �õ�����λ8���ֽ�
#define MY_USER_MAX_NUM			1000		// �����Դ洢120���û��ṹ��
#define MY_USER_FINGER			0x0001		// ��ʾ���������ָ��
#define MY_USER_RFCARD			0x0002		// ��ʾ�����������Ƶ��
#define MY_PASSWORD_LENGTH		20			// һ���������ݽṹ�Ĵ�С
#define MY_PASSWORD_FREE		0x0001		// ��ʾ�������û��ʱ������
#define MY_PASSWORD_SECTION		0x0002		// ��ʾ��������ʱ������Ϊ��ʱ��Ρ�
#define MY_PASSWORD_NURSE		0x0003		// ��ʾ��������ʱ������λ����ķ��


/******************************************** ����ͨ�Ų��� ************************************************/
#define USART_RECVBUF_LENGTH	42			// ���ڻ���ش�С��12��ͷ+6����+2*12׼��ʱ��=12+6+24=42����������������
#define ERROR_CODE_SUCCESS		0x0000   	// ִ�гɹ�
#define ERROR_CODE_TIMEOUT		0x0500   	// �ɼ���ʱ
#define ERROR_CODE_ERROR		0x0800   	// �쳣
#define ERROR_CODE_DUPLICATION	0x0400   	// ��Ϣ�ظ�
#define ERROR_CODE_FINGERBAD	0x0300   	// ָ��������


/******************************************** ���ݰ�ͷ���� ************************************************/
#define MAGICCODE				0xFECF
#define VERSION					0x0001


/*********************************************** cmdID ***************************************************/
#define SYS_NO_ORDER			0x3333		// ��ʾû�н��յ���Ч�İ�ͷ
#define SYS_RECV_ORDER			0x6666		// ��ʾ���յ�����Ч�İ�ͷ
#define CMDID_ADD_FINGER		0x0301		// ���ָ������
#define CMDID_DEL_FINGER		0x0802		// ���ָ������
#define CMDID_ADD_RFCARD		0x0302		// �����Ƶ������
#define CMDID_DEL_RFCARD		0x0803		// ɾ����Ƶ������
#define CMDID_ADD_PASSWORD		0x0104		// �����������
#define CMDID_DEL_PASSWORD		0x0804		// ɾ����������


/********************************************** �洢��ַ *************************************************/
#define PASSWORD_ADDR_START		0x080092BE	// �洢�ֻ��˷���������ı�ŵ��׵�ַ
#define MY_USER_ADDR_START		0x0800E0BE	// �û��ṹ��洢�׵�ַ
#define MY_USER_ADDR_END		0x0800FFFF	// STM32 Flash ĩ��ַ




// �޸���Щ �ǵ��ڳ�ʼ����������������ʼ��
#define alluser_amount_addr		0x800fc00	// ���������û�����
#define admin_amount_addr 		0x800fc02	// �������Ա����
#define family_amount_addr 		0x800fc04	// �����û�
#define babysitter_amount_addr 	0x800fc06	// ��ķ�û�
#define speak_flag_addr 		0x800fc08	// ��Ƶflag
#define UNLOCK_TIME_addr 		0x800fd00	// ����ṹ����������ַ
#define UNLOCK_TIME_length 		0x04		// �ṹ�峤��
// дflash ��2BΪ��λ





void Userid2Ascii(u16 userid, u8* userid_ascii);
u16 Ascii2Userid(u8* userid_ascii);
u16 RecvBuf2Cmdid(void);
u16 RecvBuf2Userid(void);
void RecvBuf2TimeUnlock_SECTION(u16* result);

#endif


// /********************************************* ��ʷ��¼���� **********************************************/
// #define UNLOCK_NOTES_ADDR		0x800fd04	// �û�������¼�洢�׵�ַ
// #define UNLOCK_NOTES_USED_FLAG  0x6666		// ���Ѿ�ռ�á�flag�������ͷ��16λ����Ϊ UNLOCK_NOTES_USED_FLAG����ô�ͱ����������������Ѿ���ռ����
// #define UNLOCK_NOTES_MSG_LENGTH 5			// һ����Ϣ����2�ֽ�������һ����Ϣһ����Ҫд�� USED_FLAG year month_date hour_minute user_number  һ�� 5*16=80bit 10�ֽ����ݣ���Ϊÿ��д����16bitһд�����Ծ���5��16bit��5��2�ֽ�
// #define UNLOCK_NOTES_SIZE		12			// �ܹ��洢����ʷ��Ϣ��������
