/*************************************************************
                      

补充说明：
***************************************************************/
#include<reg52.h>	   //头文件
#include<LCD1602.h>
#include<DHT11.h>
#include<ESP8266.h>

#define WIFI_DATA_SEND(data) ESP8266_send_string(data "\0")

/*****************灯、蜂鸣器、按键引脚定义*******************/
sbit jdq1    = P3^4;  //继电器1
sbit jdq2    = P3^5;  //继电器2

sbit led1    = P3^6;  //绿色指示灯
sbit led2    = P3^7;  //红色指示灯

sbit buzz    = P2^0;  //蜂鸣器
sbit key_set = P3^3;  //设置键
sbit key_jia = P2^1;  //加键
sbit key_jian= P2^2;  //减键

/************************变量定义***********************/
uchar temp_up=40,temp_down=10;//存储温度上、下限值
uchar humi_up=70,humi_down=10;//存储湿度上、下限值
uchar temp_low,humi_low;//存储温湿度历史数据

uchar mq2,guang,KQH=50;


uchar set_f,sz=0; 		  	//设置选择标记，=0非设置，=1设置温度上限，=2设置温度下限
					    //              =3设置温度上限，=4设置湿度下限。
uchar T0_num=100;		//计数变量

int wifi_timer = 0;
char status_update_flag = 0;
uchar Temp[36]="ST+WD:--℃;sd:--%RH;kq:---%;nz:-\r\0";//存储待发送数据
uchar Temp1[36]="YZ+WD:--℃;sd:--%RH;kq:---%;\r\0";//存储待发送数据

/*
* base16 encoder and decoders
*/
int hexDigit(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  return 0;
}
#if 0
//for encode base16
void unused ASCIItoHex(const char *input, uchar inLen, char *output, uchar *outLen)
{
	int now = 0;
  int outputIndex = 0;

  while(now < inLen)
  {
    sprintf((char*)(output+outputIndex),"%02X", *input);
    input++;
    outputIndex+=2;
  }

  *outLen = outputIndex;
}
#endif

//for decode base16
// 30 = 3�16^1+0�16^0 = 48 = '0' character
void hexToASCII(const char *input, uchar inLen, char *output, uchar *outLen)
{
  int outputIndex = 0;
	unsigned int inputIndex;
  
  for (inputIndex = 0; inputIndex < inLen; inputIndex += 2){
    output[outputIndex] = (char)(hexDigit(input[inputIndex]) * 16 + hexDigit(input[inputIndex+1]));
    outputIndex++;
  }
  
   *outLen = outputIndex;
}

/********************************************************
函数名称:void delayms(uint ms)
函数作用:毫毛延时函数
参数说明:ms为延时的毫秒数
********************************************************/
void delayms(uint ms)
{
	unsigned char i=100,j;
	for(;ms;ms--)
	{
		while(--i)
		{
			j=10;
			while(--j);
		}
	}
}

/*****************模数转换器引脚定义*******************/
sbit AD0832_CS = P1^5 ; //片选使能，低电平有效
sbit AD0832_CLK = P1^6; //芯片时钟输入
sbit AD0832_DI = P1^7; //数据输入DI

