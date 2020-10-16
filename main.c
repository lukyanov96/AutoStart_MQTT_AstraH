#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_pwr.h"              // Keil::Device:StdPeriph Drivers:PWR
#include "stm32f10x_exti.h"             // Keil::Device:StdPeriph Drivers:EXTI
#include "stm32f10x_usart.h"
#include "stm32f10x_can.h"
#include "stm32f10x_adc.h"              // Keil::Device:StdPeriph Drivers:ADC
#include "stm32f10x_tim.h" 

#include "misc.h"
#include "can.h" 
#include "mqtt.h"
#include <string.h>
#include <stdbool.h>


#define StDelay 3500 //Время кручения стартером в мс
//Переменные для обработки USART

#define RX_BUF_SIZE 80
volatile char RX_FLAG_END_LINE = 0;
volatile char RX_FLAG_MQTT = 0;
volatile char RX_FLAG_MQTT2 = 0;
volatile char MQTT_commandi;
char MQTT_command[20]={'\0'};
volatile char MQTT_command_END;
volatile char RXi;
volatile char RXc;
char RX_BUF[RX_BUF_SIZE] = {'\0'};

volatile char buffer[80] = {'\0'};       

char buf[12];
//-----------------------//

//#define Test





/*  ----------------------------------------- Номера телефонов и команды SMS !!!---------------------------------------------------------   */
char phone[]=  "+79xxxxxxxxx";        // Телефон для команд DTMF и SMS
char phone2[]= "+79xxxxxxxxx";        // Телефон только для SMS

char command1[]="xxxxxx";
char command2[]="xxxxxx";
char APN[] = "internet.mts.ru";             // Точка доступа для интернета вашего оператора
                                   /*--------------------Настройки для MQTT--------------------*/
uint8_t broker = false; 
char MQTT_user[10] = "xxxxxxx";      // api.cloudmqtt.com > Details > User  
char MQTT_pass[15] = "xxxxxxx";  // api.cloudmqtt.com > Details > Password
char MQTT_type[15] = "MQIsdp";        // тип протокола, не трогать !
char MQTT_CID[15] = "xxx";        //  уникальное название устройства MQTT
char MQTT_SERVER[] = "farmer.cloudmqtt.com";   // api.cloudmqtt.com > Details > Server  сервер MQTT 
char PORT[] = "12478"; 

/*  --------------------------------------------------------------------------------------------------------   */
uint8_t Vstart = 132;                        // Порог напряжения для распознавания работы двигателя
char  number_message;                       // номер сообщения для обработки
int8_t OutSideTemp=-20;                     
int8_t EngineTemp=0;                          
uint8_t Vbat;
uint8_t Vmin=117;                              // Минимальное напряжение при котором можно заводить
float xADC = 23.4;                            // Коэффициент для перевода в Вольты 39/11kOm
//Служебные переменные
uint8_t SMS_read=0;
unsigned long Time1, Time2 = 0;
int Timer, inDS, count, error_CF, error_C;
uint8_t interval = 0;                        

uint8_t heating = false;                       // Флаг запуска прогрева
uint8_t ring = false;                          // Флаг звонка

uint8_t sms_report=true;                       // Разрешить отправлять СМС
uint8_t status;



uint8_t Counter_CAN=0;
uint8_t test=0;
uint8_t sleep_timer=0;
uint8_t allow_sleep=true;

uint8_t hand_brake=false;
uint8_t brake=false;

uint8_t light;
uint8_t open;

uint8_t connect_flag=0;

uint8_t virtual_neutral=0;

uint16_t speed=0;
uint16_t rpm=0;
uint8_t engine_state=0;

char key[10];
uint8_t keyi;
void clear_RXBuffer(void) 
{
    for (RXi=0; RXi<RX_BUF_SIZE; RXi++)
        RX_BUF[RXi] = '\0';
    RXi = 0;
}


