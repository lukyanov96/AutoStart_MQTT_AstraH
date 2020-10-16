#ifndef __CAN
#define __CAN
#include "stm32f10x.h"                  // Device header


/******************************************************************************
 * Определение настройки CAN
 ******************************************************************************/
  #define CAN1_ReMap // Закоментировать, если нет ремапинга портов

  #define CAN1_GPIO_PORT			GPIOB
	#define CAN1_RX_SOURCE			GPIO_Pin_8				// RX-порт
	#define CAN1_TX_SOURCE			GPIO_Pin_9				// TX-порт
	#define CAN1_Periph				RCC_APB2Periph_GPIOB	// Порт перифирии


// Выбор скорости шины
 #define CAN1_SPEED_PRESCALE			60						// 33 Kb
   

#define ACC_on  GPIO_SetBits(GPIOA,GPIO_Pin_1)
#define ACC_off GPIO_ResetBits(GPIOA,GPIO_Pin_1)
#define IMO_on  GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define IMO_off GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define IGN_on  GPIO_SetBits(GPIOA,GPIO_Pin_2)
#define IGN_off GPIO_ResetBits(GPIOA,GPIO_Pin_2)
#define STARTER_on  GPIO_SetBits(GPIOA,GPIO_Pin_3)
#define STARTER_off GPIO_ResetBits(GPIOA,GPIO_Pin_3)
#define MCP_suspend_on  GPIO_SetBits(GPIOA,GPIO_Pin_6)
#define MCP_suspend_off GPIO_ResetBits(GPIOA,GPIO_Pin_6)
#define SIM800_reset_on  GPIO_SetBits(GPIOA,GPIO_Pin_8)
#define SIM800_reset_off GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define SIM800_suspend_on  GPIO_SetBits(GPIOB,GPIO_Pin_14)
#define SIM800_suspend_off GPIO_ResetBits(GPIOB,GPIO_Pin_14)
#define LED_off  GPIO_SetBits(GPIOC,GPIO_Pin_13)
#define LED_on   GPIO_ResetBits(GPIOC,GPIO_Pin_13)



/******************************************************************************
 * Определение команд протокола
 ******************************************************************************/
#define CAN_CMD_Test_Send			0x0001		// Команда отправки тестового сообщения
#define CAN_CMD_Test_Ok				0x0002		// Команда подтверждения тестового сообщения


typedef struct
{
  uint32_t StdId;  /*!< Specifies the standard identifier.
                        This parameter can be a value between 0 to 0x7FF. */

  uint32_t ExtId;  /*!< Specifies the extended identifier.
                        This parameter can be a value between 0 to 0x1FFFFFFF. */

  uint8_t IDE;     /*!< Specifies the type of identifier for the message that
                        will be transmitted. This parameter can be a value
                        of @ref CAN_identifier_type */

  uint8_t RTR;     /*!< Specifies the type of frame for the message that will
                        be transmitted. This parameter can be a value of
                        @ref CAN_remote_transmission_request */

  uint8_t DLC;     /*!< Specifies the length of the frame that will be
                        transmitted. This parameter can be a value between
                        0 to 8 */

  uint8_t Data[8]; /*!< Contains the data to be transmitted. It ranges from 0
                        to 0xFF. */
} CanTx;



/******************************************************************************
 * Определение прототипов функций
 ******************************************************************************/
void init_CAN(void);
void USB_LP_CAN1_RX0_IRQHandler(void);
void RCC_Config(void);
void TIM4_init(void);
void usart_init(void);

void usart3_init(void);
void USARTSend3(char *pucBuffer);

void USARTSend(char *pucBuffer);

void USARTSend_char(char pucBuffer);
char * utoa_builtin_div(uint32_t value, char *buffer);
void CAN_Send(uint32_t ID, uint8_t DLC, uint8_t data0,uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4,uint8_t data5,uint8_t data6,uint8_t data7);
void Delay_ms(uint32_t ms);
void ADC_init(void);
void Prepar_to_sleep(void);
char halfbyte_to_hexascii(uint8_t _halfbyte);
uint8_t hexascii_to_halfbyte(uint8_t _ascii);
#endif //__CAN
