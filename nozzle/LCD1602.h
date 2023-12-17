/*************************************************************
                      LCD1602ͷ�ļ�

ʵ�ֹ��ܣ�LCD1602�Ŀ���

����˵����
***************************************************************/
#ifndef _LCD1602_H_
#define _LCD1602_H_
#include<reg52.h>

#define uchar unsigned char
#define uint unsigned int

/*****************LCD1602���Ŷ���*******************/
#define LCD_DB P0  //���ݿ�D0~D7
sbit LCD_RS = P2^6;//����/����ѡ�� ����
sbit LCD_RW = P2^5;//��/дѡ�� ����
sbit LCD_E  = P2^7;//ʹ���ź� ����

/*****************LCD1602��������*******************/
void LCD_init(void);								   //��ʼ������
void LCD_write_command(uchar command);				   //дָ���
void LCD_write_data(uchar dat);						   //д���ݺ���
void lcd1602_write_character(uchar x,uchar y,uchar *s);//��ʾһ���ַ���,X(0-16),y(1-2)��
void delay_n40us(uint n);							   //��ʱ����

/*****************LCD1602�Զ����ַ����桱*******************/
uchar code pic[8]={0x00,0x0e,0x0a,0x0e,0x00,0x00,0x00,0x00}; //�Զ��塰�桱�ַ�
uchar code ASCII[] =  {'0','1','2','3','4','5','6','7','8','9'}; //LCD1602��ʾ���ַ�����

/********************************************************
��������:void LCD_init(void)
��������:LCD1602��ʼ������
����˵��:
********************************************************/ 
void LCD_init(void) 
{ 
	LCD_write_command(0x38);//���� 8 λ��ʽ��2 �У�5x7 
	LCD_write_command(0x38);//���� 8 λ��ʽ��2 �У�5x7
	LCD_write_command(0x38);//���� 8 λ��ʽ��2 �У�5x7
	LCD_write_command(0x0c);//������ʾ���ع�꣬����˸
	LCD_write_command(0x06);//�趨���뷽ʽ����������λ
	LCD_write_command(0x01);//�����Ļ��ʾ
	delay_n40us(100);//ʵ��֤�����ҵ� LCD1602 �ϣ��� for ѭ�� 200 �ξ��ܿɿ��������ָ�
}

/********************************************************
��������:void LCD_write_command(uchar dat)
��������:LCD1602д����
����˵��:datΪָ��ο������ֲ�
********************************************************/
void LCD_write_command(uchar dat) 
{ 
	LCD_RS=0;//ָ��
	LCD_RW=0;//д��
	LCD_DB=dat; 
	delay_n40us(3);
	LCD_E=1;//����
	delay_n40us(13);
	LCD_E=0; 
} 

/********************************************************
��������:void LCD_write_data(uchar dat)
��������:LCD1602д����
����˵��:
********************************************************/
void LCD_write_data(uchar dat) 
{ 
	LCD_RS=1;//����
	LCD_RW=0;//д��
	LCD_DB=dat;
	delay_n40us(3);
	LCD_E=1;//����
	delay_n40us(13);
	LCD_E=0; 
} 
/********************************************************
��������:void LCD_disp_char(uchar x,uchar y,uchar dat)
��������:LCD1602��ʾһ���ַ�
����˵��:�ں�����x��0~15����������y��1~2����ʾһ���ַ�dat
********************************************************/
void LCD_disp_char(uchar x,uchar y,uchar dat) 
{ 
	uchar add;
	//������ʾλ�ã�x,y��ȷ����ʾ��ַ 
	if(y==1) 			   //�ڵ�1����ʾ
		add=0x80+x; 
	else 				   //�ڵ�2����ʾ
		add=0xc0+x; 
	LCD_write_command(add);//д����Ҫ��ʾ�ĵ�ַ 
	LCD_write_data(dat);   //д����Ҫ��ʾ������
} 
/********************************************************
��������:lcd1602_write_character(uchar x,uchar y,uchar *s)
��������:LCD1602��ʾһ���ַ�
����˵��:�ں�����x��0~7����������y��1~2����ʼ��ʾ�ַ���*s
********************************************************/
void lcd1602_write_character(uchar x,uchar y,uchar *s) 
{ 
	uchar add; 
	//������ʾλ�ã�x,y��ȷ����ʾ��ַ 
	if(y==1) 
		add=0x80+x; 		 //�ڵ�1����ʾ
	else 
		add=0xc0+x;			 //�ڵ�2����ʾ
	LCD_write_command(add);  //д����Ҫ��ʾ�ĵ�ַ
	while (*s!='\0') 		 //��δ��ʾ����
	{ 
		LCD_write_data(*s++);//д����Ҫ��ʾ������
	} 
}
/********************************************************
��������:void lcd1602_write_pic(uchar add,uchar *pic_num)
��������:LCD1602д���Զ����ַ�
����˵��:��add��ַд���Զ����ַ�����
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
��������:void delay_n40us(uint n)
��������:LCD1602��ʱ����
����˵��:
********************************************************/
void delay_n40us(uint n) 
{ 
	uint i; 
	uchar j;
	for(i=n;i>0;i--) 
	for(j=0;j<2;j++);   
}
#endif