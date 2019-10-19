/*********************************************************************************************
* �ļ���lte-ec20.c
* ���ߣ�xuzhy 2018.1.11
* ˵����lte esp32 ģ������
*       
*       
*       
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#include "esp32.h"

#include <contiki.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "config.h"
#include "zxbee-inf.h"

#define DEBUG 0

#if DEBUG
#define Debug      printf
#define DebugAT    printf
#else
#define Debug(...)
#define DebugAT(...)
#endif

#define _RESPONSE_BUF_SIZE 256  //atָ����ջ��泤��

PROCESS(esp32, "esp32");

/*esp32�ײ�ӿں���*/
extern void esp32_uart_write(char *buf, int len);
extern void esp32_inf_init(void);
extern int esp32_uart_read_ch(void);


/* atָ������ṹ */
struct _at_request{
  void *req;
  void (*fun)(char * response);
  unsigned int timeout_tm;
  struct _at_request *next;
};


typedef struct
{
  uint8_t esp32;
  uint8_t mode;
  uint8_t connect;
  uint8_t udp;
  uint8_t init;
  char mac[18];
}DevStatas_t;

struct _at_request *current_at = NULL; //���浱ǰ����ִ�е�atָ��
struct _at_request * list_at = NULL;  //����atָ�������б�

DevStatas_t dev_status = {0};

static char  _response_buf[_RESPONSE_BUF_SIZE];
static int _response_offset = 0;

static char send_buf[1024];
static int send_len = 0;


/*********************************************************************************************
* ���ƣ�trim
* ���ܣ�ȥ���ַ���ǰ��ո�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void trim ( char *s )
{
  char *p;
  int i;
  
  p = s;
  while ( *p == ' ' || *p == '\t' ) {*p++;}
  strcpy ( s,p );
  
  i = strlen ( s )-1;
  while ( ( s[i] == ' ' || s[i] == '\t' ) && i >= 0 ) {i--;};
  s[i+1] = '\0';
}


/*********************************************************************************************
* ���ƣ�_request_at_1
* ���ܣ�����atָ������������������Ͷ���
* ������at ����Ҫִ�е�atָ��
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void _request_at_1(char *at)
{
  _request_at_3(at, 1000, NULL);
}

/*********************************************************************************************
* ���ƣ�_request_at_1
* ���ܣ�����atָ�����ʱ���ͽ������ص�����
* ������at����Ҫִ�е�atָ��
*       to��ָ��ִ�г�ʱʱ�䣬��λ ms
*       fun���������ص�����
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int _request_at_3(char *at, unsigned int to, void (*fun)(char*))
{
  int len = strlen(at);
  char *p = malloc(len+1);
  if (p) strcpy(p, at);
  else return -1;
  struct _at_request *pa = malloc(sizeof (struct _at_request));
  if (pa) {
    pa->req = p;
    pa->next = NULL;
    pa->fun = fun;
    pa->timeout_tm = to; //��¼��ʱʱ��
    if (list_at == NULL) list_at = pa;
    else {
      struct _at_request *pl = list_at;
      while (pl->next != NULL) pl = pl->next;
      pl->next = pa; 
    }
  } else {
    free(p);
    return -1;
  }
  return 0;
}

/*********************************************************************************************
* ���ƣ�end_process_at
* ���ܣ�atָ�������ͷ��ڴ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void end_process_at(void)
{
  if (current_at != NULL) {
    free(current_at->req);
    free(current_at);
    current_at = NULL;
  }
}

/*********************************************************************************************
* ���ƣ�esp32_Check
* ���ܣ����esp32ģ���Ƿ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void _rp_check_esp32(char *rsp)
{
  if (rsp == NULL) 
  {
    //timeout
  } 
  else if (memcmp(rsp, "OK", 2) == 0) 
  {
    dev_status.esp32 = 1;
    end_process_at();
  }
}

static void esp32_Check(void)
{
  _request_at_3("ATE0\r\n", 500, _rp_check_esp32);
}

/*********************************************************************************************
* ���ƣ�esp32_GetStatus
* ���ܣ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void esp32_GetStatus_call(char *rsp)
{
  short status = 0;
  if (rsp == NULL) 
  {
    //timeout
  } 
  else if (memcmp(rsp, "STATUS:", 7) == 0) 
  {
    status = atoi((const char*)&rsp[7]);
    if(status == 0)
      dev_status.mode = 0;
    else
      dev_status.mode = 1;
    
    if((status == 2) || (status == 3))
      dev_status.connect = 1;
    else
      dev_status.connect = 0;
  }
  else if (memcmp(rsp, "+CIPSTATUS:", 11) == 0) 
  {
    char* pdat = "0,\"UDP\",\"192.168.43.1\",7003,7004,0";
    char* ptr = &rsp[11];
    
    if(memcmp(ptr,pdat,strlen(pdat)) == 0)
      dev_status.udp = 1;
    else
      dev_status.udp = 0;
  }
  else if (memcmp(rsp, "OK", 2) == 0) 
  {
    end_process_at();
  }
}

static void esp32_GetStatus(void)
{
  _request_at_3("AT+CIPSTATUS\r\n", 500, esp32_GetStatus_call);
}

/*********************************************************************************************
* ���ƣ�esp32_SetMode
* ���ܣ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void esp32_SetMode_call(char *rsp)
{
  if (rsp == NULL) 
  {
    //timeout
  } 
  else if (memcmp(rsp, "OK", 2) == 0) 
  {
    dev_status.mode = 1;
    end_process_at();
  }
}

static void esp32_SetMode(void)
{
  _request_at_3("AT+CWMODE=1\r\n", 500, esp32_SetMode_call);
}


/*********************************************************************************************
* ���ƣ�esp32_GetMAC
* ���ܣ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void esp32_GetMAC_call(char *rsp)
{
  if (rsp == NULL) 
  {
    //timeout
  } 
  else if (memcmp(rsp, "+CIPSTAMAC:", 11) == 0) 
  {
    memcpy(dev_status.mac,(const char*)&rsp[12],17);
  }
  else if (memcmp(rsp, "OK", 2) == 0) 
  {
    end_process_at();
  }
}

static void esp32_GetMAC(void)
{
  _request_at_3("AT+CIPSTAMAC?\r\n", 500, esp32_GetMAC_call);
}


/*********************************************************************************************
* ���ƣ�esp32_Connect
* ���ܣ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void esp32_Connect_call(char* rsp)
{
  if (rsp == NULL)
  {
    //timeout
  } 
  else if (memcmp(rsp, "ERROR", 5) == 0) 
  {
    end_process_at();
  }
  else if (memcmp(rsp, "OK", 2) == 0) 
  {
    dev_status.connect = 1;
    end_process_at();
  }
}

static void esp32_Connect(char* ssid,char* password)
{
  char buf[128] = {0};
  sprintf(buf,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,password);
  _request_at_3(buf, 5000, esp32_Connect_call);
}

/*********************************************************************************************
* ���ƣ�esp32_CloseUDP
* ���ܣ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
/*
static void esp32_CloseUDP_call(char* rsp)
{
  if (rsp == NULL) 
  {
    //timeout
  } 
  else if ( (memcmp(rsp, "OK", 2) == 0)  ||
           (memcmp(rsp, "ERROR", 2) == 0) )
  {
    dev_status.udp = 0;
    end_process_at();
  }
}

static void esp32_CloseUDP()
{
  _request_at_3("AT+CIPCLOSE\r\n", 1000, esp32_CloseUDP_call);
}
*/

