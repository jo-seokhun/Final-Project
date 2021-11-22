## 1. 급양기

---

### 1) 급양기 설계

---

- <최초 설계> 스텝모터를 활용한 자동 급양기 구현 , 일정 시간이 되면 통이 돌아가면서 먹이(귀뚜라미) 떨어뜨려 주기
- 급양기 설계

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/7195344c-4cdb-4ed1-9c2e-445ed58cd7c6/Untitled.png)

- <문제 발생> 스텝모터와 연결하여 구동 시 마찰과 유격 문제로 의도대로 작동 실패
- <1차 문제 해결> 3D 프린터로 제작(ThikerCad, Cura, Sindoh 3DWOX)
    
    ![급양기.png](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/8c3450ca-5c46-4987-98c5-c0535bf4ae5c/급양기.png)
    
     
    

### 2) 급양기 원리

---

- 기본 원리
    - 급양기 회전부의 칸 수가 총 8칸이고 회전부 하판은 45도 각도로 한 칸 크기 정도 구멍이 뚫려있음
    - 스텝모터와 회전부 고정판을 연결하여 특정시간이 되면 급양기를 45*n도(n= 특정시간이 되었을 때의 count 수, count는 특정시간이 될 때마다 +1해줌)만큼 돌려줌
- 스텝모터 구동원리
    
    ![스텝모터 구동원리.png](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/3865a529-a2c7-4538-afba-9a2fc1c6c8f8/스텝모터_구동원리.png)
    
    S극 전자석 A,B,C,D에 전류를 흘러주면 자기력이 발생하여 중앙에 있는 N극 전자석이 전류에 따른 자기력을 따라 정렬
    
    예를들어 표의 2번처럼 A와B에 전류를 흘려준다면 두번째 그림의 2번처럼 N극 전자석이 A와B 사이에 위치함
    
    이런식으로 전류를 360도 돌려주면 모터는 360도 회전, 특정각도를 원하면 전류를 바꿔주면 된다.
    

### 3) 급양기 코드

---

```c
#include <stdio.h>
#include <wiringPi.h>
#include <time.h>

#define IN1 21 #define IN2 22 #define IN3 23 #define IN4 24       
#define speed 2 #define STEP 63

int count = 0;

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

int main()
{
	wiringPiSetup();
	
	time_t current;
	struct tm* t ;
	
	
	
	
	while(1)
	{
		current = time(NULL);
		t = localtime(&current);
		//시간이 00시00분00초일때 
		if(t->tm_hour==0)
		{
			if(t->tm_min==0)
			{
				if(t->tm_sec==0)
				{
					count++;	//한번돌려줄때마다 count 1증가
					for(int i = count ; i<count+1;i++)
					{
						//급양기를 45*n도(n= 특정시간이 되었을 때의 count 수, count는 특정시간이 될 때마다 +1해줌)만큼 돌려줌
						forward(speed,STEP*count);
						delay(2000);
						backward(speed,STEP*count);
						delay(2000);
						if(count==8) break;	//7번만 돌아야하기 때문에 -> 급양기의 칸이 7칸만 사용가능
					}
				}
			}
		}
		
	}
		
	return 0;
}
```

## 2. 급수기

---

### 1) 급수기 설계

---

- <최초 설계> 물의 압력 차를 활용한 자동 급수기 구현 / 도마뱀이 물을 마셔서 물의 높이가 낮아지면 지정된 위치(두 번째 구멍)까지 자동으로 물 급수

