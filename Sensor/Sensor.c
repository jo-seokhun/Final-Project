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
#include <setjmp.h>


#define MAX_BUF 512
#define p1 15		//온도센서1
#define p2 16		//온도센서2
#define pr1 22		//펌프
#define pr2 23		//가습기
#define pr3 24		//전구
#define pr4 25		//팬
#define IN1 29      //BCN_GPIO 21
#define IN2 28      //BCM_GPIO 20
#define IN3 27      //BCM_GPIO 16
#define IN4 26      //BCM_GPIO 12
#define speed 2
#define STEP 63		// 스텝 모터 한 칸 각도
#define THRASHOLD 40
#define MAX_BIT 40

#define TRY do{ jmp_buf ex_buf__; if( !setjmp(ex_buf__) ){
#define CATCH } else {
#define ETRY } }while(0)
#define THROW longjmp(ex_buf__, 1)

// DHT-11
int Data[10];
int Data2[10];
int avrData[10];
int tret = 0;		// 센서 에러 유무(2: OK, 0 이하: ERROR)
int tret1 = 1;		// 1번 센서 에러 유무(1: OK, -1: ERROR)
int tret2 = 1;		// 2번 센서 에러 유무
int setTemp = 26;	// 설정 온도
int setHum = 70;	// 설정 습도

int temp = 0;		// 양 센서에서 나온 온도의 합
int co = 0;			// 1번 센서 값 Count
int co2 = 0;		// 2번 센서 값 Count

// Fan
int fanSta = 0;		// 1: 동작, 0: 멈춤

// Network, Server
struct sockaddr_in server;
int sock;
int PORT = 9090;
char IP[25] = "192.168.2.58";		// Server IP
char buf[512];		// 서버로
char Getbuf[512] = "                        \n";	// Server로 부터 받는 Data 저장
int ServerCon = 0;

// 급여
int Feeding_Sign = 0;
int Feed_Mode = 1;	
int Food = 0;	
int Feed_H = 5;		// 급여 시간 
int Feed_M = 52;	// 급여 시간     
int count = 0;		// 스텝 모터 카운트(급여기)

// 물통
int waterLevel = 255;
int waterSign = 1;

// 시간
time_t current;
struct tm* t ;

//스레드
pthread_t thread_DHT;
pthread_t thread_Fan;
pthread_t thread_Feed;
pthread_t WaterLevelThr;
pthread_t ReadThr;

void* Fan()		// Fan 스레드
{
	while (1)
	{
		if(fanSta == 1)
		{
			digitalWrite(pr4,0);
			delay(2000);				// 2.0초 돎
			digitalWrite(pr4,1);
			fanSta = 0;
			delay(15000);				// 15초 쉼
		}
	}
}

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
	delay(2325);
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
		if(count==8) 
		{
			break;
			//printf("밥통 비었음.\n");
			Feed_Mode = 0;
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
				if(Feed_Mode != 0)
				{
					Feeding();
				}
				delay(60000);
			}
		}
	}
}

int level(int n)		// 수위센서로 들어오는Low Data 처리 함수
{
	waterLevel = n*100/255;		// Low Data 백분률
	return waterLevel;
}

void* WaterLevel()		// 수위센서 스레드
{
	int fd, Level_data;	// 센서 주소, 수위센서 Data
	int Level_count=0;	// 수위 확인절차 Count
	fd = wiringPiI2CSetup(0x48);	// i2c Detect
	wiringPiI2CWrite(fd, 2);
	
	while(1)
	{
		Level_data = wiringPiI2CRead(fd);	// 수위센서 LowData
		delay(500);
		
		if(level(Level_data)<10) Level_count++;		// 백분률이 10이하면 Count 증가(30이상 찍히면 물보충 필요 알람) 
		else								// 물이 다시 차면 초기화 
		{
			waterSign = 1;
			Level_count = 0;
		}

		if(Level_count==30)					// 물이 일정수준 이하가 계속되면
		{
			if(level(Level_data)<10)		// 카운트 30 넘었는데 수위 10이하(오류 처리)
			{
				printf("물 보충 필요\n");
				Level_count=0;
				waterSign = 0;
			}
			else							// 물이 다시 차면 초기화 
			{
				waterSign = 1;
				Level_count = 0;
			}
		}
	}
}

void* ReadBuf()		// App에서 설정한 Data 받는 버퍼
{
	while(1)
	{	
		memset(Getbuf,0,MAX_BUF);
		memset(buf,0,MAX_BUF);
		if(ServerCon == 0)
		{
			memset(&server,0,sizeof(server));
			sock = 0;
			if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == -1)
			{
				ServerCon = 0;
			}
			else
			{
				ServerCon = 1;
			}
		}
		read(sock, Getbuf, 512);
		printf("%s\n",Getbuf);
		if (Getbuf != NULL)
		{
			setTemp = atoi(strtok(Getbuf,","));
		}
		if (Getbuf != NULL)
		{
			setHum = atoi(strtok(Getbuf,","));
		}
		if (Getbuf != NULL)
		{
			if (Feed_Mode == 0)
			{
				Feed_Mode = atoi(strtok(Getbuf,","));
				if(Feed_Mode == 1)
				{
					Food = 1;
					count = 0;
				}
			}
			else
			{
				Feed_Mode = atoi(strtok(Getbuf,","));
			}
		}
		if (Getbuf != NULL)
		{
			ServerCon = atoi(strtok(Getbuf,","));
		}
		sprintf(buf, "%d, %d, %d, %d, %d, %d, %d, %d, %d\n", avrData[0], avrData[3], tret1, tret2, setTemp, setHum, waterSign, Feed_Mode, Food);	// 습도랑 온도 /2해야 함, // tret은 -1이면 오류 //  1이면 정상	
		printf("%s",buf);
		if(ServerCon == 1)
		{
			write(sock, buf, strlen(buf));
		}
		delay(1500);
	}
}

// DHT-11 설정
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
	for(int k=0; k<1;k++)
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
	}
}

void Run2()
{
	for(int j=0; j<1;j++)
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
	}
}

void* average()
{	
	while(1)
	{
		Run();
		Run2();
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

			if(avrData[3]/2 > setTemp*10)
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
		tret=0;
		delay(1500);
	}	
}

int main(int n ,char *argv[])
{
	char name[64] = "user5" ;
    int status;
	wiringPiSetup();
	pinMode(pr1, OUTPUT);
	pinMode(pr2, OUTPUT);
	pinMode(pr3, OUTPUT);
	pinMode(pr4, OUTPUT); 
	digitalWrite(pr1, 1);		// 시작할 때 꺼진 상태로 시작
	digitalWrite(pr2, 1);
	digitalWrite(pr3, 1);
	digitalWrite(pr4, 1);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(PORT);
	
	pthread_create(&thread_DHT, NULL, average, NULL);
	pthread_create(&thread_Fan, NULL, Fan, NULL);
	pthread_create(&thread_Feed, NULL, FeedThr, NULL);
	pthread_create(&WaterLevelThr, NULL, WaterLevel, NULL);
	
	printf("%d\n", ServerCon);
	
	pthread_create(&ReadThr, NULL, ReadBuf, NULL);
	
	pthread_join(thread_DHT, (void **)&status);
	return 0;
}
