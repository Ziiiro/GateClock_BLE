#ifndef __DATA_DEF_H
#define __DATA_DEF_H
#include "stm32f10x.h"
// �û���ţ���1��ʼ�������û�ʹ��
// ��Ҫ����
// flash���ޣ�Ŀǰֻ�ܴ洢128���û�������������û����ʱ��ֻ����λ��Ҳ���Ǳ�Ŵ�01~99
typedef struct//�û�׼��ʱ��
{
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}USER_TIME;//�����ʱû�� �����Ϊ���Ժ���ÿ���˿���ʱ�䲻һ����Ƶ� Ŀǰ���б�ķ��ͳһ�Ŀ���ʱ�� �����Ǹ��ṹ���������õ�

// �û�׼��ʱ��
typedef struct
{
	// ����
	uint8_t hour;
	uint8_t minute;
	// ����
	uint8_t hour2;
	uint8_t minute2;
}UNLOCK_TIME;


// error ��Ԥ����debug�õ�
typedef enum user_type
{admin=1,family,babysitter,temporary,error} user_type;

#define user_flag_value 0xAA

// �û������ݽṹ
typedef struct
{
	uint8_t	flag;				// �ǼǱ�ǣ�flag=0xAAʱ����ʾ¼�����û�������û��¼��
	user_type my_user_type;		// �û����� ����Ա ���� ��ķ ���� ��ʱ
	uint16_t number;			// �û����,3λ��
	uint8_t password_length;	// ���볤��
	uint8_t finger_number;		// ָ�Ʊ��,��1��ʼ
	uint8_t password[102];		// �������飬�֧��29λ,����106��Ϊ��ʹ�ṹ�峤��Ϊ128����Ҫ�ټӳ�Աʱ�����������鳤�����̼���
	uint32_t rfcard_id;			// ��Ƶ��id
	USER_TIME time_start;		// ׼��ʱ�俪ʼ �����ʱû�� �����Ϊ���Ժ���ÿ���˿���ʱ�䲻һ����Ƶ� Ŀǰ���б�ķ��ͳһ�Ŀ���ʱ��
	USER_TIME time_stop;		// ׼��ʱ�����
}MY_USER;
#define MY_USER_length 128		// ��Ӧ��ʮ�����ƾ��� 80w

// �����û���ʷ��¼�ṹ��
typedef struct{
	u16 flag;
	u16 year;
	u16 month_date;
	u16 hour_minute;
	u16 user_number;
}UNLOCK_NOTES;

// �����û���ʷ��¼�ṹ��
typedef struct{
	UNLOCK_NOTES note1;
	UNLOCK_NOTES note2;
	UNLOCK_NOTES note3;
	UNLOCK_NOTES note4;
	UNLOCK_NOTES note5;
	UNLOCK_NOTES note6;
	UNLOCK_NOTES note7;
	UNLOCK_NOTES note8;
	UNLOCK_NOTES note9;
	UNLOCK_NOTES note10;
	UNLOCK_NOTES note11;
	UNLOCK_NOTES note12;
}UNLOCK_NOTES_AREA;







// ÿҳ8���û�����
#define USER_PER_PAGE 8

#define MY_USER_addr_base 0x800c000		// �ӵ�49ҳ��ʼ���û��ṹ��
#define MY_USER_addr_offset MY_USER_length*USER_PER_PAGE
#define INIT_ADDR 0x800fc10				// ���һ���ؼ��֣�ÿ���û����ã�����������ؼ��֣����ڱ�ʶ�Ƿ��һ��ʹ��
#define INIT_WORD 0xAA55				// INIT_ADDR��ŵĳ�ʼ���ؼ���
#define MY_USER_addr_end 0x800f800		// ���һ�����û��ṹ���ҳ����63ҳ
#define MY_USER_MAX_NUM 120 			// �����Դ洢120���û��ṹ��

// �޸���Щ �ǵ��ڳ�ʼ����������������ʼ��
#define alluser_amount_addr		0x800fc00	// ���������û�����
#define admin_amount_addr 		0x800fc02	// �������Ա����
#define family_amount_addr 		0x800fc04	// �����û�
#define babysitter_amount_addr 	0x800fc06	// ��ķ�û�
#define speak_flag_addr 		0x800fc08	// ��Ƶflag
#define UNLOCK_TIME_addr 		0x800fd00	// ����ṹ����������ַ
#define UNLOCK_TIME_length 		0x04		// �ṹ�峤��
// дflash ��2BΪ��λ

#define UNLOCK_NOTES_ADDR		0x800fd04	// �û�������¼�洢�׵�ַ
#define UNLOCK_NOTES_USED_FLAG  0x6666		// ���Ѿ�ռ�á�flag�������ͷ��16λ����Ϊ UNLOCK_NOTES_USED_FLAG����ô�ͱ����������������Ѿ���ռ����
#define UNLOCK_NOTES_MSG_LENGTH 5			// һ����Ϣ����2�ֽ�������һ����Ϣһ����Ҫд�� USED_FLAG year month_date hour_minute user_number  һ�� 5*16=80bit 10�ֽ����ݣ���Ϊÿ��д����16bitһд�����Ծ���5��16bit��5��2�ֽ�
#define UNLOCK_NOTES_SIZE		12			// �ܹ��洢����ʷ��Ϣ��������


#endif
