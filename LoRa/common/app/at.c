/*********************************************************************************************
* �ļ���at.c
* ���ߣ�xuzhy 
* ˵����
*       
*       
*      
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

#include <string.h>
#include <ctype.h>

#include "contiki.h"
#include "usart.h"
#include "config.h"
#include "esp32.h"
#include "at-uart.h"
#include "at.h"


PROCESS(at, "at");
static process_event_t  event_at;


#define AT_BUFF_SIZE            256
#define AT_BUFF_NUM	        4
static char at_echo = 1;
static  int at_datalen = 0;     //ָʾ���ڽ��շ��͵����ݳ���
static  int at_recvdata = 0;
static char atbuff[AT_BUFF_NUM][AT_BUFF_SIZE];
static char bufferbit = 0;

/*********************************************************************************************
* ���ƣ�at_quebuffer_get
* ���ܣ�at��������
* ������
* ���أ��ɹ����ػ����ַ��ʧ�ܷ���NULL
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
char* at_quebuffer_get()
{
	for (int i=0; i<AT_BUFF_NUM; i++) {
		if (((bufferbit>>i) & 1) == 0) {
			bufferbit |= 1<<i;
			return atbuff[i];
		}
	}
	return NULL;
}
/*********************************************************************************************
* ���ƣ�at_quebuffer_put
* ���ܣ�at�����ͷ�
* ������buf�����ͷŻ����ַ
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_quebuffer_put(char *buf)
{
	for (int i=0; i<AT_BUFF_NUM; i++) {
		if (atbuff[i] == buf){
			bufferbit &= ~(1<<i);
			return;
		}
	}
}
/*********************************************************************************************
* ���ƣ�_at_get_ch
* ���ܣ�at���ڽ��յ�1���ֽڴ�����
* ������ch�����յ����ֽ�
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static int _at_get_ch(char ch)
{
  static char idx = 0;
  static char *pbuf = NULL;
  if (pbuf == NULL) {
    pbuf = at_quebuffer_get();
    if (pbuf == NULL) { 
      return 0;
    }
  }
  if (at_datalen == 0) {
    if (at_echo) {
      at_uart_write(ch);
    }
    if (idx < AT_BUFF_SIZE-1) {
      pbuf[idx++] = ch;
      if (idx >= 2 && pbuf[idx-2]=='\r' && pbuf[idx-1]=='\n'){
        idx -= 2;
        pbuf[idx] = '\0';
        if (idx > 0) {
          /* ��ȡ��at���� */
          process_post(&at, event_at, (process_data_t)pbuf);
          pbuf = NULL;
          idx = 0;
        }
      }
    } else {
      /*����*/
      idx = 0;
    }
  } else {
    pbuf[at_recvdata++] = ch;
    if (at_recvdata == at_datalen) {
      process_post(&at, event_at, (process_data_t)pbuf);
      //at_recvdata = 0;
      at_datalen = 0;
      pbuf = NULL;
    }
  }
  return 1;
}
/*********************************************************************************************
* ���ƣ�at_response_buf
* ���ܣ�at�ӿڷ���һ������
* ������s�����������ݵ�ַ
*      len�����������ݳ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_response_buf(char *s, int len)
{
  for (int i=0; i<len; i++) {
    at_uart_write(s[i]);
  }
}
/*********************************************************************************************
* ���ƣ�at_response
* ���ܣ�at�ӿڷ���һ���ַ���
* ������s���������ַ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_response(char *s)
{
  at_response_buf(s, strlen(s));
}
/*********************************************************************************************
* ���ƣ�LTEOnMessage
* ���ܣ������Ʒ��������յ����ݲ������at���ڣ���zhiyun.c�б�����
* ������buf�����յ���zxbee����
*       len�� ���ݳ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_notify_data(char *buf, int len)
{
  char buff[32];
  sprintf(buff, "+RECV:%d\r\n", len);
  at_response(buff);
  at_response_buf(buf, len);
}
/*********************************************************************************************
* ���ƣ�ATCommandInit
* ���ܣ�at�ӿڳ�ʼ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ATCommandInit(void)
{

 at_uart_set_input_call(_at_get_ch);
 at_response("+HW:ESP32\r\n");
 at_response("+RDY\r\n");

}

/*********************************************************************************************
* ���ƣ�at
* ���ܣ�at������߳�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
PROCESS_THREAD(at, ev, data)
{

  char buf[64];
  
  PROCESS_BEGIN();
 
  event_at = process_alloc_event();
  ATCommandInit();
  while (1) {
    PROCESS_WAIT_EVENT();
    if (ev == event_at) {
      char *p_msg = (char *)data;
      if (at_recvdata != 0) { //got data
        int r;
        p_msg[at_recvdata] = '\0';
        r = esp32_SendData(p_msg,strlen(p_msg));
        at_response(ATOK);
        at_recvdata = 0;
        if (r < 0) {
          at_response("+DATASEND:Error!\r\n");
        } else {
          sprintf(buf, "+DATASEND:%d\r\n", r);
          at_response(buf);
        }
      } else {
        int msg_size = strlen(p_msg);
        for (int i=0; i<msg_size; i++) {
          if (p_msg[i] == '?' || p_msg[i] == '=') break;
          p_msg[i] = toupper(p_msg[i]);
        }
        if (strcmp(p_msg, "AT") == 0) {
          at_response(ATOK);
        }
        else if (strcmp(p_msg, "ATE1") == 0) {
          at_echo = 1;
          at_response(ATOK);
        }
        else if (strcmp(p_msg, "ATE0") == 0) {
          at_echo = 0;
          at_response(ATOK);
        } 
        else if (strcmp(p_msg, "AT+HW?") == 0) {
          at_response("+HW:ESP32\r\n");
          at_response(ATOK);
        }
        else if (memcmp(p_msg, "AT+SEND=", 8)==0){
          int sz = atoi(&p_msg[8]);
          if (sz > 0 && sz<256) {
            at_response(">");
            at_datalen = sz;
          } else {
            at_response(ATERROR);
          }
        }
        /*
        else if (memcmp(p_msg, "AT+ENVSAVE", 10)==0){
          config_save();
          at_response(ATOK);
        }
        */
        
        else if (memcmp(p_msg, "AT+SSID", 7)==0){
          if (p_msg[6] == '?') {
            at_response("+SSID:");
            at_response(WiFiConfig.ssid);
            at_response("\r\n");
            at_response(ATOK);
            
          } else if (p_msg[7] == '=') {
            strncpy(WiFiConfig.ssid, &p_msg[8], sizeof WiFiConfig.ssid);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }  
        } 
        else if (memcmp(p_msg, "AT+PASSWORD", 11)==0){
          if (p_msg[7] == '?') {
            at_response("+PASSWORD:\"");
            at_response(WiFiConfig.passward);
            at_response("\"\r\n");
            at_response(ATOK);
            
          } else if (p_msg[11] == '=') {
            if (p_msg[12] == '"' && p_msg[msg_size-1] == '"') {
              p_msg[msg_size-1] = '\0';
              strncpy(WiFiConfig.passward, &p_msg[13], sizeof WiFiConfig.passward);
              at_response(ATOK);
            }
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+SIP", 6)==0){
          if (p_msg[6] == '?') {
            at_response("+SIP:");
            at_response(WiFiConfig.sip);
            at_response("\r\n");
            at_response(ATOK); 
          } else if (p_msg[6] == '='){
            strncpy(WiFiConfig.sip, &p_msg[7], sizeof WiFiConfig.sip);
            at_response(ATOK);
          } else {
              at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+RESET", 8) == 0) {
          at_response(ATOK); 
          __set_FAULTMASK(1);
          NVIC_SystemReset();
        } else{
          int8_t user_at_proc(char *msg);
          if (user_at_proc(p_msg) < 0) {
            at_response(ATERROR);
          }
        }
      }
      at_quebuffer_put(p_msg);
    }
  }
  PROCESS_END();
}