[병아리 자작 물통 만들기 / 페트병으로 손쉽게 자동급수기 만들기](https://littlefunny.tistory.com/121)

- <문제 발생> 물의 급수를 해주는 물통과 관의 수압 차 때문에 일정 높이까지 물을 채워주는 형식이 불가능
- <1차 문제 해결> 워터 펌프를 이용한 물의 급수_일정 시간(1일 1회, 급양기와 일치)이 되면 자동으로 워터펌프를 사용하여 물 급수

### 2) 급수기 원리

---

- 기본 원리
    - 워터펌프로 일정시간이 되면 도마뱀이 물 먹는 그릇에 물을 넣어준다
- 워터펌프 원리
    1. 흡입구로 들어간 물이 모터에 의해 회전
    2. 모터 안에 물은 원심력에 의해 압력 증가 
    3. 원심력에 의해서 회전 바깥쪽 물의 압력이 중심의 물의 압력보다 커짐 
    4. 결과적으로 물의 압력이 작은 곳(중심)에서 압력이 높은 곳(바깥쪽)으로 이동
    
    ![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/53d94d3a-d469-4754-9fbd-272e90fe5796/Untitled.png)
    

 

### 3) 급수기 코드

---

```c
#include <stdio.h>
#include <wiringPi.h>
#include <time.h>

#define p1 22 
#define interval 2 

void Run()
{
	//릴레이모듈에서 Low : 스위치 ON / HIGH : 스위치 OFF
	digitalWrite(p1,LOW);
	delay(interval*1000);   //워터펌프 가동 시간
	digitalWrite(p1,HIGH);
}

int main()
{
	wiringPiSetup();
	pinMode(p1, OUTPUT);
	time_t current;
	struct tm* t ;
	
	
	while(1)
	{
		current = time(NULL);
		t = localtime(&current);
		//00시00분00초(급양기와 동일)가 되면 워터펌프 가동
		if(t->tm_hour==0)
		{
			if(t->tm_min==0)
			{
				if(t->tm_sec==0)
				{
					Run();
				}
			}
		}
	}
	return 0;
}
```

## 3. 온습도 센서 DHT11

---

### 1) DHT11 설계

---

- <최초 설계> DHT센서 2개를 양쪽 끝에 배치하여 각각의 온도, 습도 값을 받아오고 평균 값을 구해 저장 → 스팟 램프,가습기, Fan과 연결하여 전원 on,off
- <문제> - DHT센서의 값을 끊임없이 받아와야 하는데 최대 50회(1분 가량) 받아오면 어떤 이유인지 프로그램이 멈추며 더이상 받아지지 않음
- <1차 문제 해결>  - DHT-브레드보드, DHT-전선 사이의 틈이 생겨 신호를 적절히 못 받아온다고 판단
1. DHT-전선을 납땜 하여 틈 없이 밀착 시켜줌
2. 여전히 1분 이내로 프로그램이 멈추며 값을 받아오지 않음
- <2차 문제 해결> - 타이밍 문제(프로그램은 최초 High에서 Low가 오길 기다리는데, 센서는 Low를 넘어 다음 High에 가 있어서 만날 수 없음)
1. 무한Loop코드를 수정
    
    1) GetRespons, ReadData 함수에서 High와 Low인 상태의 대기 시간이 일정 시간 이상 길어질 경우
    
    2) 음수의 값을 받고 
    
    3) Run함수에서 해당 While(for)문 에서Continue
    
- <스팟램프와 연결> - average 함수에서 **(avrData[3]/2)** 즉 두 온도의 합의 평균이 250(25도)보다 작으면 스팟램프를 키고 커지면 끔
- <가습기와 연결> - average 함수에서  (avrData[0]/2) 즉 습도 정수부의 평균이 55(습도 55%) 보다 작으면 가습기를 키고 커지면 끔
- <Fan과 연결>
    1. **(avrData[0]/2)** 즉 습도 정수부의 평균이 55(습도55%) 보다 크면 Fansta 값을 1로 줌
    2. **(avrData[3]/2)** 즉 두 온도의 합의 평균이 260(26도)보다 크면 Fansta 값을 1로 줌
    3. Fansta의 값이 1이 되면 while loop에서 벗어나 Fan Thread를 돌려줌
    4. Fan Thread에서 2.5초간 Fan을 켜주고 15초동안 꺼줌
        1. 이유 1] Thread가 도는동안 while loop가 돌지 않기때문에 짧은 시간만 Fan을 돌려준다
        2. 이유 2]  Fan이 짧은 시간 돌아도 습도가 순식간에 내려가기 때문 짧은 시간만 Fan을 돌려준다 

### 2) DHT11 코드

---

