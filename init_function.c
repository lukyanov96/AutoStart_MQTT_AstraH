#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_can.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "misc.h"
#include "can.h"



void RCC_Config(void)
{
	// Для настройки CAN в максимальном режиме работы на скорости до 1Mb нам необходимо
	// Настроить частоту перефирии APB1 на 16 MHz

	RCC_ClocksTypeDef RCC_Clocks;
	ErrorStatus HSEStartUpStatus;


	// Сбросим настройки тактирования системы
	RCC_DeInit();												// RCC system reset

	// Включим внешний кварц, как источник сигнала
	RCC_HSEConfig(RCC_HSE_ON);									// Enable HSE

	HSEStartUpStatus = RCC_WaitForHSEStartUp();					// Подождем включения HSE

	if (HSEStartUpStatus == SUCCESS)							// Если включился кварц
	{
		// Настроим тактирование так, как нам требуется
		RCC_HCLKConfig(RCC_SYSCLK_Div1);						// HCLK = SYSCLK     (72MHz)
		RCC_PCLK1Config(RCC_HCLK_Div2);							// PCLK1 = HCLK /2  (36MHz)
		RCC_PCLK2Config(RCC_HCLK_Div1);							// PCLK2 = HCLK	     (72MHz)
		//RCC_ADCCLKConfig(RCC_PCLK2_Div2);						// ADC CLK

		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); 	// PLLCLK = 8MHz * 9 = 72 MHz
		RCC_PLLCmd(ENABLE);										// Включаем PLL

		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}	// Ждем включения PLL

		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); 				// Выбираем PLL как источник
																// системного тактирования

		while (RCC_GetSYSCLKSource() != 0x08) {}				// Ждем, пока не установится PLL,
																// как источник системного тактирования
	}

	// Предназначен для отладки, проверяем, как настроены частоты устройства
	RCC_GetClocksFreq (&RCC_Clocks);
		GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE); 		
  
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
		
}
void Prepar_to_sleep(void)
{
	  RCC_APB2PeriphClockCmd(CAN1_Periph | RCC_APB1Periph_TIM4 |RCC_APB2Periph_ADC1, DISABLE);
    
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
 
    /* Tell system that you will use PA10 for EXTI_Line10 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);
 
    EXTI_InitTypeDef EXTI_InitStruct;
    /* PD0 is connected to EXTI_Line10 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line10;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Event;
    /* Triggers on falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
		
		
		//CAN EXTI line
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
 
    /* Tell system that you will use PB8 for EXTI_Line8 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);

    /* PD0 is connected to EXTI_Line8 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line8;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Event;
    /* Triggers on falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
		/*******/
	
}
void ADC_init(void)
     {
	 RCC_ADCCLKConfig (RCC_PCLK2_Div2);
	 // enable ADC system clock
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 // input of ADC (it doesn't seem to be needed, as default GPIO state is floating input)
	 GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	 GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_7 ;
	 GPIO_Init(GPIOA, &GPIO_InitStructure);
	 //NVIC
	 /* NVIC Configuration */
	 NVIC_InitTypeDef NVIC_InitStructure;
	 /* Enable the USARTx Interrupt */
	 NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);
	     
	ADC_InitTypeDef ADC_InitStructure;
    // define ADC config
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // we work in continuous sampling mode
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init (ADC1, &ADC_InitStructure);    //set config of ADC1
    /* ADC1 regular channel configuration */
     // define regular conversion config

    /* Enable AWD interrupt */
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
    //  ADC calibration (optional, but recommended at power on)
    ADC_ResetCalibration(ADC1); // Reset previous calibration
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1); // Start new calibration (ADC must be off at that time)
    while(ADC_GetCalibrationStatus(ADC1));

 }