void Engine_start(void)
{

	    ADC_init();
	    //Запускаем ADC, для проверки напряжение 
			Vbat=0;
			ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_41Cycles5);
			ADC_Cmd (ADC1,ENABLE);
      ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			Delay_ms(200);
			if (Vbat>=Vmin)
			{
			Counter_CAN=0;
			IMO_on;
			Delay_ms(100);
			ACC_on; 
			Delay_ms(300);Delay_ms(300);Delay_ms(300);Delay_ms(300);Delay_ms(300);Delay_ms(300);Delay_ms(300);Delay_ms(300);Delay_ms(300);Delay_ms(300); //Wait Wake Up CAN
			sleep_timer=0;
			            if (hand_brake == 1 && virtual_neutral==6)
			            {
										    engine_state=0;
									      __disable_irq();
			                  IGN_on;	
			                  Delay_ms(2000);
			                  STARTER_on;
										    Delay_ms(50);
			                  ACC_off;		
			                  Delay_ms(StDelay);
			                  STARTER_off;
										    Delay_ms(100);
			                  ACC_on;
			                  Delay_ms(7000);
										    __enable_irq();
										    Delay_ms(200);Delay_ms(200);Delay_ms(200);Delay_ms(200);Delay_ms(200);Delay_ms(200);Delay_ms(200);Delay_ms(200);
			                 
			                   if (engine_state==0x13)
			                       {
			                        allow_sleep=DISABLE;
			                        heating=true;
			                        status=1;
			                       }
			                   else if (engine_state <= 0x10)
			                       {
		                          ACC_off;IMO_off;IGN_off;
			                        heating=false;
			                        allow_sleep=ENABLE;
			                        sleep_timer=0;
			                        status=2;
			                       }
		                    
			            }
			            else
			            {
			            ACC_off;IMO_off;IGN_off;
			            heating=DISABLE;
			            allow_sleep=ENABLE;
			            sleep_timer=0;
			            status=3;
			            }
			}
			else if (Vbat<Vmin)
			{
			ACC_off;IMO_off;IGN_off;
			heating=DISABLE;
			allow_sleep=ENABLE;
			sleep_timer=0;
			status=4;
			}
		
}
void Engine_stop(void)
{
	IGN_off; Delay_ms(100);
	ACC_off; Delay_ms(100);
	IMO_off; Delay_ms(100);
	heating=false;
	status=5;
	allow_sleep=true;	
}
void MQTT_CONNECT (void) 
{
	USARTSend("\r\r"); Delay_ms(150);
  USARTSend("\rAT+CIPSEND\r"); Delay_ms (300);
     
  USARTSend_char(0x10);                                                             
  USARTSend_char(strlen(MQTT_type)+strlen(MQTT_CID)+strlen(MQTT_user)+strlen(MQTT_pass)+12);
  USARTSend_char(0); USARTSend_char(strlen(MQTT_type));USARTSend(MQTT_type);   
  USARTSend_char(0x03); USARTSend_char(0xC2);USARTSend_char(0); USARTSend_char(0x3C); 
  USARTSend_char(0); USARTSend_char(strlen(MQTT_CID));  USARTSend(MQTT_CID);  
  USARTSend_char(0); USARTSend_char(strlen(MQTT_user)); USARTSend(MQTT_user); 
  USARTSend_char(0); USARTSend_char(strlen(MQTT_pass)); USARTSend(MQTT_pass); 

  MQTT_PUB ("C5/status", "Connected");                                            
  MQTT_SUB ("C5/comand");                                                          
  MQTT_IntPub ("C5/ds0",      EngineTemp);
  MQTT_IntPub ("C5/ds2",      OutSideTemp);
  MQTT_PUB_float("C5/vbat",     Vbat);
	MQTT_IntPub ("C5/V", virtual_neutral);
	MQTT_IntPub ("C5/C", error_C);
	MQTT_IntPub ("C5/S", status);
  MQTT_IntPub ("C5/CF", error_CF); 
	if (heating==true)
			{
				 MQTT_PUB("C5/engine","start%");
			}
	else if (heating==false)
			{
				MQTT_PUB("C5/engine","stop%");
			}	 
	if (virtual_neutral==6)
			{
				MQTT_PUB("C5/neutral","on");
			}
	else
			{
				MQTT_PUB("C5/neutral","off");
			}
  USARTSend_char(0x1A),  broker = true;   
}                                        
void MQTT_fun(void)
{
	/*-----------------------MQTT--------------------*/


if (strstr(RX_BUF,"0.0.0.0") != 0) 
     {
      USARTSend("\r\r"); 
		  Delay_ms(250);
		  USARTSend("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"); 
		  //Delay_ms(200); 
		  broker=false;
			connect_flag=0;
		 } 

if (strstr(RX_BUF,"APBR=3,1,\"CONTYPE\"")!=0) // Received +SAPBR=3,1,\"CONTYPE\",\"GPRS\"
     { 
		  connect_flag=1;
		 }

if (strstr(RX_BUF,"OK") != 0 && connect_flag==1)  // Received +SAPBR=3,1,\"CONTYPE\",\"GPRS\"____OK
     { 
		 USARTSend("\r\r"); 
		 Delay_ms(300); 
		 USARTSend("AT+SAPBR=3,1, \"APN\",\""); USARTSend(APN);USARTSend("\"\r\n"); 
		 //Delay_ms (200);
		 broker=false; 
		 }        

if (strstr(RX_BUF,APN)!=0) 
     {
		 connect_flag=2; 
		 }


if (strstr(RX_BUF,"OK") != 0 && connect_flag==2)   
    { 
		USARTSend("\r\r"); 
		Delay_ms(300); 
		USARTSend("AT+SAPBR=1,1\r\n"); 
		interval = 58; 
		connect_flag=0;
		broker=false;
		} 

if (strstr(RX_BUF,"+SAPBR: 1,1") != 0 )        
    {     
		USARTSend("\r\r"); 
		Delay_ms(300);  
		USARTSend("AT+CIPSTART=\"TCP\",\""); USARTSend(MQTT_SERVER);USARTSend("\",\"");USARTSend(PORT);USARTSend("\"\r");
	  interval = 45;
		//Delay_ms (1000);
		broker=false;
		}

if (strstr(RX_BUF,"ONNECT FAIL") != 0 ) //Connect fail to mqtt server, restart SIM800      
    {      
		USARTSend("\r\r"); 
		Delay_ms(250);
		error_CF++;
		USARTSend("AT+CFUN=1,1\r\n");  
		Delay_ms (1000); 
		interval = 40 ;
		broker=false;
		}  

if (strstr(RX_BUF,"LOSED") != 0 )       //Connect closed to mqtt server, restart SIM800      
   {      
	 USARTSend("\r\r"); 
	 Delay_ms(250);
	 error_C++ ;
	 USARTSend("AT+CFUN=1,1\r\n");  
	 Delay_ms (1000); 
	 interval = 40 ;
	 broker=false;
	 } 
if (strstr(RX_BUF,"ME ERROR:") != 0 )   //Modem Error, if 5 times, restart SIM800         
	{ 
		error_CF++;
	  if (error_CF==5)
	  {
	   error_CF = 0;
	   USARTSend("\r\r"); Delay_ms(250);
	   USARTSend("AT+CFUN=1,1\r\n");  
	   Delay_ms (1000); 
	   interval = 0 ;
	   broker=false;
	  }
	} 
if (strstr(RX_BUF,"ONNECT OK") != 0)    //Connect ok to server, sign in MQTT
  {
	USARTSend("\r\r"); 
	Delay_ms(250);  
	MQTT_CONNECT();
  interval=30;		
	}

	
if (strstr(RX_BUF,"LREADY")!=0)  //Alredy connect to server, send data in MQTT               
	{ 
		USARTSend("\r\r"); Delay_ms(250); broker=true;
	  USARTSend("\r\nAT+CIPSEND\r\n"),Delay_ms (200); // ???? ?? "???????" "ALREADY CONNECT"
    MQTT_IntPub ("C5/ds0",      EngineTemp);
    MQTT_IntPub ("C5/ds2",      OutSideTemp);
	  MQTT_PUB_float("C5/vbat",     Vbat);
		MQTT_IntPub ("C5/V", virtual_neutral);
		MQTT_IntPub ("C5/S", status);
    MQTT_IntPub ("C5/C", error_C);
    MQTT_IntPub ("C5/CF", error_CF); 
	  if (heating==true)
			{
			MQTT_PUB("C5/engine","start%");
			}
		else if (heating==false)
			{
			MQTT_PUB("C5/engine","stop%");
			}
		if (virtual_neutral==6)
			{
			MQTT_PUB("C5/neutral","on");
			}
		else
			{
			MQTT_PUB("C5/neutral","off");
			}
    USARTSend_char(0x1A);
		interval=0;
  }
if (strstr(RX_BUF,"comandstart")!=0)
	{
		if (heating==false)
		{
    Engine_start();
		}
		interval=58;
             #ifdef Test							
							USARTSend3("start\r");
							#endif
	}
if (strstr(RX_BUF,"comandstop")!=0)
  {
	 if (heating==true)
	 {
     Engine_stop();
	 }
							#ifdef Test
							USARTSend3("stop\r");
							#endif
	}
if (strstr(RX_BUF,"comandUpdate")!=0)
	{
	  interval=20;
		USARTSend("\r\r"); Delay_ms(250); broker=true;
	  USARTSend("\r\nAT+CIPSEND\r\n"),Delay_ms (200); // ???? ?? "???????" "ALREADY CONNECT"
    MQTT_IntPub ("C5/ds0",      EngineTemp);
    MQTT_IntPub ("C5/ds2",      OutSideTemp);
	  MQTT_PUB_float("C5/vbat",     Vbat);
		MQTT_IntPub ("C5/V", virtual_neutral);
		MQTT_IntPub ("C5/S", status);
    MQTT_IntPub ("C5/C", error_C);
    MQTT_IntPub ("C5/CF", error_CF); 
	  if (heating==true)
			{
			MQTT_PUB("C5/engine","start%");
			}
		else if (heating==false)
			{
			MQTT_PUB("C5/engine","stop%");
			}
		if (virtual_neutral==6)
			{
			MQTT_PUB("C5/neutral","on");
			}
		else
			{
			MQTT_PUB("C5/neutral","off");
			}
    USARTSend_char(0x1A);
	}
}		


