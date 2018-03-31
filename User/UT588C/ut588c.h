#ifndef UT588C_H
#define UT588C_H
#include "stm32f10x.h"

//TG1
#define TG1_PORT GPIOB
#define TG1_GPIO_CLK      RCC_APB2Periph_GPIOB
#define TG1_PIN GPIO_Pin_5
//TG2
#define TG2_PORT GPIOB
#define TG2_GPIO_CLK      RCC_APB2Periph_GPIOB
#define TG2_PIN GPIO_Pin_4


#define	TG1 PBout(5)
#define	TG2 PBout(4)

// ������ʾ��Ч
#define SPEAK_OPEN_THE_DOOR()		UT588C_speak(4)
// ����ʧ����ʾ��Ч
#define SPEAK_OPEN_THE_DOOR_FAIL()	UT588C_speak(11)
// �����ɹ���ʾ��
#define SPEAK_OPT_SUCCESS() 		UT588C_speak(22)
// ����ʧ����ʾ��
#define SPEAK_OPT_FAIL() 			UT588C_speak(23)
// ������Ч
#define SPEAK_DUDUDU() 				UT588C_speak(5)
// �밴��ָ
#define SPEAK_PRESS_FIGER()			UT588C_speak(24)
// �밴��ָ��ˢ��������������
#define SPEAK_CARD_FIGER_OR_PW()	UT588C_speak(25)
// ����¼��һ������Ա
#define SPEAK_NO_ADMIN_NOW() 		UT588C_speak(28)


// ��֪������
#define SPEAK_VERIFY_ADMIN() 		UT588C_speak(6)
#define SPEAK_PRESS_FIGER_AG() 		UT588C_speak(6)
#define SPEAK_NUMBER_INPUT()		UT588C_speak(6)



void UT588C_init(void);
void UT588C_speak(uint8_t count);   


#endif 
