#include "weather.h"
#include "wifista.h"
#include "usart.h"
#include "malloc.h"
#include "usart3.h"
#include "delay.h"
#include "text.h"
#include "parsejson.h"

//天气连接端口号:80	
#define WEATHER_PORTNUM 	"80"
//天气服务器IP
#define WEATHER_SERVERIP 	"api.seniverse.com"

//时间端口号
#define TIME_PORTNUM			"80"
//时间服务器IP
#define TIME_SERVERIP			"www.beijing-time.org"



typedef struct   //结构体。
{
    vu16  year;
    vu8   month;
    vu8   date;
    vu8   hour;
    vu8   min;
    vu8   sec;	 
}nt_calendar_obj;	   

//定义结构体变量

 nt_calendar_obj nwt;  //定义结构体变量

//获取一次实时天气
//返回：0---获取成功，1---获取失败
u8 get_current_weather(void)
{
	u8 *p;
	u8 res;
//	u8 ipbuf[16]; 	//IP缓存
	p=mymalloc(SRAMIN,40);							//申请40字节内存
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WEATHER_SERVERIP,WEATHER_PORTNUM);    //配置目标TCP服务器
	res = atk_8266_send_cmd(p,"OK",200);//连接到目标TCP服务器
	if(res==1)
	{
		myfree(SRAMIN,p);
		return 1;
	}
	delay_ms(300);
	atk_8266_send_cmd("AT+CIPMODE=1","OK",100);      //传输模式为：透传	
//	atk_8266_get_wanip(ipbuf);//获取WAN IP

	USART3_RX_STA=0;
	atk_8266_send_cmd("AT+CIPSEND","OK",100);         //开始透传
	printf("start trans...\r\n");
	u3_printf("GET https://api.seniverse.com/v3/weather/now.json?key=50alegxkcaehcge6&location=wuxi&language=zh-Hans&unit=c\n\n");	
	delay_ms(20);//延时20ms返回的是指令发送成功的状态
//	atk_8266_at_response(1);
	USART3_RX_STA=0;	//清零串口3数据
	delay_ms(1000);
//	atk_8266_at_response(0);
	if(USART3_RX_STA&0X8000)		//此时再次接到一次数据，为天气的数据
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
	} 
	parse_now_weather();

	
	atk_8266_quit_trans();//退出透传
	atk_8266_send_cmd("AT+CIPCLOSE","OK",50);         //关闭连接
	myfree(SRAMIN,p);
	return 0;
}

//获取3天的天气
u8 get_3days_weather(void)
{
	u8 *p;
	u8 res;
	u8 ipbuf[16]; 	//IP缓存
	p=mymalloc(SRAMIN,40);							//申请40字节内存
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WEATHER_SERVERIP,WEATHER_PORTNUM);    //配置目标TCP服务器
	res = atk_8266_send_cmd(p,"OK",200);//连接到目标TCP服务器
	if(res==1)
	{
		myfree(SRAMIN,p);
		return 1;
	}
	delay_ms(300);
	atk_8266_send_cmd("AT+CIPMODE=1","OK",100);      //传输模式为：透传	
	atk_8266_get_wanip(ipbuf);//获取WAN IP
	sprintf((char*)p,"IP地址:%s 端口:%s",ipbuf,(u8*)WEATHER_PORTNUM);
//	Show_Str(30,65,200,12,p,12,0);				//显示IP地址和端口	
	USART3_RX_STA=0;
	atk_8266_send_cmd("AT+CIPSEND","OK",100);         //开始透传
	printf("start trans...\r\n");
	
  //我的密钥 50alegxkcaehcge6
	//例题密钥 x3owc7bndhbvi8oq
	u3_printf("GET https://api.seniverse.com/v3/weather/daily.json?key=50alegxkcaehcge6&location=wuxi&language=zh-Hans&unit=c&start=0&days=5\n\n");
	delay_ms(20);//延时20ms返回的是指令发送成功的状态
//	atk_8266_at_response(1);
	USART3_RX_STA=0;	//清零串口3数据
	delay_ms(1000);
//	atk_8266_at_response(0);
	if(USART3_RX_STA&0X8000)		//此时再次接到一次数据，为天气的数据
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
	} 
	
	parse_3days_weather();

	
	atk_8266_quit_trans();//退出透传
	atk_8266_send_cmd("AT+CIPCLOSE","OK",50);         //关闭连接
	myfree(SRAMIN,p);
	return 0;
}