void enter_to_stop_mode(void)
{ 
	sleep_timer=0;
	TIM_Cmd(TIM4,DISABLE);
	LED_off;
	MCP_suspend_on;
	
	Prepar_to_sleep();
	
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
	
	EXTI_ClearFlag(EXTI_Line10|EXTI_Line8);
	EXTI_DeInit();

	RCC_Config(); 
	LED_on;
	MCP_suspend_off; 

	init_CAN();
	usart_init(); ADC_init(); TIM4_init(); 
		#ifdef Test
	   usart3_init();
	 #endif
	 ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_41Cycles5);
	 ADC_Cmd (ADC1,ENABLE);
   ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	 interval=55;
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	sleep_timer=0;

	  
	CanRxMsg RxMessage;
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET) {                  
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);                  
		RxMessage.DLC =     0x00;
		RxMessage.ExtId =   0x00;
		RxMessage.FMI =     0x00;
		RxMessage.IDE =     0x00;
		RxMessage.RTR =     0x00;
		RxMessage.StdId =   0x00;
		RxMessage.Data [0] = 0x00;
		RxMessage.Data [1] = 0x00;
		RxMessage.Data [2] = 0x00;
		RxMessage.Data [3] = 0x00;
		RxMessage.Data [4] = 0x00;
		RxMessage.Data [5] = 0x00;
		RxMessage.Data [6] = 0x00;
		RxMessage.Data [7] = 0x00;
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
		
    if (RxMessage.StdId==0x108)
		{
			speed =(RxMessage.Data[4]<<1) + (RxMessage.Data[5]>>7);
			rpm = (RxMessage.Data[1]<<6) + (RxMessage.Data[2]>>2);
			engine_state=RxMessage.Data[0];
			
			if (rpm > 500 && speed == 0 && engine_state == 0x13 && virtual_neutral==0 && heating==false)
			{
				virtual_neutral=1; //Engine running and speed 0km/h, set flag for virtual neutral
			}
			if (engine_state <= 0x10 && heating==false)
			{
				if (virtual_neutral==5 && hand_brake==1 && brake==0)
				{
				 virtual_neutral=6;
				}
				if (virtual_neutral<5)
				{
					virtual_neutral=0;
					Engine_stop();
				}
			}
			if (rpm > 500 && speed > 0 && engine_state == 0x23 && virtual_neutral==6 && heating==false)
			{
				virtual_neutral=0; //Engine start, reset flag for virtual neutral
			}
		}
		/*---------------------Virtual neutral 1 step*-------------------*/
		if (RxMessage.StdId==0x145)
		{
			if (EngineTemp!=RxMessage.Data[3]-40)
			{
			EngineTemp=RxMessage.Data[3]-40;
			}
		}
		if (RxMessage.StdId==0x370)
		{
			if (RxMessage.Data[1]==1 || RxMessage.Data[1]==3) //Ruchnik podnyat
		   {
			  hand_brake=1;
			  Counter_CAN++;
		   }
		  if (RxMessage.Data[1]==0 || RxMessage.Data[1]==2)
			 {
				hand_brake=0;
				Counter_CAN++;
			 }
	  }
		if (heating==false && virtual_neutral==1 && hand_brake==1)
			   {
				  virtual_neutral=2;      //Hand brake released, wait brake and clutch not pressed
			   }
			if (heating==false && virtual_neutral < 5 && virtual_neutral>=2 && hand_brake==0)
			   {
				  virtual_neutral=0;    // Hand brake not released, return to 1 step
          Engine_stop();
			   } 
		/*---------------------Virtual neutral 3 step*-------------------*/
		if (RxMessage.StdId==0x115)
		{
		   if (RxMessage.Data[0]==0x04 || RxMessage.Data[0]==0x0C || RxMessage.Data[0]==0x02)//Tormoz najat libo vyjzato sceplenie
		   {
			  brake=1;
				if (heating==false && virtual_neutral < 5 && virtual_neutral>=3 && brake==1)
			   {
				  virtual_neutral=0;    // Brake or clutch pressed, return to 2 step
					Engine_stop();
			   } 			 
		   }
		   if (RxMessage.Data[0]==0)
		   {
			  brake=0;
		   }
		   if (heating==false && virtual_neutral==2 && hand_brake==1 && brake==0)
		   {
		  	  virtual_neutral=3;	// Vkluchaem Rele Jdem otkryiya dveri
	        IMO_on; Delay_ms(100);
	        IGN_on; Delay_ms(100);
      	  ACC_on; Delay_ms(100);
	
		      virtual_neutral=4;				 
		   }
		}
		/*---------------------Virtual neutral 4 step*-------------------*/
		
		if (RxMessage.StdId==0x230)
		{
			if (RxMessage.Data[2]==0x40 || RxMessage.Data[2]==0x50)
			{
		    if (heating==false && virtual_neutral==4 && hand_brake==1)
		      {
			     virtual_neutral=5;
			     Engine_stop();
		      }
			}
			if (RxMessage.Data[2]==0)
			{
				if (heating==false && virtual_neutral==5)
				{
					 virtual_neutral=0;
			     Engine_stop();
				}
			}
		}
		/*---------------------------------------------------------------*/
		if (RxMessage.StdId==0x445 && OutSideTemp!=(RxMessage.Data[1]/2)-40)
		{
			OutSideTemp=(RxMessage.Data[1]/2)-40;
		}
		
		if (RxMessage.StdId==0x160 && RxMessage.Data[0]==1 && RxMessage.Data[1]==0x10 && RxMessage.Data[2]==0x75 && RxMessage.Data[3]==0xE8 && heating==true )
		{
			open=true; // Open Door 1 click RemoteControl
		}		
		if (RxMessage.StdId==0x160 && RxMessage.Data[0]==1 && RxMessage.Data[1]==0x20 && RxMessage.Data[2]==0x75 && RxMessage.Data[3]==0xE8 && heating==true )
		{
      Engine_stop(); //Stop Heatind 2 click RemoteControl
		}


	}	
	
}

    
/*--------------Функция отправки SMS ------------------*/
void Send_SMS()
{
		    USARTSend("\r\r"); Delay_ms(300);
    if (status == 1 && heating==true && sms_report == true) { status = 0;          
        USARTSend("AT+CMGS=\"");if (SMS_read==1){ USARTSend(phone);} if(SMS_read==2){ USARTSend(phone2);} USARTSend("\"\r");  Delay_ms(300);
        USARTSend("Hi,engine start.");
        USARTSend("\n Engine=");   if (EngineTemp<0) {USARTSend("-"); USARTSend(utoa_builtin_div(EngineTemp*(-1), buf));}  else {USARTSend(utoa_builtin_div(EngineTemp, buf));}  USARTSend("C");
				USARTSend("\n OutSide=");  if (OutSideTemp<0){USARTSend("-"); USARTSend(utoa_builtin_div(OutSideTemp*(-1), buf));} else {USARTSend(utoa_builtin_div(OutSideTemp, buf));} USARTSend("C");
				USARTSend("\n Voltage=");  USARTSend(utoa_builtin_div(Vbat, buf)); USARTSend("V.");
        USARTSend_char(26);  }
		if (status == 2 && heating==false && sms_report == true) { status = 0;          
        USARTSend("AT+CMGS=\"");if (SMS_read==1){ USARTSend(phone);} if(SMS_read==2){ USARTSend(phone2);};USARTSend("\"\r");  Delay_ms(300);
        USARTSend("\nHi,engine no start.");
        USARTSend_char(26);  }
		if (status == 3 && heating==false && sms_report == true) { status = 0;          
        USARTSend("AT+CMGS=\"");if (SMS_read==1){ USARTSend(phone);} if(SMS_read==2){ USARTSend(phone2);};USARTSend("\"\r");  Delay_ms(300);
			  if (hand_brake==0)
				{
        USARTSend("\n Hi, hand brake error.");
				}
				if (virtual_neutral!=6)
				{
        USARTSend("\n Hi, virtual neutral error.");
				}
        USARTSend_char(26);  }
		if (status == 4 && heating==false && sms_report == true) { status = 0;          
        USARTSend("AT+CMGS=\""); if (SMS_read==1){ USARTSend(phone);} if(SMS_read==2){ USARTSend(phone2);}USARTSend("\"\r");  Delay_ms(300);
        USARTSend("\nHi,Low voltage for running.");
        USARTSend_char(26);  }
	  if (status == 5 && heating==false && sms_report == true) { status = 0;          
        USARTSend("AT+CMGS=\"");if (SMS_read==1){ USARTSend(phone);} if(SMS_read==2){ USARTSend(phone2);}USARTSend("\"\r");  Delay_ms(300);
        USARTSend("\nStop_heating.");
        USARTSend_char(26);  }
		    USARTSend("\r\r");
}
	
