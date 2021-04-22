#define BLYNK_PRINT Serial

 #include <Blynk.h>
 #include <WiFi.h>
 #include <WiFiClient.h>
 #include <BlynkSimpleEsp32.h>
 #include <DHT.h>
 #include <ESP32Servo.h>

Servo myservo; 
//int ADC_Max = 4096;

char auth[] = "envom_tX8Wm5f4RQhZtImbiXby06WDTo";

char ssid[] = "GUGU515TP";
char pass[] = "17589979";

int state_t, state_l;

float temperature1 = 0;
float humidity1   = 0;
int   ldrVal;
int   switchMode = 1; //Auto por default 
int security = 0; //seguridad OFF por default

//Set values for Auto Control Mode
const float maxT = 25;

const int maxL = 1000;



#define LDR_PIN            32 
#define DHTPIN             13 

#define pinFan      14   
#define pinLed      23   
#define servoPin    27


#define VPIN_BUTTON_L    V1
#define VPIN_BUTTON_T    V2
#define VPIN_BUTTON_C    V3
#define VPIN_BUTTON_S    V4

#define VPIN_HUMIDITY    V5
#define VPIN_TEMPERATURE V6
#define VPIN_SERVO       V7

#define VPIN_MODO        V8


// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

int disp_ultra = 26;   // triger Pin 7
int ent_ultra = 25;    // echo Pin 8
int canal=5;          // canal por donde se establece la comuncacion
int frecuencia=700;   //Sonido (agudo, grave)
int ciclo=8;          // ciclo de trabajo
long tiempo;
float distancia;


DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;


void readSensor(){
  ldrVal = analogRead(LDR_PIN);

  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Falla de conexion con el sensor DHT!");
    return;
  }
  else {
    humidity1 = h;
    temperature1 = t;
  }
}

void sendSensor(){
  readSensor();
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
}

BLYNK_CONNECTED() {
  // Request the latest state from the server
  Blynk.syncVirtual(VPIN_BUTTON_T);
  Blynk.syncVirtual(VPIN_BUTTON_L);
  Blynk.syncVirtual(VPIN_BUTTON_C);
}

// When App button is pushed - switch the state
BLYNK_WRITE(VPIN_BUTTON_T) {
  state_t = param.asInt();
  digitalWrite(pinFan, state_t);
}

BLYNK_WRITE(VPIN_BUTTON_L) {
  state_l = param.asInt();
  digitalWrite(pinLed, state_l);
}

BLYNK_WRITE(VPIN_BUTTON_C) {
  switchMode = param.asInt();
}

BLYNK_WRITE(VPIN_BUTTON_S) {
  security = param.asInt();
}


BLYNK_WRITE(VPIN_SERVO) {
  myservo.write(param.asInt());
}

BLYNK_WRITE(VPIN_MODO){
  switch(param.asInt()){
    case 1: // Modo Cine
      Serial.println("Modo Cine seleccionado");
      digitalWrite(pinLed, LOW);
      Blynk.virtualWrite(V1, 0);
      myservo.write(180);
      Blynk.virtualWrite(V7, 180);
      break;
    case 2: // Modo Fiesta
      Serial.println("Modo fiesta seleccionado");
      break;
    case 3: // Modo normal
      Serial.println("Item 3 seleccionado");
      break;
    default:
      Serial.println("Item desconocido");
      break;
  }
}

void mode(){
  if(switchMode == 1 && distancia<6 && security == 0){ //if Auto Mode
    if(ldrVal  <= maxL){
      digitalWrite(23, HIGH); 
      Blynk.virtualWrite(V1, 1);
      delay(5000);
      digitalWrite(23, LOW);
      Blynk.virtualWrite(V1, 0);
    }else if(ldrVal > maxL){
      digitalWrite(23, LOW);
      Blynk.virtualWrite(V1, 0);
    }
  
    if(temperature1  >= maxT){
      digitalWrite(14, HIGH); 
      Blynk.virtualWrite(V2, 1);
      delay(10000);
      digitalWrite(14, LOW);
      Blynk.virtualWrite(V2, 0);
    }else if(temperature1 < maxT){
      digitalWrite(14, LOW);
      Blynk.virtualWrite(V2, 0);
    }
  }
}

void sonido(){
  digitalWrite(disp_ultra, HIGH);
  delayMicroseconds(5);
  digitalWrite(disp_ultra, LOW);

  tiempo = (pulseIn(ent_ultra, HIGH)/2);
  distancia = float(tiempo * 0.0340);
  
  if(distancia<6 && security == 1){ //distancia menor a la del piso
    Serial.println("a");
    Blynk.notify("Hay un intruso");
    //Blynk.email("Seguridad", "Hay un intruso");
    ledcWriteTone(canal, 500);
    delay(2000);
    ledcWriteTone(canal, 0);
  }
}


void setup(){
  Serial.begin(115200);
  pinMode(disp_ultra, OUTPUT);
  pinMode(ent_ultra, INPUT);

  ledcSetup(canal, frecuencia, ciclo);
  ledcAttachPin(33, canal);

  pinMode(pinFan, OUTPUT);
  digitalWrite(pinFan, state_t);

  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, state_l);

  Blynk.begin(auth, ssid, pass);
  
  dht.begin();
  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);// Standard 50hz servo
  myservo.attach(servoPin, 500, 2400);
}

void loop(){
  timer.run();
  Blynk.run();
  sonido(); 
  mode();  
}

/*
void modo_fiesta(int R, int G, int B) {
  /* Ejecuta la iluminacion en casa con distintos colores para dar ambientacion de fiesta
     Recibe los pines correspondientes a los que esta conectado el led RGB
  digitalWrite(R,HIGH);
  digitalWrite(G,HIGH);
  digitalWrite(B,HIGH);
  delay(100);
  digitalWrite(R,LOW);
  digitalWrite(G,LOW);
  digitalWrite(B,LOW);
  delay(100);
  digitalWrite(R,LOW);
  digitalWrite(G,HIGH);
  digitalWrite(B,LOW);
  delay(100);
  digitalWrite(R,LOW);
  digitalWrite(G,LOW);
  digitalWrite(B,HIGH);
  delay(100);
  digitalWrite(R,LOW);
  digitalWrite(G,HIGH);
  digitalWrite(B,HIGH);
  delay(100);
  digitalWrite(R,HIGH);
  digitalWrite(G,LOW);
  digitalWrite(B,HIGH);
  delay(100);
  digitalWrite(R,HIGH);
  digitalWrite(G,HIGH);
  digitalWrite(B,LOW);
  delay(100);
  digitalWrite(R,HIGH);
  digitalWrite(G,HIGH);
  digitalWrite(B,LOW);
  delay(100);
}
*/
