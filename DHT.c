#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


#define MAX_BUF 1024
#define p1 15		//온도센서1
#define p2 16		//온도센서2
#define pr1 22		//펌프
#define pr2 23		//가습기
#define pr3 24		//전구
#define pr4 25		//팬
#define IN1 29     //BCN_GPIO 21
#define IN2 28      //BCM_GPIO 20
#define IN3 27       //BCM_GPIO 16
#define IN4 26       //BCM_GPIO 12
#define speed 2
#define STEP 63
#define THRASHOLD 40
#define MAX_BIT 40

int Data[10];
int Data2[10];
int avrData[10];
int sock;

int temp = 0;
int co = 0;			// Count
int co2 = 0;

int tret = 0;
int tret1 = 1;
int tret2 = 1;
int fanSta = 0;
int setTemp = 250;
int setHum = 55;
char buf[1024];
char Getbuf[1024] = "                        \n";

int PORT = 9000;
char IP[25] = "192.168.2.58";

int Feed_H = 0;
int Feed_M = 43;
int count = 0;

int waterLevel = 255;
int waterSign = 1;

time_t current;
struct tm* t ;

void setsteps(int n1, int n2, int n3, int n4)
{
	pinMode(IN1,OUTPUT);
	digitalWrite(IN1,n1);
	pinMode(IN2,OUTPUT);
	digitalWrite(IN2,n2);
	pinMode(IN3,OUTPUT);
	digitalWrite(IN3,n3);
	pinMode(IN4,OUTPUT);
	digitalWrite(IN4,n4);
}

// forward 함수
// 정방향(시계방향)으로 회전
// steps 수만큼 회전 
 void forward (int br,int steps)
{
	int i;
	for(i=0;i<=steps;i++)
	{
                setsteps(1, 1, 0, 0);
                delay(br);
		setsteps(0, 1, 0, 0);
		delay(br);
		setsteps(0, 1, 1, 0);
		delay(br);
		setsteps(0, 0, 1, 0);
		delay(br);
		setsteps(0, 0, 1, 1);
		delay(br);
		setsteps(0, 0, 0, 1);
		delay(br);
		setsteps(1, 0, 0, 1);
		delay(br);
		setsteps(1, 0, 0, 0);
		delay(br);

	}
}

// backward 함수
// 역방향(반시계방향)으로 회전
// steps 수만큼 회전 
void backward (int br,int steps)
{
	int k;
	for(k=0;k<=steps;k++)
	{
		setsteps(1, 0, 0, 0);
		delay(br);
		setsteps(1, 0, 0, 1);
		delay(br);
		setsteps(0, 0, 0, 1);
		delay(br);
		setsteps(0, 0, 1, 1);
		delay(br);
		setsteps(0, 0, 1, 0);
		delay(br);
		setsteps(0, 1, 1, 0);
		delay(br);
		setsteps(0, 1, 0, 0);
		delay(br);
		setsteps(1, 1, 0, 0);
        delay(br);
	}
}
void pump()
{
	digitalWrite(pr1,LOW);
	delay(1000);
	digitalWrite(pr1,HIGH);
	
}
void Feeding()
{
	count++;	//7번만 돌아야하기 때문에 -> 급양기의 칸이 7칸만 사용가능
	for(int i = count ; i<count+1;i++)
	{
		forward(speed,STEP*count);
		delay(2000);
		backward(speed,STEP*count);
		printf("%d:%d:%d", t->tm_hour, t->tm_min, t->tm_sec);
		if(count==8) 
		{
			break;
			printf("밥통 비었음.\n");
		}
		setsteps(0, 0, 0, 0);
	}
}

void* FeedThr()
{
	
	while(1)
	{
		current = time(NULL);
		t = localtime(&current);
		if(t->tm_hour==Feed_H)
		{
			if(t->tm_min==Feed_M)
			{
				pump();
				Feeding();
				delay(60000);
			}
		}
	}
}

int level(int n)
{
	waterLevel = n*100/255;
	return waterLevel;
}

