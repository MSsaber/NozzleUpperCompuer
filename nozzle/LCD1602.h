/*************************************************************
                      LCD1602头文件

实现功能：LCD1602的控制

补充说明：
***************************************************************/
#ifndef _LCD1602_H_
#define _LCD1602_H_
#include<reg52.h>

#define uchar unsigned char
#define uint unsigned int

/*****************LCD1602引脚定义*******************/
#define LCD_DB P0  //数据口D0~D7
sbit LCD_RS = P2^6;//数据/命令选择 引脚
sbit LCD_RW = P2^5;//读/写选择 引脚
sbit LCD_E  = P2^7;//使能信号 引脚

/*****************LCD1602函数声明*******************/
void LCD_init(void);								   //初始化函数
void LCD_write_command(uchar command);				   //写指令函数
void LCD_write_data(uchar dat);						   //写数据函数
void lcd1602_write_character(uchar x,uchar y,uchar *s);//显示一个字符串,X(0-16),y(1-2)。
void delay_n40us(uint n);							   //延时函数

/*****************LCD1602自定义字符“℃”*******************/
uchar code pic[8]={0x00,0x0e,0x0a,0x0e,0x00,0x00,0x00,0x00}; //自定义“℃”字符
uchar code ASCII[] =  {'0','1','2','3','4','5','6','7','8','9'}; //LCD1602显示的字符数组

/********************************************************
函数名称:void LCD_init(void)
函数作用:LCD1602初始化函数
参数说明:
********************************************************/ 
void LCD_init(void) 
{ 
	LCD_write_command(0x38);//设置 8 位格式，2 行，5x7 
	LCD_write_command(0x38);//设置 8 位格式，2 行，5x7
	LCD_write_command(0x38);//设置 8 位格式，2 行，5x7
	LCD_write_command(0x0c);//整体显示，关光标，不闪烁
	LCD_write_command(0x06);//设定输入方式，增量不移位
	LCD_write_command(0x01);//清除屏幕显示
	delay_n40us(100);//实践证明，我的 LCD1602 上，用 for 循环 200 次就能可靠完成清屏指令。
}

/********************************************************
函数名称:void LCD_write_command(uchar dat)
函数作用:LCD1602写命令
参数说明:dat为指令，参考数据手册
********************************************************/
void LCD_write_command(uchar dat) 
{ 
	LCD_RS=0;//指令
	LCD_RW=0;//写入
	LCD_DB=dat; 
	delay_n40us(3);
	LCD_E=1;//允许
	delay_n40us(13);
	LCD_E=0; 
} 

/********************************************************
函数名称:void LCD_write_data(uchar dat)
函数作用:LCD1602写数据
参数说明:
********************************************************/
void LCD_write_data(uchar dat) 
{ 
	LCD_RS=1;//数据
	LCD_RW=0;//写入
	LCD_DB=dat;
	delay_n40us(3);
	LCD_E=1;//允许
	delay_n40us(13);
	LCD_E=0; 
} 
/********************************************************
函数名称:void LCD_disp_char(uchar x,uchar y,uchar dat)
函数作用:LCD1602显示一个字符
参数说明:在横坐标x（0~15），纵坐标y（1~2）显示一个字符dat
********************************************************/
void LCD_disp_char(uchar x,uchar y,uchar dat) 
{ 
	uchar add;
	//根据显示位置（x,y）确定显示地址 
	if(y==1) 			   //在第1行显示
		add=0x80+x; 
	else 				   //在第2行显示
		add=0xc0+x; 
	LCD_write_command(add);//写入需要显示的地址 
	LCD_write_data(dat);   //写入需要显示的内容
} 
/********************************************************
函数名称:lcd1602_write_character(uchar x,uchar y,uchar *s)
函数作用:LCD1602显示一个字符
参数说明:在横坐标x（0~7），纵坐标y（1~2）开始显示字符串*s
********************************************************/
void lcd1602_write_character(uchar x,uchar y,uchar *s) 
{ 
	uchar add; 
	//根据显示位置（x,y）确定显示地址 
	if(y==1) 
		add=0x80+x; 		 //在第1行显示
	else 
		add=0xc0+x;			 //在第2行显示
	LCD_write_command(add);  //写入需要显示的地址
	while (*s!='\0') 		 //还未显示结束
	{ 
		LCD_write_data(*s++);//写入需要显示的内容
	} 
}
/********************************************************
函数名称:void lcd1602_write_pic(uchar add,uchar *pic_num)
函数作用:LCD1602写入自定义字符
参数说明:在add地址写入自定义字符代码
********************************************************/
void lcd1602_write_pic(uchar add,uchar *pic_num) 
{ 
	unsigned char i; 
	add=add<<3; 
	for(i=0;i<8;i++) 
	{
		LCD_write_command(0x40|add+i); 
		LCD_write_data(*pic_num++);
	} 
} 
/********************************************************
函数名称:void delay_n40us(uint n)
函数作用:LCD1602延时函数
参数说明:
********************************************************/
void delay_n40us(uint n) 
{ 
	uint i; 
	uchar j;
	for(i=n;i>0;i--) 
	for(j=0;j<2;j++);   
}
#endif