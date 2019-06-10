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
 ALIENTEK 阿波罗STM32F7开发板 实验53
 手写识别实验-HAL库函数版
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//最大记录的轨迹点数
atk_ncr_point READ_BUF[200];					 

//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
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
//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色

void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  

		gui_fill_circle(uRow,uCol,size,color);//画点 
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
	u8 mode=4;			            //默认是混合模式	    	    
 	u16 lastpos[2];		            //最后一次的数据 	
    
    Cache_Enable();                 //打开L1-Cache
    HAL_Init();				        //初始化HAL库
    Stm32_Clock_Init(432,25,2,9);   //设置时钟,216Mhz 
    delay_init(216);                //延时初始化
	  uart_init(115200);		        //串口初始化
		usmart_dev.init(108); 		    //初始化USMART
    //LED_Init();                     //初始化LED
    KEY_Init();                     //初始化按键
    SDRAM_Init();                   //初始化SDRAM
    LCD_Init();                     //初始化LCD
    W25QXX_Init();				    //初始化W25Q256
    tp_dev.init();				    //初始化触摸屏
    my_mem_init(SRAMIN);            //初始化内部内存池
    my_mem_init(SRAMEX);            //初始化外部SDRAM内存池
    my_mem_init(SRAMDTCM);           //初始化内部CCM内存池
   // alientek_ncr_init();		    //初始化手写识别 
    while(font_init())			//检查字库
	{	    
		LCD_ShowString(60,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,240,66,WHITE);//清除显示	     
	}
RESTART:
  	POINT_COLOR=BLUE;//设置画笔蓝色     
//	Show_Str(60,10,200,16,"阿波罗STM32F4/F7开发板",16,0);				    	 
//	Show_Str(60,30,200,16,"手写识别实验",16,0);				    	 
//	Show_Str(60,50,200,16,"正点原子@ALIENTEK",16,0);				    	 
	//Show_Str(20,30,200,32,"CNN手写数字识别",32,0);			    
	Show_Str(20,70,200,32,"识别结果:",32,0);			    
	LCD_DrawRectangle(19,114,lcddev.width-20,lcddev.height-135);
 	POINT_COLOR=BLUE;      
	//Show_Str(96,207,200,16,"手写区",16,0);	 
	tcnt=100;
	tcnt=100;
	while(1)
	{
		key=KEY_Scan(0);
		if(key==KEY2_PRES&&(tp_dev.touchtype&0X80)==0)
		{
			TP_Adjust();  	//屏幕校准
			LCD_Clear(WHITE); 
			goto RESTART;	//重新加载界面
		}
		if(key==KEY0_PRES)	
		{
			LCD_Fill(20,115,219,314,WHITE);//清除当前显示
			mode++;
			if(mode>4)mode=1;
			switch(mode)
			{
				case 1:
					Show_Str(80,207,200,16,"仅识别数字",16,0);	
					break;	 	    
				case 2:
					Show_Str(64,207,200,16,"仅识别大写字母",16,0);	
					break;	 	    
				case 3:
					Show_Str(64,207,200,16,"仅识别小写字母",16,0);	
					break;	 	    
				case 4:
					Show_Str(88,207,200,16,"全部识别",16,0);	
					break;	 
			}
			tcnt=100;
		}		
		if(key==WKUP_PRES)	
		{
			LCD_Fill(20,115,lcddev.width-20-1,lcddev.height-135-1,WHITE);
		}		
 		tp_dev.scan(0);//扫描
 		if(tp_dev.sta&TP_PRES_DOWN)//有按键被按下
		{				  
			delay_ms(1);//必要的延时,否则老认为有按键按下.
 			tcnt=0;//松开时的计数器清空 	 		    
			if((tp_dev.x[0]<(lcddev.width-20-2)&&tp_dev.x[0]>=(20+2))&&(tp_dev.y[0]<(lcddev.height-135-2)&&tp_dev.y[0]>=(115+2)))
			{			 
				if(lastpos[0]==0XFFFF)
				{
					lastpos[0]=tp_dev.x[0];
					lastpos[1]=tp_dev.y[0];
				}
				lcd_draw_bline(lastpos[0],lastpos[1],tp_dev.x[0],tp_dev.y[0],6,BLUE);//画线
				lastpos[0]=tp_dev.x[0];
				lastpos[1]=tp_dev.y[0];			
				if(pcnt<200)//总点数少于200
				{
					if(pcnt)
					{
						if((READ_BUF[pcnt-1].y!=tp_dev.y[0])&&(READ_BUF[pcnt-1].x!=tp_dev.x[0]))//x,y不相等
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
		}else //按键松开了 
		{
			lastpos[0]=0XFFFF;
			tcnt++;
			delay_ms(10);	  
			//延时识别
			i++;	 	    
			if(tcnt==40)
			{
				if(pcnt)//有有效的输入		 
				{
					for(int i =26;i<=245;i++)
					{
						for(int j=121;j<=340;j++)
						{
							if(LCD_ReadPoint(i,j)==0x001F)
							{
								POINT_COLOR=RED;//设置画笔蓝色
								LCD_DrawPoint(i,j);
								inputdata[i-26][j-121] =255;
								
							}		
							else
							{
								POINT_COLOR=WHITE;//设置画笔蓝色
								LCD_DrawPoint(i,j);
								inputdata[i-26][j-121]=0;
							}
						}
					}
					u8 temp;
					printf("总点数:%d\r\n",pcnt);
					mode=1;
					//alientek_ncr(READ_BUF,pcnt,6,mode,(char*)res);
					printf("识别结果:%s\r\n",res);
					u8 num;
					num=cnn_net(inputdata);
					pcnt=0;	 
					temp = res[0];
	  			POINT_COLOR=BLUE;//设置画笔蓝色
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