/*------------------------------------------------------*/	

int main(void)
{
	 RCC_Config();
   SIM800_suspend_off;	
   TIM4_init();
	 usart_init(); 
	 init_CAN();
	 	#ifdef Test
	   usart3_init();
	 #endif
	while(1)
		{			
      if (interval==60) // Раз в минуту проверяем подключение к интернету
			{
				#ifdef Test
				USARTSend3("AT+SAPBR=2,1\r\r");
				#endif
				USARTSend("\r\r"); Delay_ms(350);
				USARTSend("AT+SAPBR=2,1\r\r");
				interval=0;
			}
		
      if (heating==true && interval==15) // Если на прогреве раз в 15 секунд
		  {
				USARTSend("\r\r"); Delay_ms(350);
				USARTSend("AT+SAPBR=2,1\r\r");
				interval=0;
		  }
		  if (sleep_timer==20) // Уход в сон, через 20 секунд после последней активности
		  {
				enter_to_stop_mode();
		  }

			if (heating==true && brake==true) //Остановка прогрева по нажатию тормоза
		{
			Engine_stop();
		}
		if (light==true) //Передние повортники во время прогрева
		{
			Delay_ms(50);
			CAN_Send(0x250,8,0x06,0xAE,0x02,0x00,0x00,0x03,0x03,0x00);
			Delay_ms(50);
			light=false;
		}
		if (heating==true && open==true) //ЦЗ когда идет прогрев
		{
			Delay_ms(500);
			CAN_Send(0x305,7,0x00,0x00,0x00,0x00,0x10,0x80,0x01,0x00);
			open=false;
		}

		
		/*DTFM обработка*/
		if (strstr(key, "1234") != 0) //Командка 1234 для запуска двигателя
		{
			key[0]='\0';key[1]='\0';key[2]='\0';key[3]='\0';key[4]='\0';key[5]='\0';
			USARTSend("\r\r"); Delay_ms(500);
			USARTSend("\rAT+CREC=4,\"C:\\User\\2.amr\",0,90\r"); //Голосовое оповещение "Принято завожу"
			Engine_start();
			if (heating==true && status==1)
			{USARTSend("\r\r"); Delay_ms(350);USARTSend("\rAT+CREC=4,\"C:\\User\\3.amr\",0,90\r");} //Голосовое оповещение "Двигатель запущен" 
			if (heating==false && status==2)
			{USARTSend("\r\r"); Delay_ms(350);USARTSend("\rAT+CREC=4,\"C:\\User\\4.amr\",0,90\r");} //Голосовое оповещение "Двигатель не запущен" 
			if (heating==false && status==3)
				{USARTSend("\r\r"); Delay_ms(350);USARTSend("\rAT+CREC=4,\"C:\\User\\5.amr\",0,90\r");} //Голосовое оповещение "Ручник опущен"  
			if (heating==false && status==4)
				{USARTSend("\r\r"); Delay_ms(350);USARTSend("\rAT+CREC=4,\"C:\\User\\6.amr\",0,90\r");} //Голосовое оповещение "Низкое напряжение" 
			Delay_ms(1500);
			USARTSend("\r\r"); Delay_ms(150);
			USARTSend("\rATH\r");
			ring=DISABLE;
		}
		
		if (strstr(key, "5678") != 0) //Командка 5678 для остановки двигателя
		{
			key[0]='\0';key[1]='\0';key[2]='\0';key[3]='\0';key[4]='\0';key[5]='\0';
			Engine_stop();
			if (heating==false && status==5)
			{USARTSend("\r\r"); Delay_ms(150);USARTSend("\rAT+CREC=4,\"C:\\User\\7.amr\",0,90\r");} //"Глушу двигатель" 
			Delay_ms(1000);
			USARTSend("\r\r"); Delay_ms(150);USARTSend("\rATH\r");
			ring=DISABLE;
		}
		/*--------------*/
		/*Обработка данных от модема*/	
	if (RX_FLAG_END_LINE == ENABLE) 
	{
	 sleep_timer=0; 
   RX_FLAG_END_LINE = DISABLE; // Reset END_LINE Flag	
		
		MQTT_fun(); // Функция для подключения к MQTT и обработке команд
		
		
		/*------------------Обработка звонка--------------*/
  if (strstr(RX_BUF, "LIP:") != 0){ //+CLIP  
	if (strstr(RX_BUF, phone) !=0){	
	  Delay_ms(200);
	  USARTSend("\r\r"); Delay_ms(150);USARTSend("\rATA\r"); //Поднимаем трубку
		Delay_ms(500);
		USARTSend("\r\r"); Delay_ms(150);USARTSend("\rAT+CREC=4,\"C:\\User\\1.amr\",0,90\r");  //"Жду команду"
	  ring=ENABLE;
	  keyi=0;}
	else
	{
		USARTSend("\r\r"); Delay_ms(150);USARTSend("\rATH\r"); // Номер не мой кладу трубку
	ring=DISABLE;
	}
	}
	/*DTFM формируем строку*/
	if (strstr(RX_BUF, "+DTMF:") != 0 && ring==true)
	{
	 key[keyi]=RX_BUF[8];
	 keyi++;
	}
	/*---------------------------------------------------*/
	
	if (strstr(RX_BUF,"SMS Ready") !=0 || strstr(RX_BUF,"NO CARRIER") !=0 ) {interval=45; USARTSend("\r\r"); Delay_ms(150); USARTSend("\r AT+CLIP=1;+DDET=1\r");USARTSend("\rAT+CMGDA=\"DEL ALL\"\r");}
	
		/*Обработка  SMS */
	if (strstr(RX_BUF,"\"SM\"") != 0){ //+CMTI
      number_message=RX_BUF[(strrchr(RX_BUF,',')-RX_BUF)+1];
	    Delay_ms(1000);
		  USARTSend("\r\r"); Delay_ms(1000); USARTSend("\rAT+CMGR=");
		  USARTSend_char(number_message);
		  USARTSend(",0\r");
     	}
if (strstr(RX_BUF, "+CMGR:") != 0)
	{
		if (strstr(RX_BUF, phone)!=0 ||strstr(RX_BUF, phone2)!=0 ) //Проверка с моего номера или нет.
		{
			SMS_read=2;
		}
		else
		{
			USARTSend("\r\r"); Delay_ms(500); USARTSend("\rAT+CMGDA=\"DEL ALL\"\r");
		}
	}
	if (strstr(RX_BUF, command1)!=0 && SMS_read!=0) // Команда 1 на запуск
			{
			Engine_start();
			Send_SMS();
			SMS_read=0;
			USARTSend("\r\r"); Delay_ms(500); USARTSend("\rAT+CMGDA=\"DEL ALL\"\r");
			}
			if (strstr(RX_BUF, command2)!=0 && SMS_read!=0) // Команда 2 на остановку
			{
			Engine_stop();
			Send_SMS();
			SMS_read=0;
			USARTSend("\r\r"); Delay_ms(150); USARTSend("\rAT+CMGDA=\"DEL ALL\"\r");
			}
   	clear_RXBuffer();
        		 
    }  //End USART Rx
	  } //End While
}



