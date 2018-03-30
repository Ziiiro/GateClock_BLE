#ifndef __DEBUG_USART_H
#define	__DEBUG_USART_H
#include "stm32f10x.h"
//#include "stm32f10x_usart.h"
#include "stm32f10x_conf.h"
#include <stdio.h>
#include "my_board.h"

// ���ڵȴ�ʱ��
#define UART_WAIT_TIME                     0xfffff

/********************************************** ���Ŷ��� *************************************************/
#define UART_1                             USART1
#define UART_1_CLK                         RCC_APB2Periph_USART1
#define UART_1_BAUDRATE                    115200

#define UART_1_RX_GPIO_PORT                GPIOA
#define UART_1_RX_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define UART_1_RX_PIN                      GPIO_Pin_10
#define UART_1_RX_AF                       GPIO_AF_USART1
#define UART_1_RX_SOURCE                   GPIO_PinSource10

#define UART_1_TX_GPIO_PORT                GPIOA
#define UART_1_TX_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define UART_1_TX_PIN                      GPIO_Pin_9
#define UART_1_TX_AF                       GPIO_AF_USART1
#define UART_1_TX_SOURCE                   GPIO_PinSource9






/********************************************** �û����� *************************************************/
void Usart_SendUserId(USART_TypeDef* pUSARTx, u16 user_id);
void Usart_SendIC_Success(USART_TypeDef* pUSARTx, u16 user_id);
void Usart_SendIC_Error(USART_TypeDef* pUSARTx);
u16 Usart_RecvOrder(USART_TypeDef* pUSARTx);











/********************************************** �ײ㺯�� *************************************************/
void debug_usart_init(void);
uint16_t Usart_SendByte( USART_TypeDef* pUSARTx, u8 ch );
uint16_t Usart_RecvByte( USART_TypeDef* pUSARTx);
uint16_t uart_error(void);
void QS808_INT_EXT_IRQHandler(void);

void pUsart_SendByte( USART_TypeDef* pUSARTx, u8* ch );
void pUsart_SentMessage( USART_TypeDef* pUSARTx, u16* message);

#endif
