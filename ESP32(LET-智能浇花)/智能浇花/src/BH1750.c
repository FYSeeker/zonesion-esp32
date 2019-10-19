/*********************************************************************************************
* 文件：BH1750.c
* 作者：zonesion
* 说明：BH1750驱动程序
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "BH1750.h"
#include "iic.h"
#include "delay.h"
/*********************************************************************************************
* 全局变量
*********************************************************************************************/
uchar buf[2];                                                   //接收数据缓存区  
float s;

/*********************************************************************************************
* 名称：bh1750_send_byte()
* 功能：向无子地址器件发送字节数据函数，从启动总线到发送地址，数据，结束总线的全过程,从器件地址sla，使用前必须已结束总线
* 参数：无
* 返回：如果返回1表示操作成功，否则操作有误
* 修改：
* 注释：
*********************************************************************************************/
uchar bh1750_send_byte(uchar sla,uchar c)
{
  iic_start();                                                  //启动总线
  if(iic_write_byte(sla) == 0){                                 //发送器件地址
    if(iic_write_byte(c) == 0){                                 //发送数据
    }
  }
  iic_stop();                                                   //结束总线
  return(1);
}

/*********************************************************************************************
* 名称：bh1750_read_nbyte()
* 功能：连续读出BH1750内部数据
* 参数：无
* 返回：应答或非应答信号
* 修改：
* 注释：
*********************************************************************************************/
uchar bh1750_read_nbyte(uchar sla,uchar *s,uchar no)
{ 
  uchar i;	
  iic_start();                                                  //起始信号
  if(iic_write_byte(sla+1) == 0){                             	//发送设备地址+读信号
    for (i=0; i<no-1; i++){                                 	//连续读取6个地址数据，存储中BUF
      *s=iic_read_byte(0);      
      s++;
    }		
    *s=iic_read_byte(1);
  }
  iic_stop();   
  return(1);
}

/*********************************************************************************************
* 名称：bh1750_init()
* 功能：初始化BH1750
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
//初始化BH1750，根据需要请参考pdf进行修改****
void bh1750_init()
{
  iic_init();
}

/*********************************************************************************************
* 名称：bh1750_get_data()
* 功能：BH1750数据处理函数
* 参数：无
* 返回：处理结果
* 修改：
* 注释：
*********************************************************************************************/
float bh1750_get_data(void)
{  
  uchar *p=buf;
  bh1750_init();                                                //初始化BH1750
  bh1750_send_byte(0x46,0x01);                              	//power on
  bh1750_send_byte(0x46,0X20);                                  //H- resolution mode
  delay_ms(180);                                                //延时180ms
  bh1750_read_nbyte(0x46,p,2);                                  //连续读出数据，存储在BUF中
  unsigned short x = buf[0]<<8 | buf[1];
  return x/1.2;
}