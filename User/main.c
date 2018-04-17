#include "./my_board.h"


extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern u8 USART1_RecvBuf_Length;
extern u8 WAKEUP_SOURCE;
extern QS808_Rec_Buf_type QS808_Rec_Buf;
extern _calendar_obj calendar;
extern u8 BLE_MAC[17];

float Battery_quantity;
u8 SLEEP_MAX = 30;


void Interface(void);


int main(void) {

	delay_init();			// ��ϵͳʱ�ӡ���ʼ��
	RTC_Init();				// ��RTCʱ�ӡ���ʼ��
	TSM12_Init();			// ����������оƬ����ʼ��
	BLE_init();				// ����������ʼ��
	debug_usart_init();		// �����ڡ���ʼ��
	power_ctrl_init();		// ����Դ���ơ���ʼ��
	Power_ctrl_on();		// �򿪵�Դ����
	RC522_Init();			// ����Ƶ��оƬ����ʼ��
	TIM3_Int_Init(99,7199);	// ��ʱ��3�����Ժ�������ʱ����,10KƵ�ʼ���,ÿ10msһ���ж�
	QS808_Init();			// ��ָ�Ʋɼ�ͷ����ʼ��
	Gate_Init();			// ��������е���ơ���ʼ��
	OLED_Init();			// ��OLED����ʼ��
	delay_ms(100);
	VCC_Adc_Init();			// ��ADC��ͨ����ʼ��
	UT588C_init();			// ������оƬ����ʼ��
	NewLed_Init();			// �������ơ���ʼ��
	Init_BLE_MAC();			// ������MAC����ַ��ʼ��
	// QS808_CMD_DEL_ALL();	// ɾ��ȫ��ָ��


	u8 work_flag;
	u16 temp_cmdid, temp_userid, temp_return, sleep_count=0;
	u32 temp_RFCARD_ID;
	Interface();
	LED_OFF2ON();


	while(1) {
		// ˯�߼���++
		// sleep_count++;

		// ��ʾһ��������
		if (work_flag==1) {
			work_flag = 0;
			Interface();
		}

		// �ж��Ƿ�ý���˯��ģʽ
		if (sleep_count>=SLEEP_MAX && QS808_Rec_Buf.Trans_state==reset) {
			sleep_count = 0;
			QS808_Rec_Buf_refresh();
			Disp_sentence(48,2,"����",1);
			delay_ms(500);
			PWR_Standby_Mode();

			// ����֮������￪ʼִ�г�����ִ��һ��ָ��ɨ�裬���ٿ�������
			LED_OFF2ON();

			// �����ָ��ͷ����
			if(WAKEUP_SOURCE==0) {
				// �����⵽����ָ���£��Ϳ�ʼ���ָ����ȷ�ԣ�׼������
				if (QS808_CMD_FINGER_DETECT()==ERR_FINGER_DETECT) {
					sleep_count = SLEEP_MAX;
					temp_return = Confirm_Finger();
					if (temp_return==ERROR_CODE_SUCCESS) {
						show_clock_open_big();
						SPEAK_OPEN_THE_DOOR();
						Gate_Unlock();
					}
					else {
						Disp_sentence(28,2,"��֤ʧ��",1);
						SPEAK_OPEN_THE_DOOR_FAIL();
						LED_OpenError();
						delay_ms(800);	// ��֤ʧ�ܵ���ʾʱ��
					}
				}
			}
			else {
				Interface();
			}
		}

		// ������յ������ݴ��룬˵���ֻ��˷�������Ϣ������Ҫ������Ϣ¼�����һ������
		if ( Usart_RecvOrder(USART1)==SYS_RECV_ORDER ) {
			// ���� temp_cmdid �����з�֧�ж�
			temp_cmdid = RecvBuf2Cmdid();
			switch( temp_cmdid ) {
				/************************* ���յ�������ʱ�䡿ָ�� *************************/
				case CMDID_SET_TIME:
					sleep_count = 0;
					SPEAK_DUDUDU();
					Cogradient_Time();
					break;

				/************************* ���յ������ָ�ơ�ָ�� *************************/
				case CMDID_ADD_FINGER:
					sleep_count = 0;
					SPEAK_DUDUDU();
					if(Add_Finger(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_ADD_Success(USART1, 3, temp_userid);
					// ��ʱһ��ʱ�䣬��ֱֹ�ӽ���ָ�ƽ�������
					delay_ms(1000);
					break;


				/************************* ���յ���ɾ��ָ�ơ�ָ�� *************************/
				case CMDID_DEL_FINGER:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_Finger(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_DEL_Success(USART1);
					else
						Usart_SendFinger_DEL_Error(USART1);
					break;


				/************************ ���յ��������Ƶ����ָ�� ************************/
				case CMDID_ADD_RFCARD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_return = Add_RFCard(&temp_userid);
					if(temp_return== ERROR_CODE_SUCCESS)
						Usart_SendRFCard_ADD_Success(USART1, temp_userid);
					else
						Usart_SendRFCard_ADD_Error(USART1, temp_return);
					// ��ʱһ��ʱ�䣬��ֱֹ�ӽ�����Ƶ����������
					delay_ms(1000);
					break;


				/************************ ���յ���ɾ����Ƶ����ָ�� ************************/
				case CMDID_DEL_RFCARD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_RFCard(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendRFCard_DEL_Success(USART1);
					else
						Usart_SendRFCard_DEL_Error(USART1);
					break;


				/************************* ���յ���������롿ָ�� *************************/
				case CMDID_ADD_PASSWORD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_return = Add_Password(&temp_userid);
					if(temp_return== ERROR_CODE_SUCCESS)
						Usart_SendPassword_ADD_Success(USART1, temp_userid);
					else
						Usart_SendPassword_ADD_Error(USART1, temp_return);
					break;


				/************************* ���յ���ɾ�����롿ָ�� *************************/
				case CMDID_DEL_PASSWORD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_Password(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendPassword_DEL_Success(USART1);
					else
						Usart_SendPassword_DEL_Error(USART1);
					break;


				/************************* ���յ���һ��������ָ�� *************************/
				case CMDID_OPEN_DOOR:
					sleep_count = 0;
					show_clock_open_big();
					Gate_Unlock();
					SPEAK_OPEN_THE_DOOR();
					// һ�㿪�����ǳɹ��ģ�Ҳû�а취��⿪���Ƿ�ɹ���������ֱ�ӷ��ؿ����ɹ�������
					Usart_SendOpenDoor_Success(USART1);
					break;


				/************************************************************************/
				default:
					break;
			}
		}

		// ���û�н��յ��ֻ��˷�������Ϣ���Ǿ�ʱ��׼������
		else {
			// �����⵽����ָ���£��Ϳ�ʼ���ָ����ȷ�ԣ�׼������
			if (QS808_CMD_FINGER_DETECT()==ERR_FINGER_DETECT) {
				sleep_count = 0;
				work_flag = 1;
				temp_return = Confirm_Finger();
				if (temp_return==ERROR_CODE_SUCCESS) {
					show_clock_open_big();
					SPEAK_OPEN_THE_DOOR();
					Gate_Unlock();
				}
				else {
					Disp_sentence(28,2,"��֤ʧ��",1);
					LED_OpenError();
					SPEAK_OPEN_THE_DOOR_FAIL();
				}
				// ��ֹ��ʱ�����ٴν���ָ�Ƽ��
				delay_ms(800);
			}

			// �����⵽����Ƶ���������Ϳ�ʼ�����Ƶ������ȷ�ԣ�׼������
			if (RFCard_test(&temp_RFCARD_ID) == RFCARD_DETECED) {
				sleep_count = 0;
				work_flag = 1;
				temp_return = Confirm_RFCard(temp_RFCARD_ID);
				if (temp_return==ERROR_CODE_SUCCESS) {
					show_clock_open_big();
					SPEAK_OPEN_THE_DOOR();
					Gate_Unlock();
				}
				else {
					Disp_sentence(28,2,"��֤ʧ��",1);
					SPEAK_OPEN_THE_DOOR_FAIL();
					LED_OpenError();
				}
				// ��֤ʧ����ʾʱ��
				delay_ms(800);
			}

			// �����⵽�а������£��ͽ�������������棬׼������
			if (TMS12_ReadOnKey() != KEY_NULL) {
				sleep_count = 0;
				work_flag = 1;
				u8 password_buf[LENGTH_KEY_BUF], buf_length=0, last_press, temp;
				SPEAK_DUDUDU();
				OLED_CLS();

				// �����������
				Battery_quantity = Get_Battery();
				if (Battery_quantity<3.7)		OLED_Show_Power(0);
				else if (Battery_quantity<4.2)	OLED_Show_Power(1);
				else if (Battery_quantity<4.7)	OLED_Show_Power(2);
				else if (Battery_quantity<5.2)	OLED_Show_Power(3);
				else 							OLED_Show_Power(4);
				Disp_sentence(24,2,"����������",0);

				Create_NewPasswordBuf(password_buf);
				while(1) {
					// ���°���������X
					temp = Update_KeyBuf(password_buf, &buf_length, &last_press);
					// ������������������룬����һ��
					if (temp==PASSWORD_CODE_INPUTTING) {
						SPEAK_DUDUDU();
						Interface_Password(buf_length);
					}
					// ����������볬ʱ���������˳�����ֱ��break
					else if (temp==PASSWORD_CODE_TIMEOUT || temp==PASSWORD_CODE_QUIT) {
						SPEAK_DUDUDU();
						OLED_CLS();
						break;
					}
					// �������������ɣ��Ϳ�ʼ��֤�����׼ȷ��
					else if (temp==PASSWORD_CODE_COMPLETE) {
						SPEAK_DUDUDU();
						if (Confirm_Password(password_buf, buf_length)==ERROR_CODE_SUCCESS) {
							show_clock_open_big();
							SPEAK_OPEN_THE_DOOR();
							Gate_Unlock();
						}
						else {
							Disp_sentence(28,2,"��֤ʧ��",1);
							SPEAK_OPEN_THE_DOOR_FAIL();
							LED_OpenError();
						}
						// ����������ʾ
						delay_ms(800);
						break;
					}
					// ���볤�Ȳ���6λ
					else if (temp==PASSWORD_CODE_SHORT) {
						SPEAK_DUDUDU();
						Disp_sentence(28,2,"��֤ʧ��",1);
						SPEAK_OPEN_THE_DOOR_FAIL();
						LED_OpenError();
						// ����������ʾ
						delay_ms(800);
						break;
					}
				}
			}
		}
	}
}

// ����֮����ʾ���˻���������
void Interface(void) {
	char disp1[11];

	OLED_CLS();

	// ��һ����ʾ����ص�����
	Battery_quantity = Get_Battery();
	if (Battery_quantity<3.7)		OLED_Show_Power(0);
	else if (Battery_quantity<4.2)	OLED_Show_Power(1);
	else if (Battery_quantity<4.7)	OLED_Show_Power(2);
	else if (Battery_quantity<5.2)	OLED_Show_Power(3);
	else 							OLED_Show_Power(4);

	// ���ȱ���Ǿ�ֱ����ʾ���������ء�
	if (Battery_quantity<=4.2) {
		Disp_sentence(23,2,"��������",0);
	}
	// �����ȱ�磬�Ǿ���ʾ����ӭʹ�á���������ʾʱ����Ϣ
	else {
		Disp_sentence(32,2,"��ӭʹ��",0);

		// ��������ʾ���£���ʽ�磺2018-02-28
		disp1[0] = 48 + calendar.w_year/1000;
		disp1[1] = 48 + (calendar.w_year/100)%10;
		disp1[2] = 48 + (calendar.w_year/10)%10;
		disp1[3] = 48 + (calendar.w_year)%10;
		disp1[4] = '-';
		disp1[5] = 48 + (calendar.w_month)/10;
		disp1[6] = 48 + (calendar.w_month)%10;
		disp1[7] = '-';
		disp1[8] = 48 + (calendar.w_date)/10;
		disp1[9] = 48 + (calendar.w_date)%10;
		disp1[10] = 0;	// ���������
		Disp_sentence_singleline(23,4,disp1,0);

		// ��������ʾСʱ�ͷ��ӣ�18:31
		disp1[0] = 48 + (calendar.hour)/10;
		disp1[1] = 48 + (calendar.hour)%10;
		disp1[2] = ':';
		disp1[3] = 48 + (calendar.min)/10;
		disp1[4] = 48 + (calendar.min)%10;
		disp1[5] = 0;	// ���������
		Disp_sentence_singleline(43,6,disp1,0);
	}
}






#if 0

// ���� ���ظ��ַ�����ִ�
int lengthOfLongestSubstring(char* s) {
    int maxlen = 0, s_len = 0, s_start = 0, index = 0;
    if (NULL == s) return;

    while (s[index] != '\0') {
        // search for char in window
        int i;
        for (i = s_start; i < index; i++) {
            if (s[i] == s[index]) {
                if (s_len > maxlen)
                    maxlen = s_len;
                    s_start = i+1;
                    s_len = index - s_start;
                break;
            }
        }
        s_len++;
        index++;
    }
    if (s_len > maxlen) maxlen = s_len;
    return maxlen;
}

// ���������������λ��
double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
	float temp[nums1Size+nums2Size];
    // ����ȡ�м�
	if ((nums1Size+nums2Size)%2 == 1) {
		int a=0,b=0;
		for (int i=0; i<(nums1Size+nums2Size); i++) {
			if ( (a<nums1Size)&&(nums1[a]<=nums2[b] || b==nums2Size) ) {
				temp[i] = nums1[a];
				a++;
			}
			else {
				temp[i] = nums2[b];
				b++;
			}
		}
		return temp[(nums1Size+nums2Size)/2];
	}
	// ż����ƽ��
	else {
		int a=0,b=0;
		for (int i=0; i<(nums1Size+nums2Size); i++) {
			if ( (a<nums1Size)&&(nums1[a]<=nums2[b] || b==nums2Size) ) {
				temp[i] = nums1[a];
				a++;
			}
			else {
				temp[i] = nums2[b];
				b++;
			}
		}
		return (temp[(nums1Size+nums2Size)/2]+temp[(nums1Size+nums2Size)/2-1])/2;
	}
}

// ������Ӵ���������ָ��������һ������s�ĳ������Ϊ1000
char* longestPalindrome(char* s) {
    int length=0, i=0, j=0, max_length=0, temp_length=0, start=0;

	// ��ȡ�ַ�������
	while(1) {
		if (s[i++]!='\0') length++;
		else break;
	}

	char result[1001];
	char jyz[4];

	// �������ַ���
	char temps[1000*3];
	for (i=0; i<length; i++) {
		temps[i+length] = s[length-i-1];
	}
	for (i=0; i<length; i++) {
		temps[i] = '\0';
	}
	for (i=length*2; i<length*3; i++) {
		temps[i] = '\0';
	}

	// ���ҷ��ַ�����ԭʼ�ַ����������ͬ�Ӵ�
	for (i=length*2+1; i>0; i--) {
		// ����������
		char *window = &temps[i-1];

		temp_length = 0;
		for (j=0; j<length; j++) {
			if (s[j]==window[j]) {
				if (temp_length==0)	start = j;
				temp_length++;
			}
		}
		if (temp_length>max_length)	{
			max_length = temp_length;
			for (j=0; j<max_length; j++) {
				result[j] = s[start+j];
			}
			for (j=temp_length; j<1001; j++) {
                result[temp_length] = '\0';
            }
		}
	}

	// ��վ�����⣬������ֱ�������˵�� NULL  ������Ҳ�ǡ�
	for (i=0; i<4; i++) {
		jyz[i] = 'a';
	}

	return jyz;
}

// ������Ӵ���������ָ��������һ������s�ĳ������Ϊ1000
// Manacher�㷨ר�λ�������ĸ��ֲ���
char* longestPalindrome(char* s) {

}
// �������㷨
int Manacher() {
    int len = Init();  //ȡ�����ַ������Ȳ������s_new��ת��
    int maxLen = -1;   //����ĳ���
    int id;
    int mx = 0;

    for (int i = 1; i < len; i++) {
        if (i<mx)	p[i] = min(p[2*id-i], mx-i);//��������������ͼ����, mx��2*id-i�ĺ���
        else		p[i] = 1;

        while (s_new[i-p[i]] == s_new[i+p[i]])	//����߽��жϣ���Ϊ����'$',����'\0'
            p[i]++;

		// ����ÿ��һ��i����Ҫ��mx�Ƚϣ�����ϣ��mx�����ܵ�Զ���������ܸ��л���ִ��if(i < mx)�����룬�Ӷ����Ч��
        if (mx < i + p[i]) {
            id = i;
            mx = i + p[i];
        }
        maxLen = max(maxLen, p[i] - 1);
    }
    return maxLen;
}

/**
 *
 * author ���㣨Limer��
 * date   2017-02-25
 * mode   C++
 */
#include<iostream>
#include<string.h>
#include<algorithm>
using namespace std;

char s[1000];
char s_new[2000];
int p[2000];

int Init()
{
    int len = strlen(s);
    s_new[0] = '$';
    s_new[1] = '#';
    int j = 2;

    for (int i = 0; i < len; i++)
    {
        s_new[j++] = s[i];
        s_new[j++] = '#';
    }

    s_new[j] = '\0';  //������Ŷ

    return j;  //����s_new�ĳ���  s
}

int Manacher()
{
    int len = Init();  //ȡ�����ַ������Ȳ������s_new��ת��
    int maxLen = -1;   //����ĳ���

    int id;
    int mx = 0;

    for (int i = 1; i < len; i++)
    {
        if (i < mx)
            p[i] = min(p[2 * id - i], mx - i);  //��������������ͼ����, mx��2*id-i�ĺ���
        else
            p[i] = 1;

        while (s_new[i - p[i]] == s_new[i + p[i]])  //����߽��жϣ���Ϊ����'$',����'\0'
            p[i]++;


        if (mx < i + p[i])  //����ÿ��һ��i����Ҫ��mx�Ƚϣ�����ϣ��mx�����ܵ�Զ����������	���л���ִ��if (i < mx)�����룬�Ӷ����Ч��
        {
            id = i;
            mx = i + p[i];
        }

        maxLen = max(maxLen, p[i] - 1);
    }

    return maxLen;
}

int main()
{
    while (printf("�������ַ�����\n"))
    {
        scanf("%s", s);
        printf("����ĳ���Ϊ %d\n\n", Manacher());
    }

    return 0;
}


#endif
