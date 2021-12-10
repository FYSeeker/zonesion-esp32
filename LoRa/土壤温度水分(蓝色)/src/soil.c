#include "soil.h"
#include "usart.h"

char readDataBuf[8] = {0xfa,0x03,0x00,0x00,0x00,0x02,0xd1,0x80};// ��ȡ������ʪ�ȴ���������ָ��
char setZoomFactor[8] = {0xfa,0x06,0x0a,0x00,0x27,0x10,0x85,0xa5};//��������ϵ����1
float soilHumi = 0;
float soilTemp = 0;

/*********************************************************************************************
* ���ƣ�uart2_set_input_call()
* ���ܣ����ô���2�ص�����
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart2_set_input_call(int (*c)(char ch))
{
  uart2_set_input(c);
}

/*********************************************************************************************
* ���ƣ�uart2_send_data()
* ���ܣ�����2��������
* ������buf-��Ҫ���͵�����/len-���ݳ���
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart2_send_data(char* buf, unsigned int len)
{
  for(unsigned int i=0; i<len; i++)
    uart2_putc(buf[i]);
}

float get_soilHumi(void)
{
  return soilHumi;
}

float get_soilTemp(void)
{
  return soilTemp;
}

/*********************************************************************************************
* ���ƣ�soil_init()
* ���ܣ�����������������ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void soil_init(void)
{
  uart2_init(9600);
  uart2_set_input_call(uart2_rec_handle);
  uart2_send_data(setZoomFactor, sizeof(setZoomFactor));
}

void read_soil(void)
{
  uart2_send_data(readDataBuf, sizeof(readDataBuf));
}

/*********************************************************************************************
* ���ƣ�uart2_rec_handle()
* ���ܣ�����2�ص�����
* ������ch-���ڽ��յ�����
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int uart2_rec_handle(char ch)
{
#define BUFSIZE 128
  static unsigned int recLen = 0;
  static char recBuf[BUFSIZE];
  static unsigned char flag = 0;
  if(flag == 1)
  {
    recBuf[recLen++] = ch;
    if(recLen >= 7)
    {
      soilHumi = (recBuf[3]*256 + recBuf[4]) * 0.1;         // ���������ˮ��ֵ
      soilTemp = (recBuf[5]*256 + recBuf[6]) * 0.1;         // ����������¶�ֵ
      recLen = 0;
      flag = 0;
    }
  }
  else
  {
    recBuf[0] = recBuf[1];
    recBuf[1] = recBuf[2];
    recBuf[2] = ch;
    if((recBuf[0] == 0x01) && (recBuf[1] == 0x03)&&(recBuf[2] == 0x04))
    {
      recLen = 3;
      flag = 1;
    }
  }
  return 1;
}




