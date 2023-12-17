/*************************************************************
                      

²¹³äËµÃ÷£º
***************************************************************/
#include<reg52.h>	   //Í·ÎÄ¼ş
#include<LCD1602.h>
#include<DHT11.h>
#include<ESP8266.h>

#define WIFI_DATA_SEND(data) ESP8266_send_string(data "\0")

/*****************µÆ¡¢·äÃùÆ÷¡¢°´¼üÒı½Å¶¨Òå*******************/
sbit jdq1    = P3^4;  //¼ÌµçÆ÷1
sbit jdq2    = P3^5;  //¼ÌµçÆ÷2

sbit led1    = P3^6;  //ÂÌÉ«Ö¸Ê¾µÆ
sbit led2    = P3^7;  //ºìÉ«Ö¸Ê¾µÆ

sbit buzz    = P2^0;  //·äÃùÆ÷
sbit key_set = P3^3;  //ÉèÖÃ¼ü
sbit key_jia = P2^1;  //¼Ó¼ü
sbit key_jian= P2^2;  //¼õ¼ü

/************************±äÁ¿¶¨Òå***********************/
uchar temp_up=40,temp_down=10;//´æ´¢ÎÂ¶ÈÉÏ¡¢ÏÂÏŞÖµ
uchar humi_up=70,humi_down=10;//´æ´¢Êª¶ÈÉÏ¡¢ÏÂÏŞÖµ
uchar temp_low,humi_low;//´æ´¢ÎÂÊª¶ÈÀúÊ·Êı¾İ

uchar mq2,guang,KQH=50;


uchar set_f,sz=0; 		  	//ÉèÖÃÑ¡Ôñ±ê¼Ç£¬=0·ÇÉèÖÃ£¬=1ÉèÖÃÎÂ¶ÈÉÏÏŞ£¬=2ÉèÖÃÎÂ¶ÈÏÂÏŞ
					    //              =3ÉèÖÃÎÂ¶ÈÉÏÏŞ£¬=4ÉèÖÃÊª¶ÈÏÂÏŞ¡£
uchar T0_num=100;		//¼ÆÊı±äÁ¿

int wifi_timer = 0;
char status_update_flag = 0;
uchar Temp[36]="ST+WD:--¡æ;sd:--%RH;kq:---%;nz:-\r\0";//´æ´¢´ı·¢ËÍÊı¾İ
uchar Temp1[36]="YZ+WD:--¡æ;sd:--%RH;kq:---%;\r\0";//´æ´¢´ı·¢ËÍÊı¾İ

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
// 30 = 3×16^1+0×16^0 = 48 = '0' character
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
º¯ÊıÃû³Æ:void delayms(uint ms)
º¯Êı×÷ÓÃ:ºÁÃ«ÑÓÊ±º¯Êı
²ÎÊıËµÃ÷:msÎªÑÓÊ±µÄºÁÃëÊı
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

/*****************Ä£Êı×ª»»Æ÷Òı½Å¶¨Òå*******************/
sbit AD0832_CS = P1^5 ; //Æ¬Ñ¡Ê¹ÄÜ£¬µÍµçÆ½ÓĞĞ§
sbit AD0832_CLK = P1^6; //Ğ¾Æ¬Ê±ÖÓÊäÈë
sbit AD0832_DI = P1^7; //Êı¾İÊäÈëDI

