# IOT기반의 파충류 사육장  
### Kosta Final Project_IOT기반의 파충류 사육장   
**1.개발기간** 
|일정|11/5|11/12|11/19|11/26|12/3|12/10|12/15|
|-----|---|---|---|---|---|---|---|
|기획|O|||||||
|분석|O|O||||||
|설계||O|O|||||
|개발||O|O|O|O|||
|테스트|||||O|O||
|프로젝트 안정화 및 발표|||||O|O|O| 

**2.개발인원** : 6명(김강현, 박성범, 박지호, 송연석, 오정민, 조석훈)

**3.역할분담** 
|역할|이름|담당업무|
|-----|---|---|
|HW&Sensor|김강현|PI-Server 통신, 센서 인식 및 제어|
|HW&Sensor|오정민|센서 인식 및 제어|
|HW& Sensor|조석훈|3D Print 제작, 센서 인식 및 제어|
|SW|박성범|카메라 스트리밍|
|SW|박지호|안드로이드 앱 개발|
|SW|송연석|서버 및 DB 구축|

**4.개발환경**
+ OS
Windows 10
Raspbian Linux
Android

+ 개발도구
Visual Studio code
Visual Studio 2019
Android Studio
Geany
Thinkercad

+ 개발언어
C/C++
C#
Android Java
Python

**5.프로젝트 개요**

‘나는 편한데, 우리집 뚱이는?’

IoT 기반의 도마뱀 사육장 프로젝트는 이 의문으로 시작되었다. 기술의 진보에 따라 우리 주변의 많은 것들이 변화해 왔는데, 그 중 하나인 주거 환경의 변화는 사람들에게 더 크게 다가온다고 할 수 있다. 사람들은 이제 온기 없는 방 안에 들어서지 않아도 되며, 깜빡 잊고 끄지 못한 보일러를 확인하기 위해 집으로 다시 들어가지 않아도 된다. 냉장고 화면으로 장을 보고, 원격으로 전등을 제어하는 일이 가능한 것이 작금의 세상이다. IoT 기술에 기반한 스마트 홈은 우리의 삶을 더 편안하게 만들어 주고 있다.

그러나 ‘스마트 사육장’이라는 단어는 꽤나 이질적이다. 반려동물 관련물품의 시장규모는 증가세를 보여 왔지만, 아직 반려동물을 위한 사육장에 IoT 기술이 적용된 사례는 찾기 힘들었다. 핵가족화의 심화로 반려동물이 가족으로 받아들여지는 경우가 많아지는 가운데, 이들을 좀 더 편안하게 해줄 수 있는 집이 필요하겠다는 생각에 반려동물을 위한 스마트 홈을 개발하고자 하였다.

개나 고양이와 같이 자유로이 자신의 활동반경을 조정할 수 있는 동물보다는 도마뱀이나 거북이처럼 사육장 내부에서 생활하는 파충류에게 이와 같은 사육장이 더 필요할 것이라 생각했고, 그 중에서도 사육을 위해 온도와 습도를 조절해 줄 필요가 있는 도마뱀이 프로젝트 진행을 위해서 가장 적합하다고 판단하였다.

외온성 변온동물인 도마뱀의 특성상 주위 환경 및 온도변화에 매우 민감하다. 반려동물로 도마뱀을 키울 때 불가피하게 며칠간 집을 비워야 할 상황이 온다면, 도마뱀이 원하는 환경을 맞춰주지 못할 가능성이 높다. 도마뱀 사육자에게 가장 큰 불편함이 될 이런 부분을 해소할 수 있는 스마트 사육장을 개발하는 것이 목적이다.

**6.기능 정의**

+ 기능 개요

![image](https://user-images.githubusercontent.com/88314920/145770127-74f88189-7235-45d5-8b14-ea13d459d37e.png)

+ 온습도 자동제어

![image](https://user-images.githubusercontent.com/88314920/145770190-e6c6e12d-3df2-44a9-bf45-cd84c5a2e996.png)

+ 온습도 수동제어

![image](https://user-images.githubusercontent.com/88314920/145770232-49a23ffa-bafa-46cb-af1b-ba4ce846d6cf.png)

+ 자동 급여, 급수

![image](https://user-images.githubusercontent.com/88314920/145770263-e2db7d24-4202-4617-93bf-5f75f7a60688.png)

+ 수동 급여

![image](https://user-images.githubusercontent.com/88314920/145770284-14021a5d-ca11-4e7d-9f95-800e3f6adc46.png)