void USART1_IRQHandler(void) 
{
    if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
    {
            RXc = USART_ReceiveData(USART1);
						#ifdef Test
            USART_SendData(USART3, USART_ReceiveData(USART1));
			      #endif
            RX_BUF[RXi] = RXc;
            RXi++;			
            if (RXc != 13) 
						{
                if (RXi > RX_BUF_SIZE-1) 
				        {
                    clear_RXBuffer();
                }
            }
            else 
						{
                RX_FLAG_END_LINE = 1;
            }
						
						if (RXc==0)
						{
							RX_BUF[RXi-1]='\r';
						}
						if (RXc==37)
						{
							RX_BUF[RXi]='\r';
							RX_FLAG_END_LINE = 1;
							RXi++;
						}						
    }
}
void TIM4_IRQHandler(void) 
{
        if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
        { 
          TIM_ClearITPendingBit(TIM4, TIM_IT_Update);				
					if (allow_sleep==ENABLE && heating==false && broker==true) 
						{
						    sleep_timer++;
						}
					if (heating==true)
					{
						light=true;
					}
					interval++;
					
				}
}

 void ADC1_2_IRQHandler(void)
{
	if(ADC_GetITStatus(ADC1,ADC_IT_EOC))  
	{
		  Vbat=ADC_GetConversionValue(ADC1)/xADC; 
		  ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
	}
	
}
void USART3_IRQHandler(void) 
{
    if ((USART3->SR & USART_FLAG_RXNE) != (u16)RESET)
    {
			#ifdef Test
			      USART_SendData(USART1, USART_ReceiveData(USART3));
			#endif
    }
}
