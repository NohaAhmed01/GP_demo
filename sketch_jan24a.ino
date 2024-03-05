#include <Arduino_FreeRTOS.h>
#include <semphr.h>

const int IN1 = 13;
const int IN2 = 12;
const int EN1 = 11;
const int IN3 = 10;
const int IN4 = 9;
const int EN2 = 8;

const int echoPin = 6;
const int trigPin = 7;

boolean isACCActive = false;

const int yellowLED = 15;
const int redLED = 14;

float duration;
int motorSpeed, USdistance;

void ACC(void *param);
void Collision_Warning(void *param);

TaskHandle_t ACC_Handle;
TaskHandle_t CollisionWarning_Handle;

void setup() {
  // put your setup code here, to run once
    Serial.begin(9600);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    pinMode(yellowLED,OUTPUT);
    pinMode(redLED,OUTPUT);    
  
    pinMode(EN1, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(EN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    pinMode(18, INPUT_PULLUP);

    xTaskCreate(ACC,"ACC",100,NULL,1,&ACC_Handle);
    xTaskCreate(Collision_Warning,"CW",100,NULL,1,&CollisionWarning_Handle);

    attachInterrupt(digitalPinToInterrupt(18), digitalPinInterruptHandler, CHANGE);
}

void loop(){}

void digitalPinInterruptHandler() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  isACCActive = !isACCActive;
  vTaskNotifyGiveFromISR(ACC_Handle, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    taskYIELD();
  }
}


void ACC(void *param)
{
  (void) param;

  
  while (1) {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) && isACCActive) {
      int space = Read_usDistance(trigPin,echoPin);
     if(space>80)
      {
        Serial.println(space);
        Adj_speed(space); 
      }   
     else
      {
        Serial.println(space);      
        AEB();
      }  
    } 
    vTaskDelay(1000/portTICK_PERIOD_MS);  
  }  
    
}   
void Collision_Warning(void *param)
{
  (void) param;

    while (1) {  
    if(Read_usDistance(trigPin,echoPin)<=80 && Read_usDistance(trigPin,echoPin)>=50)
    {
      Serial.println("careful! small distance.");
      digitalWrite(yellowLED,HIGH);
      digitalWrite(redLED,LOW);
    }else if(Read_usDistance(trigPin,echoPin)<50)
    {
      Serial.println("Stop!! dangerous distance!! ");
      digitalWrite(yellowLED,LOW);
      digitalWrite(redLED,HIGH);
    }else{
      Serial.println("safe space ");
      digitalWrite(yellowLED,LOW);
      digitalWrite(redLED,LOW);
    }
    vTaskDelay(1000/portTICK_PERIOD_MS); 
    }
}
void AEB()
{
   Stop_Motors();
}
void Adj_speed(int distance)
{  
  if(distance>=80){
   motorSpeed=map(distance,50,80,0,255);
   motorSpeed = motorSpeed * 2;
   motorSpeed= constrain(motorSpeed,0,255);    
  }else if(distance>50&&distance<80)
  {
   motorSpeed=distance;  
  }    
      analogWrite(EN1,motorSpeed);
      analogWrite(EN2,motorSpeed);   
      
      digitalWrite(IN1,HIGH);
      digitalWrite(IN2,LOW);
      digitalWrite(IN3,HIGH);
      digitalWrite(IN4,LOW);  
}
void Stop_Motors()
{
   motorSpeed=0;
   analogWrite(EN1,motorSpeed);
   analogWrite(EN2,motorSpeed);
    
}
 int Read_usDistance(int trigPin,int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  USdistance = (duration*.0343)/2;
  delay(50);
  return USdistance;  
}