void* WaterLevel()
{
	int Level_data;
	int fd;
	int Level_count=0;
	fd = wiringPiI2CSetup(0x48);
	wiringPiI2CWrite(fd, 2);
	
	while(1)
	{
		Level_data = wiringPiI2CRead(fd);
		delay(2000);
		
		if(level(Level_data)<10) Level_count++;
		else waterSign = 1;
		if(Level_count==30)
		{
			printf("물 보충 필요\n");
			if(level(Level_data)<25)
			{
				Level_count=0;
				waterSign = 0;
			}
		}
	}
}

void* ReadBuf()
{
	while(1)
	{
		read(sock, Getbuf, strlen(Getbuf));
		delay(500);
	}
}

void StartSignal()
{
	pinMode(p1, OUTPUT);
	digitalWrite(p1,HIGH);delay(100);
	digitalWrite(p1,LOW);delay(18);
	digitalWrite(p1,HIGH);
	
}
void StartSignal2()
{
	pinMode(p2, OUTPUT);
	digitalWrite(p2,HIGH);delay(100);
	digitalWrite(p2,LOW);delay(18);
	digitalWrite(p2,HIGH);
}

int GetRespons()
{
	int tm1 = micros();
	pinMode(p1, INPUT);
	while(digitalRead(p1)== HIGH)
	{
		if(micros() - tm1 > 1000000) return -1;
	}
	int st = micros();
	tm1 = micros();
	while(digitalRead(p1)== LOW){
		if(micros() - tm1 > 1000000) return -2;
	}
	int en = micros();
	tm1 = micros();
	while(digitalRead(p1)== HIGH){
		if(micros() - tm1 > 1000000) return -3;
	}
	if(en - st < 20) return -1;
	return 0;
}

int GetRespons2()
{
	int tm1 = micros();
	pinMode(p2, INPUT);
	while(digitalRead(p2)== HIGH)
	{
		if(micros() - tm1 > 1000000) return -11;
	}
	int st = micros();
	tm1 = micros();
	while(digitalRead(p2)== LOW){
		if(micros() - tm1 > 1000000) return -12;
	}
	int en = micros();
	tm1 = micros();
	while(digitalRead(p2)== HIGH){
		if(micros() - tm1 > 1000000) return -13;
	}
	if(en - st < 20) return -1;
	return 0;
}

int ReadData()
{
	int st ,en;
	for (int i = 0; i < MAX_BIT; i++) Data[i] = 0;
	for (int i = 0; i < MAX_BIT; i++)
	{
		int tm1 = micros();
		while(digitalRead(p1)== LOW)
		{
			if(micros() - tm1 > 1000000) return -4;
		}
		st = micros();
		tm1 = micros();
		while(digitalRead(p1)== HIGH)
		{
			if(micros() - tm1 > 1000000) return -5;
		}
		en = micros();
		Data[i/8] <<= 1;
		if(en - st > THRASHOLD)Data[i/8] |=1;
	}
	if (Data[0] + Data[1] + Data[2] + Data[3] == Data[4])return 1;
	return 0;
}
int ReadData2()
{
	int st ,en;
	for (int i = 0; i < MAX_BIT; i++) Data2[i] = 0;
	for (int i = 0; i < MAX_BIT; i++)
	{
		int tm1 = micros();
		while(digitalRead(p2)== LOW)
		{
			if(micros() - tm1 > 1000000) return -14;
		}
		st = micros();
		tm1 = micros();
		while(digitalRead(p2)== HIGH)
		{
			if(micros() - tm1 > 1000000) return -15;
		}
		en = micros();
		Data2[i/8] <<= 1;
		if(en - st > THRASHOLD)Data2[i/8] |=1;
	}
	if (Data2[0] + Data2[1] + Data2[2] + Data2[3] == Data2[4])return 1;
	return 0;
}


void Run()
{
	for(int i=0; i<1;i++)
	{
		StartSignal();
		int r = GetRespons();
		printf("<%d>",co);
		co++;
		if(r < 0) 
		{
			printf("%d",r);
			continue;
		}
		int ret = ReadData();
		tret=ret;
		if(ret < 0)
		{
			printf("%d", ret);
			tret1 = -1;
			continue;
		}
		else tret1 = 1;
		
		printf("습도 :  %d.%d%% 온도 : %d.%d℃ Checksum : %d", Data[0], Data[1], Data[2], Data[3], Data[4]);
		if (ret == 1)
		{
			printf("...OK\n");
		}
		else 
		{
			printf("...ERROR!\n");
		}
				
		delay(1000);
	}
	
}

