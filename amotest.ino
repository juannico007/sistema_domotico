#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Blynk.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

/*
 * Todas las variables seguidas de una E corresponden a la entrada
 * Todas las variables seguidas de una H corresponden a la habitación
 * Todas las variables seguidas de una S corresponden a la sala
 * Para los servos, una terminacion de I es izquierda una D es derecha
 */


//Pines virtuales de actuadores
//Luces
#define VPIN_BUTTON_LS    V1
#define VPIN_BUTTON_LH    V9
#define VPIN_BUTTON_LE    V10
//Ventilador
#define VPIN_BUTTON_F     V2
//Cortinas
#define VPIN_SERVO        V7

//Pines virtuales de los modos
#define VPIN_BUTTON_C     V3 //auto o manual
#define VPIN_MODO         V8

//Pines virtuales de la muestra de variables
#define VPIN_HUMIDITY     V5
#define VPIN_TEMPERATURE  V6


//Pines de actuadores
//Luces
#define pinLedE           11
#define pinLedH           12
#define pinLedS           7
//Ventilador
#define pinFan            8
//Cortinas
#define pinServoI         9
#define pinServoD         10
//Alarma
#define buzzer            33


#define pinR              3
#define pinG              5
#define pinB              6

//Mensajes a enviar por cada modo
#define autom             100
#define cine              110
#define seguro            111
#define fiesta            112


//Pines de sensores de toma de variables
//Temperatura y humedad
#define DHTPIN            13
//Intensidad luminica
#define LDR_PINS          32
#define LDR_PINE          34
#define LDR_PINH          35
//Distancia, cada ultrasonido tiene 2 pines, disp-trig y ent-echo
#define disp_ultraS       26   
#define ent_ultraS        25    
#define disp_ultraH       14   
#define ent_ultraH        27    
#define disp_ultraE       18   
#define ent_ultraE        19    

//Definicion del tipo de sensor DHT a usar
#define DHTTYPE           DHT11  
//Inicializacion del sensor dht11
DHT dht(DHTPIN, DHTTYPE);   

//Variables globales
float temperature = 0;
float humidity = 0;
int ldrValS;
int ldrValE;
int ldrValH;
float distanciaS;
float distanciaH;
float distanciaE;
long tiempoE;
long tiempoS;
long tiempoH;
int servo_stateI = 0;
int servo_stateD = 180;

//Variables correspondientes al estado de los modos
int security;
int switchMode;

//Limites de temperatura y luz
const float maxT = 23.5;
const int maxL = 800;

//Variables para controlar adecuadamente el buzzer desde el esp32
int canal=5;          // canal por donde se establece la comuncacion
int frecuencia=700;   //Sonido (agudo, grave)
int ciclo=8;          // ciclo de trabajo

// Variables para el manejo del tiempo
int hours;
int minutes;
int seconds;

//Codigo de autenticacion para la conexion con blynk
char auth[] = "envom_tX8Wm5f4RQhZtImbiXby06WDTo";

//Usuario y contraseña para la conexion a internet
char ssid[] = /*"FAMILIA QUINTERO"*/ "HUAWEI P30 lite";
char pass[] = /*"familiaquintero1"*/ "08e7130794a2";

int last_ID = 0;
int secure_timer_ID;
int fiesta_timer_ID;
int cine_timer_ID;

BlynkTimer timer;
WidgetRTC rtc;


//Funcion para la toma de tiempo
void getCurrentTime(int &hours, int &minutes, int &seconds){
  hours = hour();
  minutes = minute();
  seconds = second();
}

//Funcion para automatizar las persianas de la sala en funcion de la hora
void AjustarPersiana(int hours, int minutes){
  //Si esta entre las 6 de la mañana y las 6 de la tarde las cortinas estan subidas, sino bajan
  if (hours >= 6) {
    servo_stateI = 180;
    servo_stateD = 0;
  }else if (hours >= 18){
    servo_stateI = 0;
    servo_stateD = 180;
  }else if (hours < 6){
    servo_stateI = 0;
    servo_stateD = 180;
  }
  //actualizo el slider en la aplicacion
  Blynk.virtualWrite(V7, servo_stateI);
  
  Wire.beginTransmission(1);
  Serial.println("Sending info left servo");
  Wire.write(pinServoI);
  Wire.write(servo_stateI);
  Wire.write(pinServoD);
  Wire.write(servo_stateD);
  Wire.endTransmission();
}

