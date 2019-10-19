/*********************************************************************************************
* �ļ���zxbee-sys-command.c
* ���ߣ�xuzhy 2018.1.12
* ˵����lte ģ�鹫��zxbeeָ���ģ��
*       
*       
*       
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#include "contiki.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stimer.h>
#include "zxbee.h"
#include "esp32.h"
#include "usart.h"
#include "sensor.h"


/*********************************************************************************************
* ���ƣ�on_command_begin
* ���ܣ�zxbeeָ���ʼ���ã�Ϊ��֧��lteģ�岦��绰�ͷ��Ͷ��Ź��ܶ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void on_command_begin()
{
}
/*********************************************************************************************
* ���ƣ�on_command_end
* ���ܣ�zxbeeָ�����ϵ��ã�Ϊ��֧��lteģ�岦��绰�ͷ��Ͷ��Ź��ܶ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void on_command_end()
{
}
/*********************************************************************************************
* ���ƣ�zxbee_sys_process_command_call
* ���ܣ�lteģ�鹫��zxbeeָ���
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int ZXBeeSysCommandProc(char* ptag, char* pval)
{
  int ret = -1;
  if (memcmp(ptag, "ECHO", 4) == 0) {
    ZXBeeAdd(ptag, pval);
    return 1;
  }
  if (memcmp(ptag, "TYPE", 4) == 0) {
    if (pval[0] == '?') {
 
      int radio_type = CONFIG_RADIO_TYPE; //lora
      int dev_type = CONFIG_DEV_TYPE; //lora ed
      char buf[16];
      ret = sprintf(buf, "%d%d%s", radio_type, dev_type, NODE_NAME);
      ZXBeeAdd("TYPE", buf);
      return 1;
    }
  }
  
  return ret;
}



