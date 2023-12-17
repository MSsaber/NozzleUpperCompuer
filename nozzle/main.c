/*************************************************************
                      

����˵����
***************************************************************/
#include<reg52.h>	   //ͷ�ļ�
#include<LCD1602.h>
#include<DHT11.h>
#include<ESP8266.h>

#define WIFI_DATA_SEND(data) ESP8266_send_string(data "\0")

/*****************�ơ����������������Ŷ���*******************/
sbit jdq1    = P3^4;  //�̵���1
sbit jdq2    = P3^5;  //�̵���2

sbit led1    = P3^6;  //��ɫָʾ��
sbit led2    = P3^7;  //��ɫָʾ��

sbit buzz    = P2^0;  //������
sbit key_set = P3^3;  //���ü�
sbit key_jia = P2^1;  //�Ӽ�
sbit key_jian= P2^2;  //����

/************************��������***********************/
uchar temp_up=40,temp_down=10;//�洢�¶��ϡ�����ֵ
uchar humi_up=70,humi_down=10;//�洢ʪ���ϡ�����ֵ
uchar temp_low,humi_low;//�洢��ʪ����ʷ����

uchar mq2,guang,KQH=50;


uchar set_f,sz=0; 		  	//����ѡ���ǣ�=0�����ã�=1�����¶����ޣ�=2�����¶�����
					    //              =3�����¶����ޣ�=4����ʪ�����ޡ�
uchar T0_num=100;		//��������

int wifi_timer = 0;
char status_update_flag = 0;
uchar Temp[36]="ST+WD:--��;sd:--%RH;kq:---%;nz:-\r\0";//�洢����������
uchar Temp1[36]="YZ+WD:--��;sd:--%RH;kq:---%;\r\0";//�洢����������

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
��������:void delayms(uint ms)
��������:��ë��ʱ����
����˵��:msΪ��ʱ�ĺ�����
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

/*****************ģ��ת�������Ŷ���*******************/
sbit AD0832_CS = P1^5 ; //Ƭѡʹ�ܣ��͵�ƽ��Ч
sbit AD0832_CLK = P1^6; //оƬʱ������
sbit AD0832_DI = P1^7; //��������DI