uchar GetAD0832(uint Channel)
{
	uchar i=0 , Data1 =0 , Data2 = 0 ;
	AD0832_CLK = 0 ; //Ê±ÖÓÖÃµÍÆ½
	AD0832_DI = 1 ; //¿ªÊ¼ĞÅºÅÎª¸ßµçÆ½
	AD0832_CS = 0 ; //Æ¬Ñ¡ĞÅºÅÖÃµÍ£¬Æô¶¯AD×ª»»Ğ¾Æ¬
	AD0832_CLK = 1 ; //²úÉúÒ»¸öÕıÂö³å,ÔÚÊ±ÖÓÉÏÉıÑØ£¬ÊäÈë¿ªÊ¼ĞÅºÅ£¨DI=1£©
	AD0832_CLK = 0 ; //µÚ1¸öÊ±ÖÓÏÂ½µÑØ
	//µÚ1´ÎÏÂ½µÑØÖ®Ç°DI¸ß£¬Æô¶¯ĞÅºÅ
	AD0832_DI = 1 ; //ÊäÈëDI=1£¬±íÊ¾Ë«Í¨µÀµ¥¼«ĞÔÊäÈë
	AD0832_CLK = 1 ; //²úÉúÒ»¸öÕıÂö³å
	AD0832_CLK = 0 ; //µÚ2¸öÏÂ½µÑØ
	//ÔÚµÚ2¸öÏÂ½µÑØ£¬Ä£ÄâĞÅºÅÊäÈëÄ£Ê½Ñ¡Ôñ£¨1£ºµ¥Ä£ĞÅºÅ£¬0£ºË«Ä£²î·ÖĞÅºÅ£©
	AD0832_DI = Channel ; //ÔÚµÚ3¸öÂö³åÖÜÆÚÄÚÉèÖÃDI£¬Ñ¡ÔñÍ¨µÀ
	AD0832_CLK = 1 ; //²úÉúÒ»¸öÕıÂö³å
	AD0832_CLK = 0 ; //µÚ3¸öÏÂ½µÑØ
	//ÔÚµÚ3¸öÏÂ½µÑØ£¬Ä£ÄâĞÅºÅÊäÈëÍ¨µÀÑ¡Ôñ£¨1£ºÍ¨µÀCH1£¬0£ºÍ¨µÀCH0£©
	AD0832_DI = 1 ; //µÚ4¸öÏÂ½µÑØÖ®Ç°£¬DIÖÃ¸ß£¬×¼±¸½ÓÊÕÊı 
  for(i=0;i<8;i++) //µÚ4~11¹²8¸öÏÂ½µÑØ¶ÁÊı¾İ£¨MSB->LSB£©
  {
		AD0832_CLK = 1 ; 
		AD0832_CLK = 0 ; 
		Data1 = Data1<<1; 
		if(AD0832_DI == 1)
		{
			Data1 = Data1 | 0x01;
		} 
	} 

	for(i=0;i<8;i++) //µÚ11~18¹²8¸öÏÂ½µÑØ¶ÁÊı¾İ£¨LSB->MSB£©
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
		
		return (Data1==Data2)?Data1:0; //ÅĞ¶Ïdat0Óëdat1ÊÇ·ñÏàµÈ
} 
////////////////////////////////
/********************************************************
º¯ÊıÃû³Æ:void display()
º¯Êı×÷ÓÃ:Õı³£ÏÔÊ¾
²ÎÊıËµÃ÷:
********************************************************/
void display()
{	
	//uchar Temp[22]="ÎÂ¶È:--¡æ Êª¶È:--%RH\r\0";//´æ´¢´ı·¢ËÍÊı¾İ
	lcd1602_write_character(0,1,"T:");  //0£º±íÊ¾µÚ1ÁĞ£¬1£º±íÊ¾µÚ1ĞĞ£¬ËùÓĞ¹ØÓÚÏÔÊ¾¶¼Ò»Ñù
	lcd1602_write_character(9,1,"H:");  //0£º±íÊ¾µÚ1ÁĞ£¬2£º±íÊ¾µÚ2ĞĞ£¬ËùÓĞ¹ØÓÚÏÔÊ¾¶¼Ò»Ñù
	lcd1602_write_character(13,1,"%RH");//12£º±íÊ¾µÚ13ÁĞ£¬2£º±íÊ¾µÚ1ĞĞ£¬ËùÓĞ¹ØÓÚÏÔÊ¾¶¼Ò»Ñù
	LCD_disp_char(4,1,0x00);			//ÏÔÊ¾×Ô¶¨Òå×Ö·û¡®¡ã¡¯
	LCD_disp_char(5,1,'C');			  //ÏÔÊ¾¡®C¡¯
	
	if(DHT11_Check()==1)          //ÅĞ¶ÏÎÂÊª¶È²É¼¯³É¹¦
	{
		//ÏÔÊ¾Êµ²âÎÂ¶ÈÖµ
		LCD_disp_char(2,1,ASCII[Temp_H/10]);	   
		LCD_disp_char(3,1,ASCII[Temp_H%10]);
		//ÏÔÊ¾Êµ²âÊª¶ÈÖµ
		LCD_disp_char(11,1,ASCII[Humi_H/10]);	   
		LCD_disp_char(12,1,ASCII[Humi_H%10]);	
	
		
		if(temp_up<=Temp_H||temp_down>Temp_H||humi_up<=Humi_H||humi_down>Humi_H)//ÅĞ¶ÏÊÇ·ñ³¬³öÎÂÊª¶ÈÉÏÏÂÏŞ
		{
			buzz=0;		 //³¬¹ıÏŞÖµ·äÃùÆ÷ÏìÆğ
			if(temp_up<=Temp_H||temp_down>Temp_H)//ÎÂ¶È³¬³ö·¶Î§£¬¿ªÆôºìÉ«Ö¸Ê¾µÆ
				led2=0;	 //µãÁÁºìÉ«±¨¾¯µÆ
			else
				led2=1;	 
			if(humi_up<=Humi_H||humi_down>Humi_H)//Êª¶È³¬³ö·¶Î§£¬¿ªÆôÂÌÉ«Ö¸Ê¾µÆ
				led1=0;	 //µãÁÁÂÌÉ«±¨¾¯µÆ
			else
				led1=1;
		}
		else		     //·ñÕßÎÂÊª¶È¶¼´¦ÓÚÏŞÖµ·¶Î§ÄÚ
		{
			buzz=1;		 //¹Ø±Õ·äÃùÆ÷
			led1=1;		 //¹Ø±ÕÂÌÉ«±¨¾¯Ö¸Ê¾µÆ
			led2=1;		 //¹Ø±ÕºìÉ«±¨¾¯Ö¸Ê¾µÆ
		}

		temp_low=Temp_H;		  //¼ÇÂ¼±¾´Î·¢ËÍÊ±µÄÎÂÊª¶ÈÊıÖµ£¬
		humi_low=Humi_H;
		Temp[6]=ASCII[Temp_H/10]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
		Temp[7]=ASCII[Temp_H%10];			
		Temp[14]=ASCII[Humi_H/10];
		Temp[15]=ASCII[Humi_H%10];	
		Temp[23]=ASCII[guang/100]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
		Temp[24]=ASCII[guang%100/10];			
		Temp[25]=ASCII[guang%10];

		Temp[31]=jdq1==0?'1':'0'; //nozzle status
			
		//limit status
		Temp1[6]=ASCII[temp_up/10]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
		Temp1[7]=ASCII[temp_up%10];			
		Temp1[14]=ASCII[humi_down/10];
		Temp1[15]=ASCII[humi_down%10];
		Temp1[23]=ASCII[KQH/100]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
		Temp1[24]=ASCII[KQH%100/10];			
		Temp1[25]=ASCII[KQH%10];

		if (status_update_flag == 10 && !Hand("HQSJ"))
		{
			status_update_flag=0;
			ESP8266_send_string(Temp);//·¢ËÍÎÂÊª¶ÈÊı¾İ	
			ESP8266_send_string(Temp1);//·¢ËÍÎÂÊª¶ÈÊı¾İ	
			clearBuff();
		}
		
		if(temp_low!=Temp_H||humi_low!=Humi_H||Hand("HQSJ"))//Êı¾İ¸üĞÂ£¬·¢ËÍµ½ÊÖ»úÉÏ
		{
			/*
			temp_low=Temp_H;		  //¼ÇÂ¼±¾´Î·¢ËÍÊ±µÄÎÂÊª¶ÈÊıÖµ£¬
			humi_low=Humi_H;
			Temp[6]=ASCII[Temp_H/10]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
			Temp[7]=ASCII[Temp_H%10];			
			Temp[14]=ASCII[Humi_H/10];
			Temp[15]=ASCII[Humi_H%10];
			Temp[23]=ASCII[guang/100]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
			Temp[24]=ASCII[guang%100/10];			
			Temp[25]=ASCII[guang%10];

			Temp[31]=jdq1==0?'1':'0'; //nozzle status
			
			//limit status
			Temp1[6]=ASCII[temp_up/10]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
			Temp1[7]=ASCII[temp_up%10];			
			Temp1[14]=ASCII[humi_down/10];
			Temp1[15]=ASCII[humi_down%10];
			Temp1[23]=ASCII[KQH/100]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
			Temp1[24]=ASCII[KQH%100/10];			
			Temp1[25]=ASCII[KQH%10];
			*/
			ESP8266_send_string(Temp);//·¢ËÍÎÂÊª¶ÈÊı¾İ	
			ESP8266_send_string(Temp1);//·¢ËÍÎÂÊª¶ÈÊı¾İ	
			clearBuff();
		}
	}
	else							  //·ñÔò£¬ÎÂÊª¶ÈÊı¾İ²É¼¯Ê§°Ü
	{
		lcd1602_write_character(2,1,"----");
		lcd1602_write_character(11,1,"--");
		if(temp_low!=0||Hand("HQSJ")) //Êı¾İ¸üĞÂ£¬·¢ËÍµ½ÊÖ»úÉÏ
		{
			temp_low=0;
			humi_low=0;	
			ESP8266_send_string(Temp);//·¢ËÍÎÂÊª¶ÈÊı¾İ
			ESP8266_send_string(Temp1);//·¢ËÍÎÂÊª¶ÈÊı¾İ
		}
		clearBuff();
	}
	
	if(ESP8266_f==1)//½ÓÊÕµ½WIFI¿ØÖÆÖ¸Áî
	{
		if(Hand("DKJDQ1"))	  //´ò¿ª¼ÌµçÆ÷1 ¿ØÖÆ
		{
			sz=1;//ÊÖ¶¯
			jdq1=0;
			WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
		}
		else if(Hand("GBJDQ1"))	  //¹Ø±Õ¼ÌµçÆ÷1 ¿ØÖÆ
		{
			sz=1;//ÊÖ¶¯
			jdq1=1;
			WIFI_DATA_SEND("RT+OK:JDQ1-CLOSED");
		}
		else if(Hand("DKJDQ2"))	  //´ò¿ª¼ÌµçÆ÷2 ¿ØÖÆ
		{
			sz=0;//×Ô¶¯
			jdq2=0;
			WIFI_DATA_SEND("RT+OK:JDQ2-OPEN");
		}
		else if(Hand("GBJDQ2"))	  //¹Ø±Õ¼ÌµçÆ÷2 ¿ØÖÆ
		{
			sz=0;//×Ô¶¯
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
			Temp1[6]=ASCII[temp_up/10]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
			Temp1[7]=ASCII[temp_up%10];			
			Temp1[23]=ASCII[KQH/100]; //½«ÎÂÊª¶ÈÊı¾İ·ÅÈë´ı·¢ËÍÊı×éÖĞ
			Temp1[24]=ASCII[KQH%100/10];			
			Temp1[25]=ASCII[KQH%10];
			ESP8266_send_string(Temp);//·¢ËÍÎÂÊª¶ÈÊı¾İ
			ESP8266_send_string(Temp1);//·¢ËÍÎÂÊª¶ÈÊı¾İ;
		}
		clearBuff();      //Çå³ı»º´æ
	}
	if(guang>=KQH&&sz==0) jdq1=0;
	if(guang<KQH&&sz==0) jdq1=1;

	if(jdq1==0)//¸ù¾İ¼ÌµçÆ÷1µÄ×´Ì¬£¬ÏÔÊ¾ÏàÓ¦ÄÚÈİ
		lcd1602_write_character(0,2,"JDQ1ON ");
	else
		lcd1602_write_character(0,2,"JDQ1OFF");	
		
//	if(jdq2==0)//¸ù¾İ¼ÌµçÆ÷2µÄ×´Ì¬£¬ÏÔÊ¾ÏàÓ¦ÄÚÈİ
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
º¯ÊıÃû³Æ:void display2(void)
º¯Êı×÷ÓÃ:ÏÔÊ¾ÉèÖÃÊ±ÎÂ¶ÈÉÏÏÂÏŞ
²ÎÊıËµÃ÷:
********************************************************/
void display2(void)
{
	T0_num++;

	lcd1602_write_character(0,1,"Set Temp limit: ");
	lcd1602_write_character(1,2,"H:");
	LCD_disp_char(5,2,0x00);			//ÏÔÊ¾×Ô¶¨Òå×Ö·û¡®¡ã¡¯
	LCD_disp_char(6,2,'C');			    //ÏÔÊ¾¡®C¡¯
	lcd1602_write_character(9,2,"L:");
	LCD_disp_char(13,2,0x00);			//ÏÔÊ¾×Ô¶¨Òå×Ö·û¡®¡ã¡¯
	LCD_disp_char(14,2,'C');			//ÏÔÊ¾¡®C¡¯

	if(T0_num%2==0)	//Å¼Êı´ÎÏÔÊ¾£¬ÆæÊı´Î²»ÏÔÊ¾¡£ÕâÑù¾Í»áÓĞÉÁË¸Ğ§¹û£¬¿ÉÒÔÇå³ş¿´µ½µ±Ç°ÉèÖÃµÄÊÇÄÄ¸öÖµ
	{
		LCD_disp_char(3,2,ASCII[temp_up/10]);	//ÏÔÊ¾ ÎÂ¶ÈÉÏÏŞ
		LCD_disp_char(4,2,ASCII[temp_up%10]);
		LCD_disp_char(11,2,ASCII[temp_down/10]);//ÏÔÊ¾ ÎÂ¶ÈÏÂÏŞ
		LCD_disp_char(12,2,ASCII[temp_down%10]);
		Delay_ms(1);
	}
	else			 //ÆæÊı´Î²»ÏÔÊ¾
	{
		switch(set_f)//¸ù¾İµ±Ç°ÉèÖÃµÄÄÚÈİ£¬¶ÔÓ¦Î»ÖÃÉÁË¸
		{
			case 1:lcd1602_write_character(3,2,"  "); break;
			case 2:lcd1602_write_character(11,2,"  "); break;
			default: break;
		}
	}
}
/********************************************************
º¯ÊıÃû³Æ:void display3(void)
º¯Êı×÷ÓÃ:ÏÔÊ¾ÉèÖÃÊ±Êª¶ÈÉÏÏÂÏŞ
²ÎÊıËµÃ÷:
********************************************************/
void display3(void)
{
	T0_num++;

	lcd1602_write_character(0,1,"Set Humi limit: ");
	lcd1602_write_character(0,2,"H:");
	lcd1602_write_character(4,2,"%RH");
	lcd1602_write_character(9,2,"L:");
	lcd1602_write_character(13,2,"%RH");

	if(T0_num%2==0)	//Å¼Êı´ÎÏÔÊ¾£¬ÆæÊı´Î²»ÏÔÊ¾¡£ÕâÑù¾Í»áÓĞÉÁË¸Ğ§¹û£¬¿ÉÒÔÇå³ş¿´µ½µ±Ç°ÉèÖÃµÄÊÇÄÄ¸öÖµ
	{
		LCD_disp_char(2,2,ASCII[humi_up/10]);	   //ÏÔÊ¾ Êª¶ÈÉÏÏŞ
		LCD_disp_char(3,2,ASCII[humi_up%10]);
		LCD_disp_char(11,2,ASCII[humi_down/10]);   //ÏÔÊ¾ Êª¶ÈÏÂÏŞ
		LCD_disp_char(12,2,ASCII[humi_down%10]);
		Delay_ms(1);
	}
	else			 //ÆæÊı´Î²»ÏÔÊ¾
	{
		switch(set_f)//¸ù¾İµ±Ç°ÉèÖÃµÄÄÚÈİ£¬¶ÔÓ¦Î»ÖÃÉÁË¸
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

	if(T0_num%2==0)	//Å¼Êı´ÎÏÔÊ¾£¬ÆæÊı´Î²»ÏÔÊ¾¡£ÕâÑù¾Í»áÓĞÉÁË¸Ğ§¹û£¬¿ÉÒÔÇå³ş¿´µ½µ±Ç°ÉèÖÃµÄÊÇÄÄ¸öÖµ
	{
		LCD_disp_char(2,2,ASCII[KQH/10]);	   //ÏÔÊ¾ Êª¶ÈÉÏÏŞ
		LCD_disp_char(3,2,ASCII[KQH%10]);
//		LCD_disp_char(11,2,ASCII[humi_down/10]);   //ÏÔÊ¾ Êª¶ÈÏÂÏŞ
//		LCD_disp_char(12,2,ASCII[humi_down%10]);
		Delay_ms(1);
	}
	else			 //ÆæÊı´Î²»ÏÔÊ¾
	{
		switch(set_f)//¸ù¾İµ±Ç°ÉèÖÃµÄÄÚÈİ£¬¶ÔÓ¦Î»ÖÃÉÁË¸
		{
			case 3:lcd1602_write_character(2,2,"  "); break;
			case 4:lcd1602_write_character(11,2,"  "); break;
			default: break;
		}
	}
}
/********************************************************
º¯ÊıÃû³Æ:void scan(void)
º¯Êı×÷ÓÃ:°´¼ü¼ì²âº¯Êı
²ÎÊıËµÃ÷:
********************************************************/ 
void scan()
{
 	//ÉèÖÃ¼ü£¬²»Ö§³ÖÁ¬°´
	if(key_set==0)		//°´¼ü°´ÏÂ
	{
		delayms(7);		//ÑÓÊ±Ïû¶¶
		if(key_set==0)	//ÔÙ´ÎÈ·ÈÏ°´¼ü°´ÏÂ
		{
			led1=1;	    //¹Ø±Õ±¨¾¯µÆ
			led2=1;
			buzz=1;	    //¹Ø±Õ·äÃùÆ÷
			if(set_f%2==0)//½øÈëÉèÖÃÏÈÇå³ıÆÁÄ»£¬ÏÔÊ¾ÉèÖÃ²¿·Ö
			{
				LCD_write_command(0x01);        //Çå³ıÆÁÄ»ÏÔÊ¾
				delay_n40us(100);		        //ÑÓÊ±µÈ´ıÈ«²¿Çå³ıÍê±Ï
			}
			set_f++;	//ÉèÖÃ±äÁ¿+1
			if(set_f==8)//È«²¿ÉèÖÃÍê³É£¬ÍË³öÉèÖÃ
			{
				set_f=0;//ÖØÖÃÉèÖÃ±äÁ¿

//				AT24C02_write_date(0,humi_up);  //Ğ´Èë¡¾Êª¶ÈÉÏÏŞ¡¿Êı¾İ±£´æ
//				AT24C02_write_date(1,humi_down);//Ğ´Èë¡¾Êª¶ÈÏÂÏŞ¡¿Êı¾İ±£´æ
//				AT24C02_write_date(2,temp_up);  //Ğ´Èë¡¾ÎÂ¶ÈÉÏÏŞ¡¿Êı¾İ±£´æ
//				AT24C02_write_date(3,temp_down);//Ğ´Èë¡¾ÎÂ¶ÈÏÂÏŞ¡¿Êı¾İ±£´æ
			}			
		}
		while(!key_set);//¼ì²â°´¼üËÉ¿ª
	}

	//¼ÓÖµ¼ü£¬Ö§³ÖÁ¬°´
	if(key_jia==0)
	{
		delayms(7);
		if(key_jia==0&&set_f!=0)
		{	
			if(set_f==1)        //ÉèÖÃ ÎÂ¶ÈÉÏÏŞÖµ
			{
				if(temp_up<99)  //×î´óÖµ¿ÉÉèÖÃ99¡æ
					temp_up++;  //Êª¶ÈÉÏÏŞ+1¡æ
			}
	
			if(set_f==2)        //ÉèÖÃ ÎÂ¶ÈÏÂÏŞÖµ
			{
				if(temp_down<99&&temp_down+1<temp_up)//×î´óÖµ¿ÉÉèÖÃ99¡æ£¬²¢ÇÒÏÂÏŞ²»¿É³¬¹ıÉÏÏŞ
					temp_down++;//ÎÂ¶ÈÏÂÏŞ+1¡æ
			}

			if(set_f==3)        //ÉèÖÃ Êª¶ÈÉÏÏŞÖµ
			{
				if(humi_up<99)  //×î´óÖµ¿ÉÉèÖÃ99%RH
					humi_up++;  //Êª¶ÈÉÏÏŞ+1%RH
			}
	
			if(set_f==4)        //ÉèÖÃ Êª¶ÈÏÂÏŞÖµ
			{
				if(humi_down<99&&humi_down+1<humi_up)//×î´óÖµ¿ÉÉèÖÃ99%RH£¬²¢ÇÒÏÂÏŞ²»¿É³¬¹ıÉÏÏŞ
					humi_down++;//Êª¶ÈÏÂÏŞ+1%RH
			}
			if(set_f==5)        //ÉèÖÃ Êª¶ÈÏÂÏŞÖµ
			{
//				if(KQH<99&&KQH+1<humi_up)//×î´óÖµ¿ÉÉèÖÃ99%RH£¬²¢ÇÒÏÂÏŞ²»¿É³¬¹ıÉÏÏŞ
					KQH++;//Êª¶ÈÏÂÏŞ+1%RH
			}
		}
		else if(key_jia==0&&set_f==0)//ÊÖ¶¯¿ØÖÆ¼ÌµçÆ÷1¿ª¹Ø
		{
			sz=1;//ÊÖ¶¯
			jdq1=~jdq1;		    //ÇĞ»»¼ÌµçÆ÷1¿ª/¹Ø
			if(jdq1==0)			//¸ù¾İ¼ÌµçÆ÷1×´Ì¬·´À¡
				WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
			else
				WIFI_DATA_SEND("RT+OK:JDQ1-OPEN");
			clearBuff();
		}
	}

	//¼õÖµ¼ü£¬Ö§³ÖÁ¬°´
	if(key_jian==0)
	{
		delayms(7);
		if(key_jian==0&&set_f!=0)
		{
			if(set_f==1)        //ÉèÖÃÎÂ¶ÈÉÏÏŞÖµ
			{
				if(temp_up!=0&&temp_up>temp_down+1)//×îĞ¡Öµ¿ÉÉèÖÃ0¡æ£¬²¢ÇÒÉÏÏŞµÃ±ÈÏÂÏŞ´ó
					temp_up--;  //ÎÂ¶ÈÉÏÏŞÖµ-1¡æ
			}
	
			if(set_f==2)        //ÉèÖÃÎÂ¶ÈÏÂÏŞÖµ
			{
				if(temp_down!=0)//×îĞ¡Öµ¿ÉÉèÖÃ0¡æ
					temp_down--;//ÎÂ¶ÈÏÂÏŞÖµ-1¡æ
			}

			if(set_f==3)        //ÉèÖÃÊª¶ÈÉÏÏŞÖµ
			{
				if(humi_up!=0&&humi_up>humi_down+1)//×îĞ¡Öµ¿ÉÉèÖÃ0%RH£¬²¢ÇÒÉÏÏŞµÃ±ÈÏÂÏŞ´ó
					humi_up--;  //Êª¶ÈÉÏÏŞÖµ-1%RH
			}
					 
			if(set_f==4)        //ÉèÖÃÊª¶ÈÏÂÏŞÖµ
			{
				if(KQH!=0)//×îĞ¡Öµ¿ÉÉèÖÃ0%RH
					KQH--;//Êª¶ÈÏÂÏŞÖµ-1%RH
			}
			
					if(set_f==5)        //ÉèÖÃÊª¶ÈÏÂÏŞÖµ
			{
				if(KQH!=0)//×îĞ¡Öµ¿ÉÉèÖÃ0%RH
					KQH--;//Êª¶ÈÏÂÏŞÖµ-1%RH
			}
		}
		else if(key_jian==0&&set_f==0)//ÊÖ¶¯¿ØÖÆ¼ÌµçÆ÷2¿ª¹Ø
		{
			sz=0;//×Ô¶¯
			jdq2=~jdq2;			 //ÊÖ¶¯¿ØÖÆ¼ÌµçÆ÷2¿ª¹Ø
			if(jdq2==0)			 //¸ù¾İ¼ÌµçÆ÷2×´Ì¬·´À¡
				WIFI_DATA_SEND("RT+OK:JDQ2-OPEN");
			else
				WIFI_DATA_SEND("RT+OK:JDQ2-CLOSED");
			clearBuff();
		}	
	}
}
/********************************************************
º¯ÊıÃû³Æ:void main()
º¯Êı×÷ÓÃ:Ö÷º¯Êı
²ÎÊıËµÃ÷:
********************************************************/
void main()
{	
	if(key_jian==0)
	{
		delayms(100);
		if(key_jian==0)
		{
			;
//			AT24C02_write_date(0,85);//Ğ´Èë¡¾Êª¶ÈÉÏÏŞÖµ¡¿Êı¾İ±£´æ
//			AT24C02_write_date(1,40);//Ğ´Èë¡¾Êª¶ÈÏÂÏŞÖµ¡¿Êı¾İ±£´æ
//			AT24C02_write_date(2,38);//Ğ´Èë¡¾ÎÂ¶ÈÉÏÏŞÖµ¡¿Êı¾İ±£´æ
//			AT24C02_write_date(3,16);//Ğ´Èë¡¾ÎÂ¶ÈÏÂÏŞÖµ¡¿Êı¾İ±£´æ
		}
	}

//	humi_up=AT24C02_read_date(0);  //¶ÁÈ¡´æ´¢µÄ¡¾Êª¶ÈÉÏÏŞÖµ¡¿Êı¾İ
//	humi_down=AT24C02_read_date(1);//¶ÁÈ¡´æ´¢µÄ¡¾Êª¶ÈÏÂÏŞÖµ¡¿Êı¾İ
//	temp_up=AT24C02_read_date(2);  //¶ÁÈ¡´æ´¢µÄ¡¾ÎÂ¶ÈÉÏÏŞÖµ¡¿Êı¾İ
//	temp_down=AT24C02_read_date(3);//¶ÁÈ¡´æ´¢µÄ¡¾ÎÂ¶ÈÏÂÏŞÖµ¡¿Êı¾İ
	LCD_init();				    //LCD1602³õÊ¼»¯
	lcd1602_write_pic(0x00,pic);//½«×Ô¶¨Òå×Ö·û¡°¡ã¡±Ğ´Èëµ½LCD1602ÖĞ
  ESP8266_init();			    //ESP8266³õÊ¼»¯

	while(1)			        //ËÀÑ­»·
	{
		scan();		          //°´¼ü¼ì²â´¦Àí
		if(set_f==0)			  //Õı³£ÏÔÊ¾Ä£Ê½
		{
			T0_num++;			    //Ñ­»·±äÁ¿+1
			if(T0_num>=100)		//Ã¿Ñ­»·100´Î£¬½øÈë¶ÁÈ¡¡¢ÏÔÊ¾Êı¾İ
			{
				T0_num=0;		    //ÖØÖÃÑ­»·±äÁ¿
				DHT11_read();   //¶ÁÈ¡DHT11ÎÂÊª¶È
				mq2=GetAD0832(0);				//?¨¢¨¨??¨¬?¨ª?¦Ì
				guang= GetAD0832(1); //??DDA/D¡Áa??
				display();	    //ÏÔÊ¾µ±Ç°ÎÂ¡¢Êª¶È
			}
		}
		else if(set_f==1||set_f==2)
		{
			display2();	    //ÏÔÊ¾ÉèÖÃÎÂ¶ÈÉÏÏÂÏŞ
		}
		else if(set_f==3||set_f==4)
		{
			display3();  	//ÏÔÊ¾ÉèÖÃÊª¶ÈÉÏÏÂÏŞ
		}
		else if(set_f==5||set_f==6)
		{
			display4();
		}		
	}
}