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





// �û������ݽṹ
typedef struct
{
	u16 m_USER_Number;	// ��ţ���000~999��
	u8  m_USER_Type;	// ���ͣ���0x00.û�м�¼����0x01.ָ�ơ���0x02.��Ƶ������0x03.���롿
	u32 m_USER_Data;	// ���ݣ�����m_USER_Type�����ݴ洢�����
}MY_USER;





/********************************************** �û����� *************************************************/
#define MY_USER_LENGTH			8			// һ���û����ݽṹ�Ĵ�С��ͨ�� sizeof �õ�����λ8���ֽ�
#define MY_USER_MAX_NUM			1000		// �����Դ洢120���û��ṹ��


/******************************************** ����ͨ�Ų��� ************************************************/
#define ERROR_CODE_SUCCESS		0x0000   	// ִ�гɹ�
#define ERROR_CODE_TIMEOUT		0x0500   	// �ɼ���ʱ


/********************************************* ��ʷ��¼���� **********************************************/
#define UNLOCK_NOTES_ADDR		0x800fd04	// �û�������¼�洢�׵�ַ
#define UNLOCK_NOTES_USED_FLAG  0x6666		// ���Ѿ�ռ�á�flag�������ͷ��16λ����Ϊ UNLOCK_NOTES_USED_FLAG����ô�ͱ����������������Ѿ���ռ����
#define UNLOCK_NOTES_MSG_LENGTH 5			// һ����Ϣ����2�ֽ�������һ����Ϣһ����Ҫд�� USED_FLAG year month_date hour_minute user_number  һ�� 5*16=80bit 10�ֽ����ݣ���Ϊÿ��д����16bitһд�����Ծ���5��16bit��5��2�ֽ�
#define UNLOCK_NOTES_SIZE		12			// �ܹ��洢����ʷ��Ϣ��������


/********************************************** �洢��ַ *************************************************/
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


#endif