uchar GetAD0832(uint Channel)
{
	uchar i=0 , Data1 =0 , Data2 = 0 ;
	AD0832_CLK = 0 ; //时钟置低平
	AD0832_DI = 1 ; //开始信号为高电平
	AD0832_CS = 0 ; //片选信号置低，启动AD转换芯片
	AD0832_CLK = 1 ; //产生一个正脉冲,在时钟上升沿，输入开始信号（DI=1）
	AD0832_CLK = 0 ; //第1个时钟下降沿
	//第1次下降沿之前DI高，启动信号
	AD0832_DI = 1 ; //输入DI=1，表示双通道单极性输入
	AD0832_CLK = 1 ; //产生一个正脉冲
	AD0832_CLK = 0 ; //第2个下降沿
	//在第2个下降沿，模拟信号输入模式选择（1：单模信号，0：双模差分信号）
	AD0832_DI = Channel ; //在第3个脉冲周期内设置DI，选择通道
	AD0832_CLK = 1 ; //产生一个正脉冲
	AD0832_CLK = 0 ; //第3个下降沿
	//在第3个下降沿，模拟信号输入通道选择（1：通道CH1，0：通道CH0）
	AD0832_DI = 1 ; //第4个下降沿之前，DI置高，准备接收数 
  for(i=0;i<8;i++) //第4~11共8个下降沿读数据（MSB->LSB）
  {
		AD0832_CLK = 1 ; 
		AD0832_CLK = 0 ; 
		Data1 = Data1<<1; 
		if(AD0832_DI == 1)
		{
			Data1 = Data1 | 0x01;
		} 
	} 

	for(i=0;i<8;i++) //第11~18共8个下降沿读数据（LSB->MSB）
  { 
		Data2 = Data2>>1;
		if(AD0832_DI == 1)
		{
			Data2 = Data2 | 0x80;} 
			AD0832_CLK = 1 ; 
			AD0832_CLK = 0 ; 
	  }
		AD0832_CLK = 1 ; 
		AD0832_DI = 1 ; 
		AD0832_CS = 1 ; 
		
		return (Data1==Data2)?Data1:0; //判断dat0与dat1是否相等
} 
////////////////////////////////
/********************************************************
函数名称:void display()
函数作用:正常显示
参数说明:
********************************************************/
void display()
{	
	//uchar Temp[22]="温度:--℃ 湿度:--%RH\r\0";//存储待发送数据
	lcd1602_write_character(0,1,"T:");  //0：表示第1列，1：表示第1行，所有关于显示都一样
	lcd1602_write_character(9,1,"H:");  //0：表示第1列，2：表示第2行，所有关于显示都一样
	lcd1602_write_character(13,1,"%RH");//12：表示第13列，2：表示第1行，所有关于显示都一样
	LCD_disp_char(4,1,0x00);			//显示自定义字符‘°’
	LCD_disp_char(5,1,'C');			  //显示‘C’
	
	if(DHT11_Check()==1)          //判断温湿度采集成功
	{
		//显示实测温度值
		LCD_disp_char(2,1,ASCII[Temp_H/10]);	   
		LCD_disp_char(3,1,ASCII[Temp_H%10]);
		//显示实测湿度值
		LCD_disp_char(11,1,ASCII[Humi_H/10]);	   
		LCD_disp_char(12,1,ASCII[Humi_H%10]);	
	
		
		if(temp_up<=Temp_H||temp_down>Temp_H||humi_up<=Humi_H||humi_down>Humi_H)//判断是否超出温湿度上下限
		{
			buzz=0;		 //超过限值蜂鸣器响起
			if(temp_up<=Temp_H||temp_down>Temp_H)//温度超出范围，开启红色指示灯
				led2=0;	 //点亮红色报警灯
			else
				led2=1;	 
			if(humi_up<=Humi_H||humi_down>Humi_H)//湿度超出范围，开启绿色指示灯
				led1=0;	 //点亮绿色报警灯
			else
				led1=1;
		}
		else		     //否者温湿度都处于限值范围内
		{
			buzz=1;		 //关闭蜂鸣器
			led1=1;		 //关闭绿色报警指示灯
			led2=1;		 //关闭红色报警指示灯
		}

		temp_low=Temp_H;		  //记录本次发送时的温湿度数值，
		humi_low=Humi_H;
		Temp[6]=ASCII[Temp_H/10]; //将温湿度数据放入待发送数组中
		Temp[7]=ASCII[Temp_H%10];			
		Temp[14]=ASCII[Humi_H/10];
		Temp[15]=ASCII[Humi_H%10];	
		Temp[23]=ASCII[guang/100]; //将温湿度数据放入待发送数组中
		Temp[24]=ASCII[guang%100/10];			
		Temp[25]=ASCII[guang%10];

		Temp[31]=jdq1==0?'1':'0'; //nozzle status
			
		//limit status
		Temp1[6]=ASCII[temp_up/10]; //将温湿度数据放入待发送数组中
		Temp1[7]=ASCII[temp_up%10];			
		Temp1[14]=ASCII[humi_down/10];
		Temp1[15]=ASCII[humi_down%10];
		Temp1[23]=ASCII[KQH/100]; //将温湿度数据放入待发送数组中
		Temp1[24]=ASCII[KQH%100/10];			
		Temp1[25]=ASCII[KQH%10];

		if (status_update_flag == 10 && !Hand("HQSJ"))
		{
			status_update_flag=0;
			ESP8266_send_string(Temp);//发送温湿度数据	
			ESP8266_send_string(Temp1);//发送温湿度数据	
			clearBuff();
		}
		
		if(temp_low!=Temp_H||humi_low!=Humi_H||Hand("HQSJ"))//数据更新，发送到手机上
		{
			/*
			temp_low=Temp_H;		  //记录本次发送时的温湿度数值，
			humi_low=Humi_H;
			Temp[6]=ASCII[Temp_H/10]; //将温湿度数据放入待发送数组中
			Temp[7]=ASCII[Temp_H%10];			
			Temp[14]=ASCII[Humi_H/10];
			Temp[15]=ASCII[Humi_H%10];
			Temp[23]=ASCII[guang/100]; //将温湿度数据放入待发送数组中
			Temp[24]=ASCII[guang%100/10];			
			Temp[25]=ASCII[guang%10];

			Temp[31]=jdq1==0?'1':'0'; //nozzle status
			
			//limit status
			Temp1[6]=ASCII[temp_up/10]; //将温湿度数据放入待发送数组中
			Temp1[7]=ASCII[temp_up%10];			
			Temp1[14]=ASCII[humi_down/10];
			Temp1[15]=ASCII[humi_down%10];
			Temp1[23]=ASCII[KQH/100]; //将温湿度数据放入待发送数组中
			Temp1[24]=ASCII[KQH%100/10];			
			Temp1[25]=ASCII[KQH%10];
			*/
			ESP8266_send_string(Temp);//发送温湿度数据	
			ESP8266_send_string(Temp1);//发送温湿度数据	
			clearBuff();
		}
	}
	else							  //否则，温湿度数据采集失败
	{
		lcd1602_write_character(2,1,"----");
		lcd1602_write_character(11,1,"--");
		if(temp_low!=0||Hand("HQSJ")) //数据更新，发送到手机上
		{
			temp_low=0;
			humi_low=0;	
			ESP8266_send_string(Temp);//发送温湿度数据
			ESP8266_send_string(Temp1);//发送温湿度数据
		}
		clearBuff();
	}
	
	if(ESP8266_f==1)//接收到WIFI控制指令
	{
		if(Hand("DKJDQ1"))	  //打开继电器1 控制
		{
			sz=1;//手动
			jdq1=0;
			WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
		}
		else if(Hand("GBJDQ1"))	  //关闭继电器1 控制
		{
			sz=1;//手动
			jdq1=1;
			WIFI_DATA_SEND("RT+OK:JDQ1-CLOSED");
		}
		else if(Hand("DKJDQ2"))	  //打开继电器2 控制
		{
			sz=0;//自动
			jdq2=0;
			WIFI_DATA_SEND("RT+OK:JDQ2-OPEN");
		}
		else if(Hand("GBJDQ2"))	  //关闭继电器2 控制
		{
			sz=0;//自动
			jdq2=1;	
			WIFI_DATA_SEND("RT+OK:JDQ2-CLOSED");
		}
	  else if(Hand("LT+"))	  //jia
		{
			uchar len = 0;
			char *p = ESP8266_data;
			char *end = ESP8266_data + MAXCHAR;
			while (p != end) {
				if(*p == 'L' && (p + 1) != end && *(p + 1) == 'T') break;
				p++;
			}
			hexToASCII(p+3, 2, &temp_up, &len);
			hexToASCII(p+5, 2, &KQH, &len);

			//limit status
			Temp1[6]=ASCII[temp_up/10]; //将温湿度数据放入待发送数组中
			Temp1[7]=ASCII[temp_up%10];			
			Temp1[23]=ASCII[KQH/100]; //将温湿度数据放入待发送数组中
			Temp1[24]=ASCII[KQH%100/10];			
			Temp1[25]=ASCII[KQH%10];
			ESP8266_send_string(Temp);//发送温湿度数据
			ESP8266_send_string(Temp1);//发送温湿度数据;
		}
		clearBuff();      //清除缓存
	}
	if(guang>=KQH&&sz==0) jdq1=0;
	if(guang<KQH&&sz==0) jdq1=1;

	if(jdq1==0)//根据继电器1的状态，显示相应内容
		lcd1602_write_character(0,2,"JDQ1ON ");
	else
		lcd1602_write_character(0,2,"JDQ1OFF");	
		
//	if(jdq2==0)//根据继电器2的状态，显示相应内容
//		lcd1602_write_character(9,2,"JDQ2ON ");
//	else
//		lcd1602_write_character(9,2,"JDQ2OFF");

	lcd1602_write_character(9,2,"KQ:");
	LCD_disp_char(12,2,ASCII[guang/100]);
	LCD_disp_char(13,2,ASCII[guang%100/10]);
	LCD_disp_char(14,2,ASCII[guang%10]);
}

