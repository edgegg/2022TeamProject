#include <L298NX2.h>  // 모터드라이버 라이브러리
#include <SoftwareSerial.h> // 소프트웨어 시리얼 통신 라이브러리

//-----------모터 드라이버 핀 설정----------------

const unsigned int EN_A = 6;  //왼쪽 바퀴
const unsigned int IN1_A = 2;
const unsigned int IN2_A = 3;

const unsigned int IN1_B = 4; // 오른쪽 바퀴
const unsigned int IN2_B = 5;
const unsigned int EN_B = 6;

L298NX2 motors(EN_A, IN1_A, IN2_A, EN_B, IN1_B, IN2_B);

//---------블루투스 설정---------------------

#define RXD_PIN 8 //아두이노 수신 핀
#define TXD_PIN 9 //아두이노 발신 핀

SoftwareSerial bt(RXD_PIN, TXD_PIN);  // Rx,Tx 설정 / 시리얼 통신 객체 생성

String data = ""; // 문자열 받기
int spd = 50; // 속력
int a;
int state = 5;  //현재 rc카 동작 상태 //초기 정지상태
int mode = 1; // 모드 설정 // 초기 일반모드

//---------초음파 센서 설정-------------------

#define echo 12 // echo 핀
#define trig 13 // trig 핀

long duration, cm; // 거리 변수 설정

//------------부저 설정-----------------------

#define BUZ 7

//-------------LED 설정----------------------

#define led 10

int LED_State = LOW;  // LED 상태
int bright; // LED 밝기

unsigned long timeVal = 0; // 현재 시간 값 저장
unsigned long previousVal = 0; // 이전 시간 값 저장
unsigned long LEDinterval = 1000; // LED 점등 간격

//-------------조도 센서 설정------------------

#define cds A0

int cdsvalue; // 조도센서 값


void MoveCar(int n) // 모터 동작 함수
{
  switch(n)
  {
    case 1: // 전진
      motors.setSpeed(spd);
      motors.forwardA();
      motors.forwardB();
      break;
    case 2: // 후진
      motors.setSpeed(spd);
      motors.backwardA();
      motors.backwardB();
      break;
    case 3: // 우회전
      motors.setSpeed(spd);
      motors.forwardA();
      motors.backwardB();
      break;
    case 4: // 좌회전
      motors.setSpeed(spd);
      motors.backwardA();
      motors.forwardB();
      break;
    case 5: // 정지
      motors.stop();
      break;
    default:
      motors.stop();
      break;
  }
}

void WarningCar(int n) // 모터 동작 함수
{
  switch(n)
  {
    case 2: // 후진
      motors.setSpeed(spd);
      motors.backwardA();
      motors.backwardB();
      break;
    case 5: // 정지
      motors.stop();
      break;
    default:
      motors.stop();
      break;
  }
}

void UltraSonic(void) // 초음파 센서 동작 함수
{
  digitalWrite(trig, LOW);  //HICH신호를 보내기 전에 LOW신호를 출력해서 출력을 깨끗하게
  delayMicroseconds(2);

  digitalWrite(trig, HIGH); //초음파 trig 출력 시작
  delayMicroseconds(10);

  digitalWrite(trig, LOW);  //출력 종료

  duration = pulseIn(echo, HIGH); //echo핀이 trig가 HIGH인 시간 저장 

  cm = (duration/2)/29.4; //초음파 거리를 cm로 산출
}

void LED(void)  // 조도 센서의 값에 따른 LED 동작 함수
{
  cdsvalue = analogRead(cds);
  bright = map(cdsvalue, 900, 1023, 0, 255); // 충분히 어두울때의 LED 출력값 변환
  
  if(cdsvalue > 900)  // 충분히 어두우면 LED가 켜짐
  {
    analogWrite(led, bright);  // 어두울수록 LED가 더 밝아짐
  }
  else
  {
    analogWrite(led, 0);  // 밝아서 LED꺼짐
  }
}

void WarningLED(void) // 위험시 점등 LED 동작 함수
{
  timeVal = millis(); // 현재 시간
  
  if(timeVal - previousVal >= LEDinterval)  // LEDinterval 간격으로 LED 점등
  {
    previousVal = timeVal;
    
    if(LED_State == LOW)
    {
      LED_State = HIGH;
    }
    else
    {
      LED_State = LOW;
    }
    digitalWrite(led, LED_State);
  }
}

void setup()
{
  bt.begin(9600); // bt 시리얼 통신 시작
  pinMode(echo, INPUT); // echo 입력
  pinMode(trig, OUTPUT);  //trig 출력
  pinMode(BUZ, OUTPUT); // 부저 출력
  pinMode(led, OUTPUT); // LED 출력
  pinMode(cds, INPUT);  // 조도센서 입력

  motors.setSpeed(spd); // 초기 속도 설정
}


void loop()
{
  if(bt.available())
  {
    data = bt.readStringUntil('e'); // e직전까지 읽기
    a = data.toInt();               // data를 정수형으로
    if(a>=50)
    {
      spd = a;
    }
    else if(a == 6)
    {
      mode = 1; // 일반모드
    }
    else if(a == 7)
    {
      mode = 2; // 안전모드
    }
    else
    {
      state = a;
    }
  }

  if(mode == 1) // 일반 모드
  {
    LED();  // led 동작
    noTone(BUZ);
    MoveCar(state); //모터 동작
  }

  if(mode == 2) // 안전모드
  {
    UltraSonic(); // 초음파 센서 동작
    if(cm < 10)
    {
      WarningCar(state); //위험시 모터 동작
      tone(BUZ, 1000);
      WarningLED(); //위험시 LED 동작
    }
    else
    {
      MoveCar(state); //모터 동작
      noTone(BUZ);
      LED();  // led 동작
    }
  }
}