/*********************************************************************************************
* ���ƣ�esp32_OpenUDP
* ���ܣ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void esp32_OpenUDP_call(char* rsp)
{
  if (rsp == NULL) 
  {
    //timeout
  } 
  else if (memcmp(rsp, "OK", 2) == 0) 
  {
    dev_status.udp = 1;
    end_process_at();
  }
}

static void esp32_OpenUDP(char* sip,uint32_t rPort,uint32_t lPort)
{
  char buf[128] = {0};
  sprintf(buf,"AT+CIPSTART=\"UDP\",\"%s\",%d,%d,0\r\n",sip,rPort,lPort);
  _request_at_3(buf, 2000, esp32_OpenUDP_call);
}


/*********************************************************************************************
* ���ƣ�__on_send_data
* ���ܣ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void __on_send_data(char *rsp)
{
  if (rsp == NULL) { //timeout
    send_len = 0;
  } 
  else if (rsp[0] == '>') 
  {
    /*  write data */
    Debug("<-- ");
    for (int i=0; i<send_len; i++) {
      Debug("%c", send_buf[i]);
    }
    Debug("\r\n");
    
    esp32_uart_write(send_buf, send_len);
  } 
  else 
  {
    //trim(rsp);
    //Debug("--> %s\r\n", rsp);
    if (strlen(rsp)>0 && memcmp(rsp, "SEND",4) == 0)
    {
      send_len = 0;
      end_process_at();
    }
  }
}