BLYNK_CONNECTED() {
  // Toma el ultimo estado de los pines de la aplicacion
  Blynk.syncVirtual(VPIN_BUTTON_F);
  Blynk.syncVirtual(VPIN_BUTTON_LS);
  Blynk.syncVirtual(VPIN_BUTTON_LH);
  Blynk.syncVirtual(VPIN_BUTTON_LE);
  Blynk.syncVirtual(VPIN_BUTTON_C);
  Blynk.syncVirtual(VPIN_MODO);
  // Synchronize time on connection
  // rtc.begin();
}

//Toma del estado de las variables
void readSensor(){
  //Iluminacion
  ldrValS = analogRead(LDR_PINS);
  ldrValE = analogRead(LDR_PINE);
  ldrValH = analogRead(LDR_PINH);
  Serial.println(ldrValH);

  //Temperatura y humedad
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
  if (isnan(h) || isnan(t)) {
    Serial.println("Falla de conexion con el sensor DHT!");
    return;
  }
  else {
    humidity = h;
    temperature = t;
  }

}

//Envio de temperatura y humedad a la aplicacion
void sendSensor(){
  Serial.println("a morir");
  readSensor();
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature);
}

//Lectura del boton de luz de la sala y actualización de actuadores
BLYNK_WRITE(VPIN_BUTTON_LS) {
  int state_l = param.asInt();
  Wire.beginTransmission(1);
  Serial.println("Sending info led_s");
  Wire.write(pinLedS);
  Wire.write(state_l);
  Wire.endTransmission();
}

//Lectura del boton de luz de la habitacion y actualización de actuadores
BLYNK_WRITE(VPIN_BUTTON_LH) {
  int state_lh = param.asInt();
  Wire.beginTransmission(1);
  Serial.println("Sending info led_h");
  Wire.write(pinLedH);
  Wire.write(state_lh);
  Wire.endTransmission();
}

//Lectura del boton de luz de la entrada y actualización de actuadores
BLYNK_WRITE(VPIN_BUTTON_LE) {
  int state_le = param.asInt();
  Wire.beginTransmission(1);
  Serial.println("Sending info led_e");
  Wire.write(pinLedE);
  Wire.write(state_le);
  Wire.endTransmission();
}

//Lectura del boton del ventilador y actualización de actuadores
BLYNK_WRITE(VPIN_BUTTON_F) {
  int state_f = param.asInt();
  Wire.beginTransmission(1);
  Serial.println("Sending info fan");
  Wire.write(pinFan);
  Wire.write(state_f);
  Wire.endTransmission();
}

//Lectura del slider de las persianas y actualizacion de actuadores
BLYNK_WRITE(VPIN_SERVO) {
  int state_s = param.asInt();
  Wire.beginTransmission(1);
  Serial.println("Sending info servos");
  Wire.write(pinServoI);
  Wire.write(state_s);
  Wire.write(pinServoD);
  Wire.write(180-state_s);
  Wire.endTransmission();
}

//Lectura del boton de seleccion de modo automatico y manual
BLYNK_WRITE(VPIN_BUTTON_C) {
  switchMode = param.asInt();
  Wire.beginTransmission(1);
  Serial.println("Sending info mode");
  Wire.write(autom);
  Wire.write(switchMode);
  Wire.endTransmission();
  BLYNK_WRITE(VPIN_MODO);
}