void TIM4_init(void) 
	{
	
    TIM_TimeBaseInitTypeDef TIMER_InitStructure;
		
    NVIC_InitTypeDef NVIC_InitStructure;
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
 
    TIM_TimeBaseStructInit(&TIMER_InitStructure);
    TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIMER_InitStructure.TIM_Prescaler = 7200;
    TIMER_InitStructure.TIM_Period = 10000; //1 SEC
    TIM_TimeBaseInit(TIM4, &TIMER_InitStructure);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
 
    /* NVIC Configuration */
    /* Enable the TIM4_IRQn Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
		
		TIM_Cmd(TIM4, ENABLE);
}
void init_CAN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* CAN GPIOs configuration */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 		// включаем тактирование AFIO
	RCC_APB2PeriphClockCmd(CAN1_Periph, ENABLE); 				    // включаем тактирование порта
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);		// включаем тактирование CAN-шины
	// Настраиваем CAN RX pin
	GPIO_InitStructure.GPIO_Pin   = CAN1_RX_SOURCE;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN1_GPIO_PORT, &GPIO_InitStructure);
	// Настраиваем CAN TX pin
	GPIO_InitStructure.GPIO_Pin   = CAN1_TX_SOURCE;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN1_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);			// Переносим Can1 на PB8, PB9
	// Инициализация шины
	CAN_InitTypeDef CAN_InitStructure;
	CAN_DeInit( CAN1);
	CAN_StructInit(&CAN_InitStructure);
	// CAN cell init
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = ENABLE;
	CAN_InitStructure.CAN_NART = ENABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;			// Обычный режим работы устройства
  //CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;	// Для тестирования без подключенных устройств шины
	CAN_InitStructure.CAN_SJW = CAN_SJW_4tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	CAN_InitStructure.CAN_Prescaler = CAN1_SPEED_PRESCALE	;	// Выбираем нужную скорость
	CAN_Init(CAN1, &CAN_InitStructure);

	// CAN filter init
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0370<<5; //HandBrake
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0115<<5;  //Brake
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0160<<5; //RemoteControl
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0305<<5;  //LockDoor
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	 CAN_FilterInitStructure.CAN_FilterNumber = 1;                        
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0145<<5; //Engine Temperature
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0445<<5;  //OutDoor Temperature
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0108<<5; //RPM and Speed
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0230<<5;            
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	// CAN FIFO0 message pending interrupt enable
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

	// NVIC Configuration
	// Enable CAN1 RX0 interrupt IRQ channel
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}
void usart_init(void)
{
    /* Enable USART1 and GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* NVIC Configuration */
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable the USARTx Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure the GPIOs */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure the USART1 */
    USART_InitTypeDef USART_InitStructure;

    /* USART1 configuration ------------------------------------------------------*/
    /* USART1 configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
        - USART Clock disabled
        - USART CPOL: Clock is active low
        - USART CPHA: Data is captured on the middle
        - USART LastBit: The clock pulse of the last data bit is not output to
            the SCLK pin
     */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);

    /* Enable USART1 */
    USART_Cmd(USART1, ENABLE);

    /* Enable the USART1 Receive interrupt: this interrupt is generated when the
        USART1 receive data register is not empty */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void usart3_init(void)
{
    /* Enable USART1 and GPIOA clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);
    /* NVIC Configuration */
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable the USARTx Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure the GPIOs */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure the USART1 */
    USART_InitTypeDef USART_InitStructure;

    /* USART1 configuration ------------------------------------------------------*/
    /* USART1 configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
        - USART Clock disabled
        - USART CPOL: Clock is active low
        - USART CPHA: Data is captured on the middle
        - USART LastBit: The clock pulse of the last data bit is not output to
            the SCLK pin
     */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStructure);

    /* Enable USART1 */
    USART_Cmd(USART3, ENABLE);

    /* Enable the USART1 Receive interrupt: this interrupt is generated when the
        USART1 receive data register is not empty */
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}

void USARTSend3(char *pucBuffer)
{
    while (*pucBuffer)
    {
        USART_SendData(USART3, *pucBuffer++);
        while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
        {
        }
    }
}

void CAN_Send(uint32_t ID, uint8_t DLC, uint8_t data0,uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4,uint8_t data5,uint8_t data6,uint8_t data7)
{
	CanTxMsg TxMessage;
	TxMessage.StdId = ID;			// Команда шины

	TxMessage.ExtId = 0x00;							// Расширенную команду указывать нет смысла

	TxMessage.IDE = CAN_Id_Standard;				// Формат кадра
	TxMessage.RTR = CAN_RTR_DATA;					// Тип сообщения
	TxMessage.DLC = DLC;								// Длина блока данных 3 - передадим три байта

	TxMessage.Data[0] = data0;						// Байт данных №1
	TxMessage.Data[1] = data1;						// Байт данных №2
	TxMessage.Data[2] = data2;						// Байт данных №3
	TxMessage.Data[3] = data3;						// Байт данных №4
	TxMessage.Data[4] = data4;						// Байт данных №5
	TxMessage.Data[5] = data5;						// Байт данных №6
	TxMessage.Data[6] = data6;						// Байт данных №7
	TxMessage.Data[7] = data7;						// Байт данных №8

	CAN_Transmit(CAN1, &TxMessage);
}
void USARTSend(char *pucBuffer)
{
    while (*pucBuffer)
    {
        USART_SendData(USART1, *pucBuffer++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {
        }
    }
}

void USARTSend_char(char pucBuffer)
{
        USART_SendData(USART1, pucBuffer);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {
        }
}
char halfbyte_to_hexascii(uint8_t _halfbyte)
{
 _halfbyte &= 0x0F ;
 if(_halfbyte >= 10) return('A' + _halfbyte - 10) ;
	else               return('0' + _halfbyte) ;
}
uint8_t hexascii_to_halfbyte(uint8_t _ascii)
{
 if((_ascii >= '0') && (_ascii <= '9')) return(_ascii - '0') ;
 if((_ascii >= 'a') && (_ascii <= 'f')) return(_ascii - 'a'+10) ;
 if((_ascii >= 'A') && (_ascii <= 'F')) return(_ascii - 'A'+10) ;
 return(0xFF)	;
}
char * utoa_builtin_div(uint32_t value, char *buffer)
{
   buffer += 11; 
// 11 байт достаточно для десятичного представления 32-х байтного числа
// и  завершающего нуля
   *--buffer = 0;
   do
   {
      *--buffer = value % 10 + '0';
      value /= 10;
   }
   while (value != 0);
   return buffer;
}
void Delay_ms(uint32_t ms)
{
        volatile uint32_t nCount;
        RCC_ClocksTypeDef RCC_Clocks;
        RCC_GetClocksFreq (&RCC_Clocks);

        nCount=(RCC_Clocks.HCLK_Frequency/12012)*ms;
        for (; nCount!=0; nCount--);
}