/*********************************************************************************************
* ���ƣ�esp32_tcp_send
* ���ܣ���������
* ������dat�����������ݻ����ַ
*       len�����������ݳ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int esp32_SendData(char *dat,int len)
{
  char buf[32] = {0};
  if (len > 1024) return -1;
  if (send_len != 0)
  {
    Debug("Send fail,data buffer busy\r\n");
    return -2; //busy
  }
  
  if(dev_status.init == 1)
  {
    
    sprintf(send_buf,"%s=%s",&dev_status.mac[0],dat);
    send_len = len+18;
    sprintf(buf, "AT+CIPSEND=%d\r\n",send_len);
    _request_at_3(buf, 100, __on_send_data);
    return 0;
  }
  return -3;
}


/*********************************************************************************************
* ���ƣ�esp32_tcp_send
* ���ܣ���������
* ������dat�����������ݻ����ַ
*       len�����������ݳ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void esp32_RecvData(char* dat)
{
  ZXBeeInfRecv(dat,strlen(dat));
  free(dat);
}


static struct ctimer esp32_timer;

/*********************************************************************************************
* ���ƣ�e_esp32_timer_call
* ���ܣ���ʱ���ص�����������esp32ģ��״̬
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void _esp32_timer_call(void *ptr)
{
  static int tick = 0;
  
  ctimer_reset(&esp32_timer);
  
  if(dev_status.init == 0)
  {
    if (dev_status.esp32 == 0) 
    {
      if (tick % 2 == 0) 
      {
        esp32_Check();
        tick = 0;
      }
    }
    else if(dev_status.mac[0] == 0)
    {
      esp32_GetMAC();
      esp32_GetStatus();
      tick = 0;
    }
    else if(dev_status.mode == 0)
    {
      esp32_SetMode();
      tick = 0;
    }
    else if (dev_status.connect == 0) 
    {
      if (tick % 10 == 0) 
      {
        esp32_Connect(WiFiConfig.ssid,WiFiConfig.passward);
      }
    }
    else if (dev_status.udp == 0) 
    {
      if (tick % 8 == 0) 
      {
        esp32_OpenUDP(WiFiConfig.sip,WiFiConfig.rport,WiFiConfig.lport);
      }
    }
    if(dev_status.udp == 1)
    {
      leds_on(1);
      dev_status.init = 1;
    }
  }
  tick ++;
}

/*********************************************************************************************
* ���ƣ�_poll_request
* ���ܣ�at������ѯ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void _poll_request(void)
{
  if (current_at == NULL) {
    current_at = list_at;
    if (current_at != NULL) {
      DebugAT("<-- %s", (char*)current_at->req);
      
      current_at->timeout_tm = clock_time()+ current_at->timeout_tm;
      esp32_uart_write(current_at->req, strlen(current_at->req));
      list_at = list_at->next;
    }
  } else {
    unsigned int tm = clock_time();
    if (((int)(tm - current_at->timeout_tm)) > 0) {
      DebugAT("AT timeout: %s\r\n",current_at->req);
      if (current_at->fun != NULL) {
        current_at->fun(NULL);
      }
      end_process_at();
    } 
  }
}
/*********************************************************************************************
* ���ƣ�_poll_response
* ���ܣ�����at��Ӧ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void _poll_response(void)
{
  int ch;
  ch = esp32_uart_read_ch();
  while (ch >= 0) 
  {
    if (_response_offset == 0 && ch == '>') 
    {
      DebugAT(">");
      if (current_at != NULL && current_at->fun != NULL) 
      {    
        current_at->fun(">");
      }
    }
    else
    {
      _response_buf[_response_offset++] = ch;
      if ((_response_offset>1) &&
          (_response_buf[_response_offset-2] == '\r') &&
          (_response_buf[_response_offset-1] == '\n')) 
      {
        _response_buf[_response_offset-2] = '\0';
        _response_offset -= 2;
        if (_response_offset > 0) 
        {
          DebugAT("--> %s\r\n", _response_buf);
          if (current_at != NULL && current_at->fun != NULL) {
            current_at->fun(_response_buf);
          }
          if (memcmp(_response_buf, "ERROR", 7) == 0) 
          {
            end_process_at();
          }
          
          if (memcmp(_response_buf, "+IPD,", 5) == 0) 
          {
            short DataLen = atoi((const char*)&_response_buf[5]);
            char* ptr = NULL;
            char* pdata = malloc(DataLen);
            if(pdata != NULL)
            {
              ptr = strstr((const char*)&_response_buf[5],":");
              memcpy(pdata,(const char*)ptr+1,DataLen);
              esp32_RecvData(pdata);
            }
          }
        }
        _response_offset = 0;
      }
    }
    
    /**/
    ch = esp32_uart_read_ch();
  }
}
//////////////////////////////////////////////////////

PROCESS_THREAD(esp32, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  
  esp32_inf_init();
  
  etimer_set(&et, CLOCK_SECOND/10); // 100 ms
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER); 
  
  ctimer_set(&esp32_timer, CLOCK_SECOND/2, _esp32_timer_call, NULL);
  while (1) 
  {
    _poll_request();
    _poll_response();
    etimer_set(&et, CLOCK_SECOND/100);
    PROCESS_YIELD();
  }
  PROCESS_END();
}