//Toma de las distancias en los espacios de la casa
void sonido(){
  //En la entrada
  digitalWrite(disp_ultraE, HIGH);
  delayMicroseconds(5);
  digitalWrite(disp_ultraE, LOW);
  tiempoE = (pulseIn(ent_ultraE, HIGH)/2);
  distanciaE = float(tiempoE * 0.0340);
  
  digitalWrite(disp_ultraS, HIGH);
  delayMicroseconds(5);
  digitalWrite(disp_ultraS, LOW);
  tiempoS = (pulseIn(ent_ultraS, HIGH)/2);
  distanciaS = float(tiempoS * 0.0340);
  
  digitalWrite(disp_ultraH, HIGH);
  delayMicroseconds(5);
  digitalWrite(disp_ultraH, LOW);
  tiempoH = (pulseIn(ent_ultraH, HIGH)/2);
  distanciaH = float(tiempoH * 0.0340);
}


//Lectura del menu de modos y ejecucion del mismo
BLYNK_WRITE(VPIN_MODO){
  switch(param.asInt()){
      
    // Modo en casa, desactiva la seguridad
    case 1: 
      timer.disable(last_ID);
      Serial.println("Modo en casa seleccionado");
      security = 0;
      break;
        
    // Modo Cine, apaga luces y baja persianas
    case 2:
      timer.disable(last_ID);
      timer.enable(cine_timer_ID);
      last_ID = cine_timer_ID;
      break;
        
    // Modo Fiesta
    case 3: 
      timer.disable(last_ID);
      timer.enable(fiesta_timer_ID);
      last_ID = fiesta_timer_ID;
      break;
        
    // Modo Seguro, detecta movimiento en la entrada para disparar la alarma y baja la persiana
    case 4: 
      timer.disable(last_ID);
      timer.enable(secure_timer_ID);
      last_ID = secure_timer_ID;
      break;
  
    //Por default estara en modo casa
    default:
      timer.disable(last_ID);
      Serial.println("Modo en casa seleccionado");
      security = 0;
      break;
  }
  
}

void cine_f(){
  if(switchMode == 1){
    security = 2;
    Serial.println("Modo Cine seleccionado");
    Wire.beginTransmission(1);
    Serial.println("Sending info modo cine");
    Wire.write(cine);
    Wire.write(0);
    Wire.endTransmission();
        
    Blynk.virtualWrite(VPIN_BUTTON_LS, 0);
    Blynk.virtualWrite(VPIN_BUTTON_LH, 0);
    Blynk.virtualWrite(VPIN_BUTTON_LE, 0);
    Blynk.virtualWrite(VPIN_SERVO, 0);
  }
}

void secure_f(){
  if(switchMode == 1){
    Serial.println("Modo seguro seleccionado");
    security = 1;
    //distancia menor a la del piso
    if(distanciaE<12 && security == 1){ 
      Blynk.notify("Hay un intruso");
      Serial.println("Intruso!");
      ledcWriteTone(canal, 500);
      delay(2000);
      ledcWriteTone(canal, 0);
    }
    Wire.beginTransmission(1);
    Serial.println("Sending info modo seguro");
    Wire.write(seguro);
    Wire.write(0);
    Wire.endTransmission();
    Blynk.virtualWrite(VPIN_SERVO, 0);
  }
}

void fiesta_f(){
  if(switchMode == 1){
    security = 2;
    Serial.println("Modo fiesta seleccionado");
    Wire.beginTransmission(1);
    Serial.println("Sending info modo fiesta");
    Wire.write(fiesta);
    Wire.write(0);
    Wire.endTransmission();
    
    Blynk.virtualWrite(VPIN_BUTTON_LS, 0);
    Blynk.virtualWrite(VPIN_BUTTON_LH, 0);
    Blynk.virtualWrite(VPIN_BUTTON_LE, 0);
    Blynk.virtualWrite(VPIN_SERVO, 0);
  }
}


