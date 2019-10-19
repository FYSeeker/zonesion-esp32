#include "EXTI/exti.h"
#include "delay.h"


//�ⲿ�жϳ�ʼ������
void EXTI0_Init(uint8_t GPIO_PortSourceX)
{
  
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//�ⲿ�жϣ���Ҫʹ��AFIOʱ��
  //�ж����Լ��жϳ�ʼ������
  GPIO_EXTILineConfig(GPIO_PortSourceX,GPIO_PinSource0);
  
  EXTI_InitStructure.EXTI_Line = GPIO_PinSource0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);  	  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}

void EXTI0_IRQHandler(void)
{
	EXTI_ClearITPendingBit(EXTI_Line0);  //���EXTI0��·����λ
}



//�ⲿ�жϳ�ʼ������
void EXTI9_5_Init(uint8_t GPIO_PortSourceX,uint8_t GPIO_PinSourceX)
{
  uint32_t EXTI_Line_x = 0;
  
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//�ⲿ�жϣ���Ҫʹ��AFIOʱ��
  //�ж����Լ��жϳ�ʼ������
  EXTI_DeInit();
  
  GPIO_EXTILineConfig(GPIO_PortSourceX,GPIO_PinSourceX);
  
  switch(GPIO_PinSourceX)
  {
  case GPIO_PinSource5:EXTI_Line_x = EXTI_Line5;
    break;
  case GPIO_PinSource6:EXTI_Line_x = EXTI_Line6;
    break;
  case GPIO_PinSource7:EXTI_Line_x = EXTI_Line7;
    break;
  case GPIO_PinSource8:EXTI_Line_x = EXTI_Line8;
    break;
  case GPIO_PinSource9:EXTI_Line_x = EXTI_Line9;
    break;
  }
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line_x;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//�����ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);  	  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}


void (*EXTI9_5_IrqCall[5])(void) = {NULL};

short EXTI9_5_IrqCallSet(uint8_t index,void(*func)(void))
{
  if((index < 5) || (index > 9)) return 1;
  EXTI9_5_IrqCall[index-5] = func;
  return 0;
}


void EXTI9_5_IRQHandler(void)
{			
  if(EXTI_GetITStatus(EXTI_Line5) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line5);    //����жϱ�־λ  
    if(EXTI9_5_IrqCall[0] != NULL)
      EXTI9_5_IrqCall[0]();
  }
  if(EXTI_GetITStatus(EXTI_Line6) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line6);    //����жϱ�־λ  
    if(EXTI9_5_IrqCall[1] != NULL)
      EXTI9_5_IrqCall[1]();
  }
  if(EXTI_GetITStatus(EXTI_Line7) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line7);    //����жϱ�־λ 
    if(EXTI9_5_IrqCall[2] != NULL)
      EXTI9_5_IrqCall[2](); 
  }
  if(EXTI_GetITStatus(EXTI_Line8) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line8);    //����жϱ�־λ  
    if(EXTI9_5_IrqCall[3] != NULL)
      EXTI9_5_IrqCall[3]();
  }
  if(EXTI_GetITStatus(EXTI_Line9) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line9);    //����жϱ�־λ 
    if(EXTI9_5_IrqCall[4] != NULL)
      EXTI9_5_IrqCall[4](); 
  }
}


//�ⲿ�жϳ�ʼ������
void EXTI15_10_Init(uint8_t GPIO_PortSourceX,uint8_t GPIO_PinSourceX)
{
  
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//�ⲿ�жϣ���Ҫʹ��AFIOʱ��
  //�ж����Լ��жϳ�ʼ������
  GPIO_EXTILineConfig(GPIO_PortSourceX,GPIO_PinSourceX);
  
  EXTI_InitStructure.EXTI_Line = GPIO_PinSourceX;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);  	  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}

void EXTI15_10_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line15);  //���LINE15��·����λ
}