void Timer0_Routine(void) interrupt 1
{
  TH0 = 64535/256;
  TL0 = 64535%256;
	delayms(10);
  wifi_timer++;
  if(wifi_timer % 1100 == 0)
  {
		status_update_flag++;
    wifi_timer = 0;		
  }
}
/********************************************************
函数名称:void display2(void)
函数作用:显示设置时温度上下限
参数说明:
********************************************************/
void display2(void)
{
	T0_num++;

	lcd1602_write_character(0,1,"Set Temp limit: ");
	lcd1602_write_character(1,2,"H:");
	LCD_disp_char(5,2,0x00);			//显示自定义字符‘°’
	LCD_disp_char(6,2,'C');			    //显示‘C’
	lcd1602_write_character(9,2,"L:");
	LCD_disp_char(13,2,0x00);			//显示自定义字符‘°’
	LCD_disp_char(14,2,'C');			//显示‘C’

	if(T0_num%2==0)	//偶数次显示，奇数次不显示。这样就会有闪烁效果，可以清楚看到当前设置的是哪个值
	{
		LCD_disp_char(3,2,ASCII[temp_up/10]);	//显示 温度上限
		LCD_disp_char(4,2,ASCII[temp_up%10]);
		LCD_disp_char(11,2,ASCII[temp_down/10]);//显示 温度下限
		LCD_disp_char(12,2,ASCII[temp_down%10]);
		Delay_ms(1);
	}
	else			 //奇数次不显示
	{
		switch(set_f)//根据当前设置的内容，对应位置闪烁
		{
			case 1:lcd1602_write_character(3,2,"  "); break;
			case 2:lcd1602_write_character(11,2,"  "); break;
			default: break;
		}
	}
}
/********************************************************
函数名称:void display3(void)
函数作用:显示设置时湿度上下限
参数说明:
********************************************************/
void display3(void)
{
	T0_num++;

	lcd1602_write_character(0,1,"Set Humi limit: ");
	lcd1602_write_character(0,2,"H:");
	lcd1602_write_character(4,2,"%RH");
	lcd1602_write_character(9,2,"L:");
	lcd1602_write_character(13,2,"%RH");

	if(T0_num%2==0)	//偶数次显示，奇数次不显示。这样就会有闪烁效果，可以清楚看到当前设置的是哪个值
	{
		LCD_disp_char(2,2,ASCII[humi_up/10]);	   //显示 湿度上限
		LCD_disp_char(3,2,ASCII[humi_up%10]);
		LCD_disp_char(11,2,ASCII[humi_down/10]);   //显示 湿度下限
		LCD_disp_char(12,2,ASCII[humi_down%10]);
		Delay_ms(1);
	}
	else			 //奇数次不显示
	{
		switch(set_f)//根据当前设置的内容，对应位置闪烁
		{
			case 3:lcd1602_write_character(2,2,"  "); break;
			case 4:lcd1602_write_character(11,2,"  "); break;
			default: break;
		}
	}
}