void mode(){
  //Si se encuentra en modo automatico y no esta en modo seguro
  if(switchMode == 1 && security == 0){ 
    AjustarPersiana(hours, minutes);
    
    //Si detecta una persona en la sala
    if(distanciaS < 15){
      if(ldrValS <= maxL){
        Blynk.virtualWrite(VPIN_BUTTON_LS, 1);
        Wire.beginTransmission(1);
        Serial.println("Low light");
        Wire.write(pinLedS);
        Wire.write(1);
        Wire.endTransmission();
  
  
      }else if(ldrValS > maxL){
        Blynk.virtualWrite(VPIN_BUTTON_LS, 0);
        Wire.beginTransmission(1);
        Serial.println("High light");
        Wire.write(pinLedS);
        Wire.write(0);
        Wire.endTransmission();
      }
  
      if(temperature  >= maxT){
        Blynk.virtualWrite(VPIN_BUTTON_F, 1);
        Wire.beginTransmission(1);
        Serial.println("High temp");
        Wire.write(pinFan);
        Wire.write(1);
        Wire.endTransmission();
  
  
      }else if(temperature < maxT){
        Blynk.virtualWrite(VPIN_BUTTON_F, 0);
        Wire.beginTransmission(1);
        Serial.println("Low temp");
        Wire.write(pinFan);
        Wire.write(0);
        Wire.endTransmission();
      }
    }
    
    //Si detecta una persona en la entrada
    if(distanciaE < 12){
      if(ldrValE <= maxL){
        Blynk.virtualWrite(VPIN_BUTTON_LE, 1);
        Wire.beginTransmission(1);
        Serial.println("Low light");
        Wire.write(pinLedE);
        Wire.write(1);
        Wire.endTransmission();
  
  
      }else if(ldrValE > maxL){
        Blynk.virtualWrite(VPIN_BUTTON_LE, 0);
        Wire.beginTransmission(1);
        Serial.println("High light");
        Wire.write(pinLedE);
        Wire.write(0);
        Wire.endTransmission();
      }
    }

    //Si detecta una persona en la habitacion
    if(distanciaH < 20){
      Serial.println("h");
      if(ldrValH <= maxL){
        Blynk.virtualWrite(VPIN_BUTTON_LH, 1);
        Wire.beginTransmission(1);
        Serial.println("Low light");
        Wire.write(pinLedH);
        Wire.write(1);
        Wire.endTransmission();
  
  
      }else if(ldrValH > maxL){
        Blynk.virtualWrite(VPIN_BUTTON_LH, 0);
        Wire.beginTransmission(1);
        Serial.println("High light");
        Wire.write(pinLedH);
        Wire.write(0);
        Wire.endTransmission();
      }
    }

    //Espera 5 segundos y apaga actuadores
    delay(5000);
    Blynk.virtualWrite(VPIN_BUTTON_LS, 0);
    Blynk.virtualWrite(VPIN_BUTTON_LH, 0);
    Blynk.virtualWrite(VPIN_BUTTON_LE, 0);
    Blynk.virtualWrite(VPIN_BUTTON_F, 0);

    Serial.println(pinLedH);
    Wire.beginTransmission(1);
    Serial.println("Turning off light");
    Wire.write(pinLedS);
    Wire.write(0);
    Wire.write(pinLedE);
    Wire.write(0);
    Wire.write(pinLedH);
    Wire.write(0);
    Serial.println("Low temp");
    Wire.write(pinFan);
    Wire.write(0);
    Wire.endTransmission();
  }
}


void setup() {
  Wire.begin();
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  secure_timer_ID = timer.setInterval(1000L, secure_f);
  timer.disable(secure_timer_ID);
  fiesta_timer_ID = timer.setInterval(1000L, fiesta_f);
  timer.disable(fiesta_timer_ID);
  cine_timer_ID = timer.setInterval(1000L, cine_f);
  timer.disable(cine_timer_ID);
  
  pinMode(disp_ultraS, OUTPUT);
  pinMode(ent_ultraS, INPUT);
  pinMode(disp_ultraE, OUTPUT);
  pinMode(ent_ultraE, INPUT);
  pinMode(disp_ultraH, OUTPUT);
  pinMode(ent_ultraH, INPUT);
  
  ledcSetup(canal, frecuencia, ciclo);
  ledcAttachPin(buzzer, canal);
  Serial.println("setup");
}

void loop() {
  timer.run();
  Blynk.run();
  sendSensor();
  sonido();
  mode();
  getCurrentTime(hours, minutes, seconds);
  Serial.println(distanciaE);

}
