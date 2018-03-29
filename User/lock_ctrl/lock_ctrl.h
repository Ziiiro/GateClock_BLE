#ifndef __LOCK_CTRL_H
#define __LOCK_CTRL_H
#include "stm32f10x.h"
#include "./data/data_def.h"
#include <stdio.h>
#include "./TSM12/keyboard.h"
#include "./Delay/delay.h"
#include "stm32f10x_conf.h"
#include  "./qs808/qs808_drive.h"
#include "./STMFLASH/stmflash.h"
#include "./OLED/oled.h"
#include "./UT588C/ut588c.h"
#include "./RC522/rc522_function.h"
#include "./RC522/rc522_config.h"
#include <stdlib.h>


#define USER_LOGIN_SUCCESS 0x00
#define USER_LOGIN_TIMEOUT  0x01
#define USER_LOGIN_BAD_FINGER	0x02
#define USER_LOGIN_DUPLICATION_FINGER 0x03
#define USER_BACK 0x04//��*����
#define VERIFF_SUCCESS 0x05
#define VERIFF_FAIL 0x06
#define USER_CONFIRM 0x07
#define USER_ID_REDUP 0x08
#define LOGIN_ERROR 0x09

void start_interface(void);     // �����桿��ȫ����ʾʱ��
uint8_t unlock(uint8_t key,uint8_t source,uint32_t RFCARD_ID);  // �����ܡ�������
uint8_t num_unlock_interface(uint8_t first_key, uint8_t* length,uint8_t disp_or_not);   // �����桿�����뿪��

void setting_interface(void);   // �����桿����֤����Ա
void setting_interface2 (void); // �����桿������ģʽ
void admin_settings(void);      // �����桿������Ա����
void nuser_settings(void);      // �����桿����ͨ�û�����
void system_settings(void);     // �����桿��ϵͳ����
void nurse_time_settings(void);	// �����桿����ķʱ������
void data_note(void);           // �����桿������ͳ�Ƽ���¼


// ���ײ㺯����
int first_time_init(void);
void time_disp_big(void);        // ȫ����ʾʱ��
void user_struct_store(uint16_t user_num,uint16_t * user_struct);
void user_delete(user_type my_user_type,uint16_t user_num);
void key_disp(uint8_t x,uint8_t y,uint8_t key,uint8_t clr);
void time_disp_bottom(void);    // ����Ļ��������ʾ��ǰʱ�䣬��ʽΪ��2018-3-5 11:18
void time_disp(void);
void back2factory(void);        // �ָ���������
void unlock_notes_write(u32 addr, uint16_t user_number);
void unlock_notes_set_all_ff(void);
void unlock_notes_set_all_flag_ff(void);
void unlock_notes_disp(u8 pos);
uint8_t password_verify_2admin(uint8_t key_length);
uint8_t password_verify_2unlock(uint8_t key_length, user_type *user_type_o);
uint8_t password_verify_only6(uint8_t key_length, user_type *user_type_o, uint8_t *key_ans);
uint8_t password_verify_only6_admin(uint8_t key_lengthm, uint8_t *key_ans);
uint8_t user_login(user_type my_user_type,uint16_t user_num);
uint8_t admin_mode(void);
uint8_t admin_verify(void);     // ��֤����Ա����
uint8_t user_id_cap(uint32_t * user_id,user_type * user_type_o);
uint8_t user_modify(uint16_t user_num);
uint8_t time_settings(void);    // ϵͳʱ������
uint8_t unlock_time_fun(void);  // ����ʱ�����ã����ڱ�ķ�û�
uint8_t time_verify(void);      // �жϵ�ǰʱ���Ƿ����ڿɿ���ʱ�Σ����ڱ�ķ�û�
uint8_t start_interface_key_cap(uint8_t first_key, uint8_t* length,uint8_t disp_or_not);

#endif
