#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "sdram.h"
#include "w25qxx.h"
#include "nand.h"  
#include "sdmmc_sdcard.h"
#include "usmart.h"
#include "malloc.h"
#include "ftl.h"  
#include "ff.h"
#include "fontupd.h"
#include "text.h"
#include "exfuns.h"
#include "atk_ncr.h"
#include "touch.h"
#include "cnn.h"
/************************************************
 ALIENTEK ������STM32F7������ ʵ��53
 ��дʶ��ʵ��-HAL�⺯����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//����¼�Ĺ켣����
atk_ncr_point READ_BUF[200];					 

//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//������֮��ľ���ֵ 
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ

void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  

		gui_fill_circle(uRow,uCol,size,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}

uint8_t inputdata[220][220]={0};

int main(void)
{
    u8 i=0;	    	
	u8 tcnt=0;    
	u8 res[10];
	u8 key;		    
	u16 pcnt=0;
	u8 mode=4;			            //Ĭ���ǻ��ģʽ	    	    
 	u16 lastpos[2];		            //���һ�ε����� 	
    
    Cache_Enable();                 //��L1-Cache
    HAL_Init();				        //��ʼ��HAL��
    Stm32_Clock_Init(432,25,2,9);   //����ʱ��,216Mhz 
    delay_init(216);                //��ʱ��ʼ��
	  uart_init(115200);		        //���ڳ�ʼ��
		usmart_dev.init(108); 		    //��ʼ��USMART
    //LED_Init();                     //��ʼ��LED
    KEY_Init();                     //��ʼ������
    SDRAM_Init();                   //��ʼ��SDRAM
    LCD_Init();                     //��ʼ��LCD
    W25QXX_Init();				    //��ʼ��W25Q256
    tp_dev.init();				    //��ʼ��������
    my_mem_init(SRAMIN);            //��ʼ���ڲ��ڴ��
    my_mem_init(SRAMEX);            //��ʼ���ⲿSDRAM�ڴ��
    my_mem_init(SRAMDTCM);           //��ʼ���ڲ�CCM�ڴ��
   // alientek_ncr_init();		    //��ʼ����дʶ�� 
    while(font_init())			//����ֿ�
	{	    
		LCD_ShowString(60,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,240,66,WHITE);//�����ʾ	     
	}
RESTART:
  	POINT_COLOR=BLUE;//���û�����ɫ     
//	Show_Str(60,10,200,16,"������STM32F4/F7������",16,0);				    	 
//	Show_Str(60,30,200,16,"��дʶ��ʵ��",16,0);				    	 
//	Show_Str(60,50,200,16,"����ԭ��@ALIENTEK",16,0);				    	 
	//Show_Str(20,30,200,32,"CNN��д����ʶ��",32,0);			    
	Show_Str(20,70,200,32,"ʶ����:",32,0);			    
	LCD_DrawRectangle(19,114,lcddev.width-20,lcddev.height-135);
 	POINT_COLOR=BLUE;      
	//Show_Str(96,207,200,16,"��д��",16,0);	 
	tcnt=100;
	tcnt=100;
	while(1)
	{
		key=KEY_Scan(0);
		if(key==KEY2_PRES&&(tp_dev.touchtype&0X80)==0)
		{
			TP_Adjust();  	//��ĻУ׼
			LCD_Clear(WHITE); 
			goto RESTART;	//���¼��ؽ���
		}
		if(key==KEY0_PRES)	
		{
			LCD_Fill(20,115,219,314,WHITE);//�����ǰ��ʾ
			mode++;
			if(mode>4)mode=1;
			switch(mode)
			{
				case 1:
					Show_Str(80,207,200,16,"��ʶ������",16,0);	
					break;	 	    
				case 2:
					Show_Str(64,207,200,16,"��ʶ���д��ĸ",16,0);	
					break;	 	    
				case 3:
					Show_Str(64,207,200,16,"��ʶ��Сд��ĸ",16,0);	
					break;	 	    
				case 4:
					Show_Str(88,207,200,16,"ȫ��ʶ��",16,0);	
					break;	 
			}
			tcnt=100;
		}		
		if(key==WKUP_PRES)	
		{
			LCD_Fill(20,115,lcddev.width-20-1,lcddev.height-135-1,WHITE);
		}		
 		tp_dev.scan(0);//ɨ��
 		if(tp_dev.sta&TP_PRES_DOWN)//�а���������
		{				  
			delay_ms(1);//��Ҫ����ʱ,��������Ϊ�а�������.
 			tcnt=0;//�ɿ�ʱ�ļ�������� 	 		    
			if((tp_dev.x[0]<(lcddev.width-20-2)&&tp_dev.x[0]>=(20+2))&&(tp_dev.y[0]<(lcddev.height-135-2)&&tp_dev.y[0]>=(115+2)))
			{			 
				if(lastpos[0]==0XFFFF)
				{
					lastpos[0]=tp_dev.x[0];
					lastpos[1]=tp_dev.y[0];
				}
				lcd_draw_bline(lastpos[0],lastpos[1],tp_dev.x[0],tp_dev.y[0],6,BLUE);//����
				lastpos[0]=tp_dev.x[0];
				lastpos[1]=tp_dev.y[0];			
				if(pcnt<200)//�ܵ�������200
				{
					if(pcnt)
					{
						if((READ_BUF[pcnt-1].y!=tp_dev.y[0])&&(READ_BUF[pcnt-1].x!=tp_dev.x[0]))//x,y�����
						{
							READ_BUF[pcnt].x=tp_dev.x[0];
							READ_BUF[pcnt].y=tp_dev.y[0]; 
							pcnt++;
						}	
					}else 
					{
						READ_BUF[pcnt].x=tp_dev.x[0];
						READ_BUF[pcnt].y=tp_dev.y[0]; 
						pcnt++;
					}		  
				}   						  				  
			}    
		}else //�����ɿ��� 
		{
			lastpos[0]=0XFFFF;
			tcnt++;
			delay_ms(10);	  
			//��ʱʶ��
			i++;	 	    
			if(tcnt==40)
			{
				if(pcnt)//����Ч������		 
				{
					for(int i =26;i<=245;i++)
					{
						for(int j=121;j<=340;j++)
						{
							if(LCD_ReadPoint(i,j)==0x001F)
							{
								POINT_COLOR=RED;//���û�����ɫ
								LCD_DrawPoint(i,j);
								inputdata[i-26][j-121] =255;
								
							}		
							else
							{
								POINT_COLOR=WHITE;//���û�����ɫ
								LCD_DrawPoint(i,j);
								inputdata[i-26][j-121]=0;
							}
						}
					}
					u8 temp;
					printf("�ܵ���:%d\r\n",pcnt);
					mode=1;
					//alientek_ncr(READ_BUF,pcnt,6,mode,(char*)res);
					printf("ʶ����:%s\r\n",res);
					u8 num;
					num=cnn_net(inputdata);
					pcnt=0;	 
					temp = res[0];
	  			POINT_COLOR=BLUE;//���û�����ɫ
		 			LCD_ShowString(60+104,70,200,32,32,&num);	
					LCD_ShowString(10,70,200,16,16,&temp);								
				}
				//LCD_Fill(20,115,lcddev.width-20-1,lcddev.height-5-1,WHITE);
			} 
		}  
		if(i==30)
		{
			i=0;
			LED0_Toggle;
		}		   
	}  							  	       
}