void Run2()
{
	for(int i=0; i<1;i++)
	{
		StartSignal2();
		int r2 = GetRespons2();
		printf("[%d]",co2);
		co2++;
		
		if(r2 < 0) 
		{
			printf("%d",r2);
			continue;
		}
		int ret2 = ReadData2();
		tret+=ret2;

		if(ret2 < 0)
		{
			printf("%d", ret2);
			tret2 = -1;
			continue;
		}
		else tret2 = 1;
		printf("습도 :  %d.%d%% 온도 : %d.%d℃ Checksum : %d", Data2[0], Data2[1], Data2[2], Data2[3], Data2[4]);
		if (ret2 == 1)
		{
			printf("...OK\n");
		}
		else 
		{
			printf("...ERROR!\n");
		}
		
		delay(1000);
	}
}

void* Fan()
{
	while (1)
	{
		if(fanSta == 1)
		{
			digitalWrite(pr4,0);
			delay(2500);
			digitalWrite(pr4,1);
			fanSta = 0;
			delay(15000);
		}
	}
}

void average()
{
	avrData[0]=(Data[0]+Data2[0]);		// 습도 정수부 합
	avrData[1]=(Data[1]+Data2[1]);		// 습도 소수부 합 
	avrData[2]=((Data[2]+Data2[2]) * 10 ); //온도 합
	avrData[3]=avrData[2] + (Data[3]+Data2[3]); //온도 최종 값 
	avrData[4]=(Data[4]+Data2[4])/2; //checksum
	printf("<평균>  습도 :  %d.%d%% 온도 : %d℃ Checksum : %d 수위 : %d%%", avrData[0], avrData[1], avrData[3], avrData[4], waterLevel);

	if (tret==2)
	{
		printf("...OK\n");
		
		if(avrData[0]/2 > setHum)
		{
			digitalWrite(pr2,1);
			fanSta = 1;
		}
		else
		{
			digitalWrite(pr2,0);
		}

		if(avrData[3]/2 > setTemp)
		{
			digitalWrite(pr3,1);
		}
		else
		{
			digitalWrite(pr3,0);
		}
		
		if(avrData[3]/2 > 260)
		{
			fanSta = 1;
		}
	}
	else 
	{
		printf("...ERROR!\n");
	} 
	temp = avrData[3];

	memset(buf,0,MAX_BUF);
	//sprintf(buf, ":1,%d,%d,%d,%d,%d,%d,%d", avrData[0], avrData[3], tret1, tret2, setTemp, setHum, waterSign);	// 습도랑 온도 /2해야 함, // tret은 -1이면 오류 //  1이면 정상	
	sprintf(buf, ":1,%d,%d,%d,%d", avrData[0], avrData[3], tret1, tret2);
	printf("%s",Getbuf);
	//char *SetGets = strtok(Getbuf,',');
	//write(sock, buf, strlen(buf));
	//printf("%s", buf);//sprintf 확인용
}

int main(int n ,char *argv[])
{
	char name[64] = "user5" ;
	struct sockaddr_in server;
	pthread_t thread_Fan;
	pthread_t thread_Feed;
	pthread_t WaterLevelThr;
	pthread_t ReadThr;
	wiringPiSetup();
	pinMode(pr1, OUTPUT);
	pinMode(pr2, OUTPUT);
	pinMode(pr3, OUTPUT);
	pinMode(pr4, OUTPUT);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(PORT);
	
	pthread_create(&thread_Fan, NULL, Fan, NULL);
	pthread_create(&thread_Feed, NULL, FeedThr, NULL);
	pthread_create(&WaterLevelThr, NULL, WaterLevel, NULL);
	
	//connect(sock, (struct sockaddr*)&server, sizeof(server));
	
	//pthread_create(&ReadThr, NULL, ReadBuf, NULL);
	while(1)
	{
		Run();
		Run2();
		average();
		tret==0;		
//그 전 사이클에서 제대로 나와서 값이 2인데 
//다음 사이클 첫번째에서 오류가 뜨면 그대로 tret가 2이기때문에 
//평균값이 ok가 나올수있으므로 tret초기
	}
	return 0;
}
