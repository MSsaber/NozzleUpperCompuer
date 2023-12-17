/*************************************************************
                      DHT11头文件
 
实现功能：DHT11的控制

补充说明：
***************************************************************/
#ifndef _DHT11_H_
#define _DHT11_H_
#include <reg52.h>

#define uchar unsigned char
#define uint unsigned int

/*****************DHT11引脚定义*******************/
sbit DHT11_DB=P3^2;

/*****************DHT11变量定义*********************/
uchar Temp_H,Temp_L,Humi_H,Humi_L,Check_data;//温度整数位，小数位，湿度整数位，小数位，数据校验位
uchar DHT11_num;			//计数变量
/*****************DHT11函数定义*********************/
void Delay_ms(uint n); 		//ms延时函数
void Delay_us(uchar n);		//us延时函数
uchar DHT11_receive(void);	//DHT11接收数据函数
void DHT11_read(void);		//DHT11读取温湿度函数
bit  DHT11_Check(void);     //DHT11校验函数，返回1：校验成功、返回0：校验失败

/********************************************************
函数名称:void Delay_ms(uint n)
函数作用:ms延时函数
参数说明:
********************************************************/
void Delay_ms(uint n)
{
	unsigned char j; 
	while(n--)
	{
		for(j=0;j<125;j++);
	}
}

/********************************************************
函数名称:void Delay_us(uchar n)
函数作用:us延时函数
参数说明:
********************************************************/
void Delay_us(uchar n)
{
	n=n/2;
	while(--n);
}
/********************************************************
函数名称:uchar DHT11_receive(void)
函数作用:接收DHT11传回来的数据
参数说明:返回接收到的八位数据
********************************************************/
uchar DHT11_receive(void)
{
	uchar i,Data; 
	for(i=0;i<8;i++) //接收8位数据
	{
		DHT11_num=2; //重置超时变量
		while(!DHT11_DB&&DHT11_num++);//等待DHT11变为高电平
		Delay_us(35);//延时35us		 
		Data<<=1; 	 //数据向左移动一位
		if(DHT11_DB) //如果DHT11为高电平，表示接收的数据为1
			Data|=1; //存入数据
		DHT11_num=2; //重置超时变量
		while(DHT11_DB&&DHT11_num++);//等待DHT11变为低电平
	}
	return Data;	 //返回接收到的数据
}
/********************************************************
函数名称:void DHT11_read(void)
函数作用:读取DHT11温湿度
参数说明:
********************************************************/
void DHT11_read(void)
{
	DHT11_DB=0;						  //拉低DHT11信号线
	Delay_ms(20);					  //延时20ms
	DHT11_DB=1;						  //拉高DHT11信号线
	Delay_us(60);					  //延时60us
	if(!DHT11_DB)					  //等待DHT11拉高响应
	{
		DHT11_num=2;				  //重置超时变量
		while(!DHT11_DB&&DHT11_num++);//等待DHT11变为高电平
		DHT11_num=2;				  //重置超时变量
		while(DHT11_DB&&DHT11_num++); //等待DHT11变为低电平
		Humi_H=DHT11_receive();		  //接收温度整数位
		Humi_L=DHT11_receive();		  //接收温度小数位
		Temp_H=DHT11_receive();		  //接收湿度整数位
		Temp_L=DHT11_receive();		  //接收湿度小数位
		Check_data=DHT11_receive();	  //接收数据校验位
		DHT11_DB=1;					  //拉高DHT11信号，结束本次数据读取
	}
}
/********************************************************
函数名称:bit DHT11_Check(void)
函数作用:DHT11校验函数
参数说明:返回=1：校验成功；返回=0：校验失败
********************************************************/
bit DHT11_Check(void)
{
	if((Temp_H+Temp_L+Humi_H+Humi_L)==Check_data)//判断校验和是否正确
		return 1;//返回=1：校验成功
	else
		return 0;//返回=0：校验失败
}
#endif