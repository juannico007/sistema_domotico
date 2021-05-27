//Slave code
#include<Wire.h>
#include <Servo.h>

#define l_s 7
#define l_e 11
#define l_h 12
#define fan 8
#define servo 9
#define servo2 10

#define R 3
#define G 5
#define B 6

#define DHTPIN 13
#define AUTO 100 
#define cine 110
#define seguro 111
#define fiesta 112

//
//const float maxT = 23.5;
//const int maxL = 1000;

int ldrVal; 
int pin;
int fiesta_activo = 0;
Servo myservo;
Servo myservo2;

void setup() {
  Wire.begin(1);
  Serial.begin(9600);
  pinMode(l_s, OUTPUT);
  pinMode(l_h, OUTPUT);
  pinMode(l_e, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(fan, OUTPUT);
  myservo.attach(9);
  myservo2.attach(10);
  Wire.onReceive(receiveEvent);
  Serial.println("a");
}

void fiesta_f(){
  if(fiesta_activo == 1){
      double start = millis();
      digitalWrite(R,HIGH);
      digitalWrite(G,HIGH);
      digitalWrite(B,HIGH);
      delay(500);
      digitalWrite(R,LOW);
      digitalWrite(G,HIGH);
      digitalWrite(B,LOW);
      delay(500);
      digitalWrite(R,LOW);
      digitalWrite(G,LOW);
      digitalWrite(B,HIGH);
      delay(500);
      digitalWrite(R,LOW);
      digitalWrite(G,HIGH);
      digitalWrite(B,HIGH);
      delay(500);
      digitalWrite(R,HIGH);
      digitalWrite(G,LOW);
      digitalWrite(B,HIGH);
      delay(500);
      digitalWrite(R,HIGH);
      digitalWrite(G,HIGH);
      digitalWrite(B,LOW);
      delay(500);
      digitalWrite(R,HIGH);
      digitalWrite(G,LOW);
      digitalWrite(B,LOW);
      delay(500);
      digitalWrite(R,LOW);
      digitalWrite(G,LOW);
      digitalWrite(B,LOW);
      delay(500);
      double end = millis();
      Serial.println(end-start);
  }
}

void receiveEvent(){
  int n = Wire.available()/2;
  Serial.print("received info for ");
  Serial.print(n);
  Serial.println("pins");
  int pins[n];
  int states[n];
  for(int i = 0; i < n; i++){
    pins[i] = Wire.read();
    states[i] = Wire.read();
  }
  
  for(int i = 0; i < n; i++){
    Serial.println(states[i]);
    Serial.println(pins[i]);
    
    if(pins[i] == 9){
      myservo.write(states[i++]);
      myservo2.write(states[i]);
    }else{
      digitalWrite(pins[i], states[i]);
    }
    
    //escenarios modos 
    if(pins[i] == cine){
      //Serial.println("Entra cine");
      digitalWrite(l_s, 0);
      digitalWrite(l_e, 0);
      digitalWrite(l_h, 0);
      myservo.write(0);
      myservo2.write(180);
      
    }if(pins[i] == seguro){
      myservo.write(0);
      myservo2.write(180);
    }
    if(pins[i] == fiesta){
      Serial.println("Entra fiesta");
      digitalWrite(l_s, 0);
      digitalWrite(l_e, 0);
      digitalWrite(l_h, 0);
      myservo.write(0);
      myservo2.write(0);
      fiesta_activo = 1;
      Serial.println("b");
    }else{
      fiesta_activo = 0;
    }
    
    Serial.print("pin ");
    Serial.print(pins[i]);
    Serial.print(" in state ");
    Serial.println(states[i]);
  }
}


void loop() {
  fiesta_f();
  delay(1000);
}