```c
#include <stdio.h> #include <stdlib.h> #include <string.h> #include <unistd.h> #include <netinet/in.h> #include <wiringPi.h> #include <wiringPiI2C.h>
#include <softPwm.h> #include <pthread.h> #include <unistd.h>
#define MAX_BUF 1024 #define THRASHOLD 40 #define MAX_BIT 40
#define p1 15		//온도센서1
#define p2 16		//온도센서2
#define pr1 22		//펌프
#define pr2 23		//가습기
#define pr3 24		//전구
#define pr4 25		//팬

int Data[10]; int Data2[10]; int avrData[10];
int sock;

int temp = 0; int co = 0; int co2 = 0;

int tret = 0; int tret1 = 1; int tret2 = 1; int fanSta = 0;
char buf[1024];

int PORT = 9000; char IP[25] = "192.168.2.58";

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
	//printf("<평균>  습도 :  %d.%d%% 온도 : %d℃ Checksum : %d", avrData[0], avrData[1], avrData[3], avrData[4]);

	if (tret==2)
	{
		printf("...OK\n");
		
		//습도평균이 55보다 높으면 가습기는 끄고 Fan은 틀고(fanSta =1) 
		**if(avrData[0]/2 > 55)
		{
			digitalWrite(pr2,1);
			fanSta = 1;
		}
		else
		{
			digitalWrite(pr2,0);
		}**

		//온도평균이 25도보다 높으면 전구 끔
		**if(avrData[3]/2 > 250)
		{
			digitalWrite(pr3,1);
		}
		else
		{
			digitalWrite(pr3,0);
		}**
		
		//온도평균이 26도보다 높으면 Fan을 틈(fanSta=1)
		**if(avrData[3]/2 > 260)
		{
			fanSta = 1;
		}**
	}
	else 
	{
		printf("...ERROR!\n");
	} 
	temp = avrData[3];

	memset(buf,0,MAX_BUF);
	sprintf(buf, ":1,%d,%d,%d,%d", avrData[0], avrData[3], tret1, tret2);	// 습도랑 온도 /2해야 함, // tret은 -1이면 오류 //  1이면 정상	
	//write(sock, buf, strlen(buf));
	//printf("%s", buf);//sprintf 확인용
}

int main(int n ,char *argv[])
{
	char name[64] = "user5" ;
	struct sockaddr_in server;
	pthread_t thread_Fan;
	wiringPiSetup();
	pinMode(pr1, OUTPUT);
	pinMode(pr2, OUTPUT);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(PORT);
	
	pthread_create(&thread_Fan, NULL, Fan, NULL);
	//connect(sock, (struct sockaddr*)&server, sizeof(server));
	
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
```

## 4. 수위 센서

---

### 1) 수위센서 설계

---

- <최초 설계>가습기와 급수기의 필요한 물을 충만 하는 물통의 물이 부족해지면 어플리케이션에서 사육자에게 알림을 주는 형식 고안
    1. 아두이노 수위 센서를 물통에 부착하여 수위를 측정
    2. 수위를 백분율로 변환
    3. 수위의 백분율이 한 자리가 지속되면(1초 단위 30회 이상) 물통의 수위가 낮아졌다는 뜻이므로 서버에 정보 전송
- <문제> 라즈베리파이의 GPIO는 아두이노와 다르게 디지털 입력만 가능 → ADC 내장되어있지 않음 → analogread()함수를 사용할 수 없음
- <문제해결> 별도의 ADC장치를 사용하여 문제 해결

### 2) 수위센서 원리

---

- 수위센서 원리
    
    ![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/dad4a7f3-8e28-45c5-9b8f-96a4d5b45a63/Untitled.png)
    
    - 세로로 길게 있는 전극 부분이 수위에 따라 저항이 변함
    - 전압이 일정하기 때문에 저항이 변함에 따라 전류도 변함
    - 즉 수위가 증가하면 저항이 감소하고 전류 증가, 수위가 감소하면 저항이 증가하고 전류 감소

### 3) 수위센서 코드

---

```c
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>

//수위를 백분율로 변환해주는 함수
int level(int n)
{
	int resist=n*100/255;
	return resist;
}

int main(int n, char*v[])
{
	int fd, data;
	int ch = 2;
	int count=0;
	fd = wiringPiI2CSetup(0x48);		
	
	wiringPiI2CWrite(fd, 2);	
	
	
	while(1)
	{
		data = wiringPiI2CRead(fd);
		printf("현재 수위는 %d %% 입니다.\n",level(data));
		delay(100);
		
		if(level(data)<10) count++;
		if(count==30)
		{
			printf("물 보충 필요\n");
			count=0;
			
		}
	}
	
}
```