uchar GetAD0832(uint Channel)
{
	uchar i=0 , Data1 =0 , Data2 = 0 ;
	AD0832_CLK = 0 ; //ʱ���õ�ƽ
	AD0832_DI = 1 ; //��ʼ�ź�Ϊ�ߵ�ƽ
	AD0832_CS = 0 ; //Ƭѡ�ź��õͣ�����ADת��оƬ
	AD0832_CLK = 1 ; //����һ��������,��ʱ�������أ����뿪ʼ�źţ�DI=1��
	AD0832_CLK = 0 ; //��1��ʱ���½���
	//��1���½���֮ǰDI�ߣ������ź�
	AD0832_DI = 1 ; //����DI=1����ʾ˫ͨ������������
	AD0832_CLK = 1 ; //����һ��������
	AD0832_CLK = 0 ; //��2���½���
	//�ڵ�2���½��أ�ģ���ź�����ģʽѡ��1����ģ�źţ�0��˫ģ����źţ�
	AD0832_DI = Channel ; //�ڵ�3����������������DI��ѡ��ͨ��
	AD0832_CLK = 1 ; //����һ��������
	AD0832_CLK = 0 ; //��3���½���
	//�ڵ�3���½��أ�ģ���ź�����ͨ��ѡ��1��ͨ��CH1��0��ͨ��CH0��
	AD0832_DI = 1 ; //��4���½���֮ǰ��DI�øߣ�׼�������� 
  for(i=0;i<8;i++) //��4~11��8���½��ض����ݣ�MSB->LSB��
  {
		AD0832_CLK = 1 ; 
		AD0832_CLK = 0 ; 
		Data1 = Data1<<1; 
		if(AD0832_DI == 1)
		{
			Data1 = Data1 | 0x01;
		} 
	} 

	for(i=0;i<8;i++) //��11~18��8���½��ض����ݣ�LSB->MSB��
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
		
		return (Data1==Data2)?Data1:0; //�ж�dat0��dat1�Ƿ����
} 
////////////////////////////////
/********************************************************
��������:void display()
��������:������ʾ
����˵��:
********************************************************/
void display()
{	
	//uchar Temp[22]="�¶�:--�� ʪ��:--%RH\r\0";//�洢����������
	lcd1602_write_character(0,1,"T:");  //0����ʾ��1�У�1����ʾ��1�У����й�����ʾ��һ��
	lcd1602_write_character(9,1,"H:");  //0����ʾ��1�У�2����ʾ��2�У����й�����ʾ��һ��
	lcd1602_write_character(13,1,"%RH");//12����ʾ��13�У�2����ʾ��1�У����й�����ʾ��һ��
	LCD_disp_char(4,1,0x00);			//��ʾ�Զ����ַ����㡯
	LCD_disp_char(5,1,'C');			  //��ʾ��C��
	
	if(DHT11_Check()==1)          //�ж���ʪ�Ȳɼ��ɹ�
	{
		//��ʾʵ���¶�ֵ
		LCD_disp_char(2,1,ASCII[Temp_H/10]);	   
		LCD_disp_char(3,1,ASCII[Temp_H%10]);
		//��ʾʵ��ʪ��ֵ
		LCD_disp_char(11,1,ASCII[Humi_H/10]);	   
		LCD_disp_char(12,1,ASCII[Humi_H%10]);	
	
		
		if(temp_up<=Temp_H||temp_down>Temp_H||humi_up<=Humi_H||humi_down>Humi_H)//�ж��Ƿ񳬳���ʪ��������
		{
			buzz=0;		 //������ֵ����������
			if(temp_up<=Temp_H||temp_down>Temp_H)//�¶ȳ�����Χ��������ɫָʾ��
				led2=0;	 //������ɫ������
			else
				led2=1;	 
			if(humi_up<=Humi_H||humi_down>Humi_H)//ʪ�ȳ�����Χ��������ɫָʾ��
				led1=0;	 //������ɫ������
			else
				led1=1;
		}
		else		     //������ʪ�ȶ�������ֵ��Χ��
		{
			buzz=1;		 //�رշ�����
			led1=1;		 //�ر���ɫ����ָʾ��
			led2=1;		 //�رպ�ɫ����ָʾ��
		}

		temp_low=Temp_H;		  //��¼���η���ʱ����ʪ����ֵ��
		humi_low=Humi_H;
		Temp[6]=ASCII[Temp_H/10]; //����ʪ�����ݷ��������������
		Temp[7]=ASCII[Temp_H%10];			
		Temp[14]=ASCII[Humi_H/10];
		Temp[15]=ASCII[Humi_H%10];	
		Temp[23]=ASCII[guang/100]; //����ʪ�����ݷ��������������
		Temp[24]=ASCII[guang%100/10];			
		Temp[25]=ASCII[guang%10];

		Temp[31]=jdq1==0?'1':'0'; //nozzle status
			
		//limit status
		Temp1[6]=ASCII[temp_up/10]; //����ʪ�����ݷ��������������
		Temp1[7]=ASCII[temp_up%10];			
		Temp1[14]=ASCII[humi_down/10];
		Temp1[15]=ASCII[humi_down%10];
		Temp1[23]=ASCII[KQH/100]; //����ʪ�����ݷ��������������
		Temp1[24]=ASCII[KQH%100/10];			
		Temp1[25]=ASCII[KQH%10];

		if (status_update_flag == 10 && !Hand("HQSJ"))
		{
			status_update_flag=0;
			ESP8266_send_string(Temp);//������ʪ������	
			ESP8266_send_string(Temp1);//������ʪ������	
			clearBuff();
		}
		
		if(temp_low!=Temp_H||humi_low!=Humi_H||Hand("HQSJ"))//���ݸ��£����͵��ֻ���
		{
			/*
			temp_low=Temp_H;		  //��¼���η���ʱ����ʪ����ֵ��
			humi_low=Humi_H;
			Temp[6]=ASCII[Temp_H/10]; //����ʪ�����ݷ��������������
			Temp[7]=ASCII[Temp_H%10];			
			Temp[14]=ASCII[Humi_H/10];
			Temp[15]=ASCII[Humi_H%10];
			Temp[23]=ASCII[guang/100]; //����ʪ�����ݷ��������������
			Temp[24]=ASCII[guang%100/10];			
			Temp[25]=ASCII[guang%10];

			Temp[31]=jdq1==0?'1':'0'; //nozzle status
			
			//limit status
			Temp1[6]=ASCII[temp_up/10]; //����ʪ�����ݷ��������������
			Temp1[7]=ASCII[temp_up%10];			
			Temp1[14]=ASCII[humi_down/10];
			Temp1[15]=ASCII[humi_down%10];
			Temp1[23]=ASCII[KQH/100]; //����ʪ�����ݷ��������������
			Temp1[24]=ASCII[KQH%100/10];			
			Temp1[25]=ASCII[KQH%10];
			*/
			ESP8266_send_string(Temp);//������ʪ������	
			ESP8266_send_string(Temp1);//������ʪ������	
			clearBuff();
		}
	}
	else							  //������ʪ�����ݲɼ�ʧ��
	{
		lcd1602_write_character(2,1,"----");
		lcd1602_write_character(11,1,"--");
		if(temp_low!=0||Hand("HQSJ")) //���ݸ��£����͵��ֻ���
		{
			temp_low=0;
			humi_low=0;	
			ESP8266_send_string(Temp);//������ʪ������
			ESP8266_send_string(Temp1);//������ʪ������
		}
		clearBuff();
	}
	
	if(ESP8266_f==1)//���յ�WIFI����ָ��
	{
		if(Hand("DKJDQ1"))	  //�򿪼̵���1 ����
		{
			sz=1;//�ֶ�
			jdq1=0;
			WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
		}
		else if(Hand("GBJDQ1"))	  //�رռ̵���1 ����
		{
			sz=1;//�ֶ�
			jdq1=1;
			WIFI_DATA_SEND("RT+OK:JDQ1-CLOSED");
		}
		else if(Hand("DKJDQ2"))	  //�򿪼̵���2 ����
		{
			sz=0;//�Զ�
			jdq2=0;
			WIFI_DATA_SEND("RT+OK:JDQ2-OPEN");
		}
		else if(Hand("GBJDQ2"))	  //�رռ̵���2 ����
		{
			sz=0;//�Զ�
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
			Temp1[6]=ASCII[temp_up/10]; //����ʪ�����ݷ��������������
			Temp1[7]=ASCII[temp_up%10];			
			Temp1[23]=ASCII[KQH/100]; //����ʪ�����ݷ��������������
			Temp1[24]=ASCII[KQH%100/10];			
			Temp1[25]=ASCII[KQH%10];
			ESP8266_send_string(Temp);//������ʪ������
			ESP8266_send_string(Temp1);//������ʪ������;
		}
		clearBuff();      //�������
	}
	if(guang>=KQH&&sz==0) jdq1=0;
	if(guang<KQH&&sz==0) jdq1=1;

	if(jdq1==0)//���ݼ̵���1��״̬����ʾ��Ӧ����
		lcd1602_write_character(0,2,"JDQ1ON ");
	else
		lcd1602_write_character(0,2,"JDQ1OFF");	
		
//	if(jdq2==0)//���ݼ̵���2��״̬����ʾ��Ӧ����
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
��������:void display2(void)
��������:��ʾ����ʱ�¶�������
����˵��:
********************************************************/
void display2(void)
{
	T0_num++;

	lcd1602_write_character(0,1,"Set Temp limit: ");
	lcd1602_write_character(1,2,"H:");
	LCD_disp_char(5,2,0x00);			//��ʾ�Զ����ַ����㡯
	LCD_disp_char(6,2,'C');			    //��ʾ��C��
	lcd1602_write_character(9,2,"L:");
	LCD_disp_char(13,2,0x00);			//��ʾ�Զ����ַ����㡯
	LCD_disp_char(14,2,'C');			//��ʾ��C��

	if(T0_num%2==0)	//ż������ʾ�������β���ʾ�������ͻ�����˸Ч�����������������ǰ���õ����ĸ�ֵ
	{
		LCD_disp_char(3,2,ASCII[temp_up/10]);	//��ʾ �¶�����
		LCD_disp_char(4,2,ASCII[temp_up%10]);
		LCD_disp_char(11,2,ASCII[temp_down/10]);//��ʾ �¶�����
		LCD_disp_char(12,2,ASCII[temp_down%10]);
		Delay_ms(1);
	}
	else			 //�����β���ʾ
	{
		switch(set_f)//���ݵ�ǰ���õ����ݣ���Ӧλ����˸
		{
			case 1:lcd1602_write_character(3,2,"  "); break;
			case 2:lcd1602_write_character(11,2,"  "); break;
			default: break;
		}
	}
}
/********************************************************
��������:void display3(void)
��������:��ʾ����ʱʪ��������
����˵��:
********************************************************/
void display3(void)
{
	T0_num++;

	lcd1602_write_character(0,1,"Set Humi limit: ");
	lcd1602_write_character(0,2,"H:");
	lcd1602_write_character(4,2,"%RH");
	lcd1602_write_character(9,2,"L:");
	lcd1602_write_character(13,2,"%RH");

	if(T0_num%2==0)	//ż������ʾ�������β���ʾ�������ͻ�����˸Ч�����������������ǰ���õ����ĸ�ֵ
	{
		LCD_disp_char(2,2,ASCII[humi_up/10]);	   //��ʾ ʪ������
		LCD_disp_char(3,2,ASCII[humi_up%10]);
		LCD_disp_char(11,2,ASCII[humi_down/10]);   //��ʾ ʪ������
		LCD_disp_char(12,2,ASCII[humi_down%10]);
		Delay_ms(1);
	}
	else			 //�����β���ʾ
	{
		switch(set_f)//���ݵ�ǰ���õ����ݣ���Ӧλ����˸
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

	if(T0_num%2==0)	//ż������ʾ�������β���ʾ�������ͻ�����˸Ч�����������������ǰ���õ����ĸ�ֵ
	{
		LCD_disp_char(2,2,ASCII[KQH/10]);	   //��ʾ ʪ������
		LCD_disp_char(3,2,ASCII[KQH%10]);
//		LCD_disp_char(11,2,ASCII[humi_down/10]);   //��ʾ ʪ������
//		LCD_disp_char(12,2,ASCII[humi_down%10]);
		Delay_ms(1);
	}
	else			 //�����β���ʾ
	{
		switch(set_f)//���ݵ�ǰ���õ����ݣ���Ӧλ����˸
		{
			case 3:lcd1602_write_character(2,2,"  "); break;
			case 4:lcd1602_write_character(11,2,"  "); break;
			default: break;
		}
	}
}
/********************************************************
��������:void scan(void)
��������:������⺯��
����˵��:
********************************************************/ 
void scan()
{
 	//���ü�����֧������
	if(key_set==0)		//��������
	{
		delayms(7);		//��ʱ����
		if(key_set==0)	//�ٴ�ȷ�ϰ�������
		{
			led1=1;	    //�رձ�����
			led2=1;
			buzz=1;	    //�رշ�����
			if(set_f%2==0)//���������������Ļ����ʾ���ò���
			{
				LCD_write_command(0x01);        //�����Ļ��ʾ
				delay_n40us(100);		        //��ʱ�ȴ�ȫ��������
			}
			set_f++;	//���ñ���+1
			if(set_f==8)//ȫ��������ɣ��˳�����
			{
				set_f=0;//�������ñ���

//				AT24C02_write_date(0,humi_up);  //д�롾ʪ�����ޡ����ݱ���
//				AT24C02_write_date(1,humi_down);//д�롾ʪ�����ޡ����ݱ���
//				AT24C02_write_date(2,temp_up);  //д�롾�¶����ޡ����ݱ���
//				AT24C02_write_date(3,temp_down);//д�롾�¶����ޡ����ݱ���
			}			
		}
		while(!key_set);//��ⰴ���ɿ�
	}

	//��ֵ����֧������
	if(key_jia==0)
	{
		delayms(7);
		if(key_jia==0&&set_f!=0)
		{	
			if(set_f==1)        //���� �¶�����ֵ
			{
				if(temp_up<99)  //���ֵ������99��
					temp_up++;  //ʪ������+1��
			}
	
			if(set_f==2)        //���� �¶�����ֵ
			{
				if(temp_down<99&&temp_down+1<temp_up)//���ֵ������99�棬�������޲��ɳ�������
					temp_down++;//�¶�����+1��
			}

			if(set_f==3)        //���� ʪ������ֵ
			{
				if(humi_up<99)  //���ֵ������99%RH
					humi_up++;  //ʪ������+1%RH
			}
	
			if(set_f==4)        //���� ʪ������ֵ
			{
				if(humi_down<99&&humi_down+1<humi_up)//���ֵ������99%RH���������޲��ɳ�������
					humi_down++;//ʪ������+1%RH
			}
			if(set_f==5)        //���� ʪ������ֵ
			{
//				if(KQH<99&&KQH+1<humi_up)//���ֵ������99%RH���������޲��ɳ�������
					KQH++;//ʪ������+1%RH
			}
		}
		else if(key_jia==0&&set_f==0)//�ֶ����Ƽ̵���1����
		{
			sz=1;//�ֶ�
			jdq1=~jdq1;		    //�л��̵���1��/��
			if(jdq1==0)			//���ݼ̵���1״̬����
				WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
			else
				WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
			clearBuff();
		}
	}

	//��ֵ����֧������
	if(key_jian==0)
	{
		delayms(7);
		if(key_jian==0&&set_f!=0)
		{
			if(set_f==1)        //�����¶�����ֵ
			{
				if(temp_up!=0&&temp_up>temp_down+1)//��Сֵ������0�棬�������޵ñ����޴�
					temp_up--;  //�¶�����ֵ-1��
			}
	
			if(set_f==2)        //�����¶�����ֵ
			{
				if(temp_down!=0)//��Сֵ������0��
					temp_down--;//�¶�����ֵ-1��
			}

			if(set_f==3)        //����ʪ������ֵ
			{
				if(humi_up!=0&&humi_up>humi_down+1)//��Сֵ������0%RH���������޵ñ����޴�
					humi_up--;  //ʪ������ֵ-1%RH
			}
					 
			if(set_f==4)        //����ʪ������ֵ
			{
				if(KQH!=0)//��Сֵ������0%RH
					KQH--;//ʪ������ֵ-1%RH
			}
			
					if(set_f==5)        //����ʪ������ֵ
			{
				if(KQH!=0)//��Сֵ������0%RH
					KQH--;//ʪ������ֵ-1%RH
			}
		}
		else if(key_jian==0&&set_f==0)//�ֶ����Ƽ̵���2����
		{
			sz=0;//�Զ�
			jdq2=~jdq2;			 //�ֶ����Ƽ̵���2����
			if(jdq2==0)			 //���ݼ̵���2״̬����
				WIFI_DATA_SEND("RT+OK:JDQ2-OPEN");
			else
				WIFI_DATA_SEND("RT+OK:JDQ2-CLOSED");
			clearBuff();
		}	
	}
}
/********************************************************
��������:void main()
��������:������
����˵��:
********************************************************/
void main()
{	
	if(key_jian==0)
	{
		delayms(100);
		if(key_jian==0)
		{
			;
//			AT24C02_write_date(0,85);//д�롾ʪ������ֵ�����ݱ���
//			AT24C02_write_date(1,40);//д�롾ʪ������ֵ�����ݱ���
//			AT24C02_write_date(2,38);//д�롾�¶�����ֵ�����ݱ���
//			AT24C02_write_date(3,16);//д�롾�¶�����ֵ�����ݱ���
		}
	}

//	humi_up=AT24C02_read_date(0);  //��ȡ�洢�ġ�ʪ������ֵ������
//	humi_down=AT24C02_read_date(1);//��ȡ�洢�ġ�ʪ������ֵ������
//	temp_up=AT24C02_read_date(2);  //��ȡ�洢�ġ��¶�����ֵ������
//	temp_down=AT24C02_read_date(3);//��ȡ�洢�ġ��¶�����ֵ������
	LCD_init();				    //LCD1602��ʼ��
	lcd1602_write_pic(0x00,pic);//���Զ����ַ����㡱д�뵽LCD1602��
  ESP8266_init();			    //ESP8266��ʼ��

	while(1)			        //��ѭ��
	{
		scan();		          //������⴦��
		if(set_f==0)			  //������ʾģʽ
		{
			T0_num++;			    //ѭ������+1
			if(T0_num>=100)		//ÿѭ��100�Σ������ȡ����ʾ����
			{
				T0_num=0;		    //����ѭ������
				DHT11_read();   //��ȡDHT11��ʪ��
				mq2=GetAD0832(0);				//?����??��?��?��
				guang= GetAD0832(1); //??DDA/D��a??
				display();	    //��ʾ��ǰ�¡�ʪ��
			}
		}
		else if(set_f==1||set_f==2)
		{
			display2();	    //��ʾ�����¶�������
		}
		else if(set_f==3||set_f==4)
		{
			display3();  	//��ʾ����ʪ��������
		}
		else if(set_f==5||set_f==6)
		{
			display4();
		}		
	}
}