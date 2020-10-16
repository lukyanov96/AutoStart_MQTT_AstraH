#include "stm32f10x.h"                  // Device header
#include "can.h"  
#include "mqtt.h"  
#include <string.h>
#include <stdbool.h>                      




 /*-----------------??????? ??? MQTT--------------*/
 void  MQTT_PUB (char MQTT_topic[15], char MQTT_messege[15]) {          // ????? ?? ??????????

  USARTSend_char(0x30); USARTSend_char(strlen(MQTT_topic)+strlen(MQTT_messege)+2);
  USARTSend_char(0); USARTSend_char(strlen(MQTT_topic)); USARTSend(MQTT_topic); // 
  USARTSend(MQTT_messege);   }                                                  
 void  MQTT_PUB_minus (char MQTT_topic[15], char MQTT_messege[15]) {          //?????  ?? ?????????? ??? ????????????? ????????
  USARTSend_char(0x30); USARTSend_char(strlen(MQTT_topic)+strlen(MQTT_messege)+2+1);
  USARTSend_char(0); USARTSend_char(strlen(MQTT_topic)); USARTSend(MQTT_topic); 
  USARTSend_char('-'); USARTSend(MQTT_messege);   }   
void  MQTT_SUB (char MQTT_topic[15]) {                                       // ????? ???????? ?? ?????
  
  USARTSend_char(0x82); USARTSend_char(strlen(MQTT_topic)+5);                          // ????? ??????
  USARTSend_char(0); USARTSend_char(0x01); USARTSend_char(0);                
  USARTSend_char(strlen(MQTT_topic)); USARTSend(MQTT_topic);                      // ?????
  USARTSend_char(0);  }  

void  MQTT_IntPub (char topic[15], int val) {   //??? ?????????? ?????
char st[12]; 
if (val<0){ MQTT_PUB_minus (topic, utoa_builtin_div(val*(-1),st));}
else {    	MQTT_PUB (topic, utoa_builtin_div(val,st));}
}
void  MQTT_PUB_float (char MQTT_topic[15], uint32_t Int) {          // ????? ?? ??????????
  char st[12]; 
  USARTSend_char(0x30); USARTSend_char(strlen(MQTT_topic)+strlen(utoa_builtin_div(Int/10,st))+2+2);
  USARTSend_char(0); USARTSend_char(strlen(MQTT_topic)); USARTSend(MQTT_topic); // 
  USARTSend(utoa_builtin_div(Int/10,st));USARTSend("."); USARTSend(utoa_builtin_div(Int%10,st));  } 

	