//获取北京时间
u8 get_beijing_time(void)
{
	u8 *p;
	u8 res;
	
	u8 *resp;
	u8 *p_end;
	u8 ipbuf[16]; 	//IP缓存
	p=mymalloc(SRAMIN,40);							//申请40字节内存
	resp=mymalloc(SRAMIN,10);
	p_end=mymalloc(SRAMIN,40);
	
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",TIME_SERVERIP,TIME_PORTNUM);    //配置目标TCP服务器
	res = atk_8266_send_cmd(p,"OK",200);//连接到目标TCP服务器
	if(res==1)
	{
		myfree(SRAMIN,p);
		return 1;
	}
	delay_ms(300);
	atk_8266_send_cmd("AT+CIPMODE=1","OK",100);      //传输模式为：透传	
	atk_8266_get_wanip(ipbuf);//获取WAN IP
	
	sprintf((char*)p,"IP地址:%s 端口:%s",ipbuf,(u8*)TIME_PORTNUM);
	
	//Show_Str(30,65,200,12,p,12,0);				//显示IP地址和端口	
	 printf("设备 %s\r\n",p);
	
	USART3_RX_STA=0;
	atk_8266_send_cmd("AT+CIPSEND","OK",100);         //开始透传
	printf("start trans...\r\n");

	u3_printf("GET /time15.asp HTTP/1.1\r\nHost:www.beijing-time.org\n\n");
	delay_ms(20);
//	atk_8266_at_response(1);
	USART3_RX_STA=0;	
	delay_ms(1000);
	
//atk_8266_at_response(1);
	
	
	//USART3_RX_BUF 为时间信息

	
	if(USART3_RX_STA & 0x8000)
                        {
													resp="Date";
                        USART3_RX_BUF[USART3_RX_STA & 0x7ff] = 0;
													printf("get_tim_srt：%s\r\n",USART3_RX_BUF);
                        if(strstr((char*)USART3_RX_BUF,(char*)resp)) 
                         {       resp="GMT";
                                p_end = (u8*)strstr((char*)USART3_RX_BUF,(char*)resp);
                                p = p_end - 9; 
                               printf("get_net_str %s\r\n",p);
                               nwt.hour = ((*p - 0x30)*10 + (*(p+1) - 0x30) + 8) % 24;  //GMT0-->GMT8
													      
                                nwt.min = ((*(p+3) - 0x30)*10 + (*(p+4) - 0x30)) % 60;
													     
                                nwt.sec = ((*(p+6) - 0x30)*10 + (*(p+7) - 0x30)) % 60;
                             
													       nwt.year = ((*(p-5) - 0x30)*1000 + (*(p-4) - 0x30)*100+ (*(p-3) - 0x30)*10+ (*(p-2) - 0x30)); 
                                
													       nwt.date = ((*(p-12) - 0x30)*10 + (*(p-11) - 0x30)); 

                                if        ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Jan")) nwt.month=1; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Feb")) nwt.month=2; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Mar")) nwt.month=3; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Apr")) nwt.month=4; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "May")) nwt.month=5; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Jun")) nwt.month=6; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Jul")) nwt.month=7; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Aug")) nwt.month=8; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Sep")) nwt.month=9; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Oct")) nwt.month=10; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Nov")) nwt.month=11; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Dec")) nwt.month=12;
                                                              
														     
													       printf("nwt.year = %d\r\n",nwt.year);
													       printf("nwt.month = %d\r\n",nwt.month);
  													     printf("nwt.date = %d\r\n",nwt.date);  //获取data 28日
                   
													       printf("nwt.hour = %d\r\n",nwt.hour);
													       printf("nwt.min = %d\r\n",nwt.min);
													       printf("nwt.sec = %d\r\n",nwt.sec);
                                USART3_RX_STA = 0;
                                 
																 
														printf("uddate:nettime!!!");
														RTC_Set(nwt.year,nwt.month ,nwt.date ,nwt.hour ,nwt.min,nwt.sec);
														
														
                                }
                        USART3_RX_STA = 0;
																
												myfree(SRAMIN,resp);
											  myfree(SRAMIN,p_end);
																
						
                        }               
	printf("\r\n\r\n");
	
//  printf("get_beijing_time= %s\r\n",USART3_RX_BUF);
	
	atk_8266_quit_trans();//退出透传
	atk_8266_send_cmd("AT+CIPCLOSE","OK",50);         //关闭连接
	myfree(SRAMIN,p);
	return 0;
}