void display4(void)
{
	T0_num++;

//	lcd1602_write_character(0,1,"Set Humi limit: ");
	lcd1602_write_character(0,1,"Set KQ:         ");
	lcd1602_write_character(0,2,"S:");
	lcd1602_write_character(4,2,"%");
//	lcd1602_write_character(9,2,"L:");
//	lcd1602_write_character(13,2,"%RH");

	if(T0_num%2==0)	//偶数次显示，奇数次不显示。这样就会有闪烁效果，可以清楚看到当前设置的是哪个值
	{
		LCD_disp_char(2,2,ASCII[KQH/10]);	   //显示 湿度上限
		LCD_disp_char(3,2,ASCII[KQH%10]);
//		LCD_disp_char(11,2,ASCII[humi_down/10]);   //显示 湿度下限
//		LCD_disp_char(12,2,ASCII[humi_down%10]);
		Delay_ms(1);
	}
	else			 //奇数次不显示
	{
		switch(set_f)//根据当前设置的内容，对应位置闪烁
		{
			case 3:lcd1602_write_character(2,2,"  "); break;
			case 4:lcd1602_write_character(11,2,"  "); break;
			default: break;
		}
	}
}
/********************************************************
函数名称:void scan(void)
函数作用:按键检测函数
参数说明:
********************************************************/ 
void scan()
{
 	//设置键，不支持连按
	if(key_set==0)		//按键按下
	{
		delayms(7);		//延时消抖
		if(key_set==0)	//再次确认按键按下
		{
			led1=1;	    //关闭报警灯
			led2=1;
			buzz=1;	    //关闭蜂鸣器
			if(set_f%2==0)//进入设置先清除屏幕，显示设置部分
			{
				LCD_write_command(0x01);        //清除屏幕显示
				delay_n40us(100);		        //延时等待全部清除完毕
			}
			set_f++;	//设置变量+1
			if(set_f==8)//全部设置完成，退出设置
			{
				set_f=0;//重置设置变量

//				AT24C02_write_date(0,humi_up);  //写入【湿度上限】数据保存
//				AT24C02_write_date(1,humi_down);//写入【湿度下限】数据保存
//				AT24C02_write_date(2,temp_up);  //写入【温度上限】数据保存
//				AT24C02_write_date(3,temp_down);//写入【温度下限】数据保存
			}			
		}
		while(!key_set);//检测按键松开
	}

	//加值键，支持连按
	if(key_jia==0)
	{
		delayms(7);
		if(key_jia==0&&set_f!=0)
		{	
			if(set_f==1)        //设置 温度上限值
			{
				if(temp_up<99)  //最大值可设置99℃
					temp_up++;  //湿度上限+1℃
			}
	
			if(set_f==2)        //设置 温度下限值
			{
				if(temp_down<99&&temp_down+1<temp_up)//最大值可设置99℃，并且下限不可超过上限
					temp_down++;//温度下限+1℃
			}

			if(set_f==3)        //设置 湿度上限值
			{
				if(humi_up<99)  //最大值可设置99%RH
					humi_up++;  //湿度上限+1%RH
			}
	
			if(set_f==4)        //设置 湿度下限值
			{
				if(humi_down<99&&humi_down+1<humi_up)//最大值可设置99%RH，并且下限不可超过上限
					humi_down++;//湿度下限+1%RH
			}
			if(set_f==5)        //设置 湿度下限值
			{
//				if(KQH<99&&KQH+1<humi_up)//最大值可设置99%RH，并且下限不可超过上限
					KQH++;//湿度下限+1%RH
			}
		}
		else if(key_jia==0&&set_f==0)//手动控制继电器1开关
		{
			sz=1;//手动
			jdq1=~jdq1;		    //切换继电器1开/关
			if(jdq1==0)			//根据继电器1状态反馈
				WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
			else
				WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
			clearBuff();
		}
	}

	//减值键，支持连按
	if(key_jian==0)
	{
		delayms(7);
		if(key_jian==0&&set_f!=0)
		{
			if(set_f==1)        //设置温度上限值
			{
				if(temp_up!=0&&temp_up>temp_down+1)//最小值可设置0℃，并且上限得比下限大
					temp_up--;  //温度上限值-1℃
			}
	
			if(set_f==2)        //设置温度下限值
			{
				if(temp_down!=0)//最小值可设置0℃
					temp_down--;//温度下限值-1℃
			}

			if(set_f==3)        //设置湿度上限值
			{
				if(humi_up!=0&&humi_up>humi_down+1)//最小值可设置0%RH，并且上限得比下限大
					humi_up--;  //湿度上限值-1%RH
			}
					 
			if(set_f==4)        //设置湿度下限值
			{
				if(KQH!=0)//最小值可设置0%RH
					KQH--;//湿度下限值-1%RH
			}
			
					if(set_f==5)        //设置湿度下限值
			{
				if(KQH!=0)//最小值可设置0%RH
					KQH--;//湿度下限值-1%RH
			}
		}
		else if(key_jian==0&&set_f==0)//手动控制继电器2开关
		{
			sz=0;//自动
			jdq2=~jdq2;			 //手动控制继电器2开关
			if(jdq2==0)			 //根据继电器2状态反馈
				WIFI_DATA_SEND("RT+OK:JDQ2-OPEN");
			else
				WIFI_DATA_SEND("RT+OK:JDQ2-CLOSED");
			clearBuff();
		}	
	}
}
/********************************************************
函数名称:void main()
函数作用:主函数
参数说明:
********************************************************/
void main()
{	
	if(key_jian==0)
	{
		delayms(100);
		if(key_jian==0)
		{
			;
//			AT24C02_write_date(0,85);//写入【湿度上限值】数据保存
//			AT24C02_write_date(1,40);//写入【湿度下限值】数据保存
//			AT24C02_write_date(2,38);//写入【温度上限值】数据保存
//			AT24C02_write_date(3,16);//写入【温度下限值】数据保存
		}
	}

//	humi_up=AT24C02_read_date(0);  //读取存储的【湿度上限值】数据
//	humi_down=AT24C02_read_date(1);//读取存储的【湿度下限值】数据
//	temp_up=AT24C02_read_date(2);  //读取存储的【温度上限值】数据
//	temp_down=AT24C02_read_date(3);//读取存储的【温度下限值】数据
	LCD_init();				    //LCD1602初始化
	lcd1602_write_pic(0x00,pic);//将自定义字符“°”写入到LCD1602中
  ESP8266_init();			    //ESP8266初始化

	while(1)			        //死循环
	{
		scan();		          //按键检测处理
		if(set_f==0)			  //正常显示模式
		{
			T0_num++;			    //循环变量+1
			if(T0_num>=100)		//每循环100次，进入读取、显示数据
			{
				T0_num=0;		    //重置循环变量
				DHT11_read();   //读取DHT11温湿度
				mq2=GetAD0832(0);				//?áè??ì?í?μ
				guang= GetAD0832(1); //??DDA/D×a??
				display();	    //显示当前温、湿度
			}
		}
		else if(set_f==1||set_f==2)
		{
			display2();	    //显示设置温度上下限
		}
		else if(set_f==3||set_f==4)
		{
			display3();  	//显示设置湿度上下限
		}
		else if(set_f==5||set_f==6)
		{
			display4();
		}		
	}
}