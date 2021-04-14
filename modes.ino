 #define BLYNK_PRINT Serial


 #include <WiFi.h>
 #include <WiFiClient.h>
 #include <BlynkSimpleEsp32.h>
 #include <DHT.h>
 

 char auth[] = "envom_tX8Wm5f4RQhZtImbiXby06WDTo";

char ssid[] = "GUGU515TP";
char pass[] = "17589979";

int state_t, state_l;

float temperature1 = 0;
float humidity1   = 0;
int   ldrVal;
int   switchMode = 1; //Auto por default 

//Set values for Auto Control Mode
const float maxT = 24.5;

const int maxL = 800;



#define LDR_PIN            32 
#define DHTPIN             13 

#define pinFan      35   
#define pinLed      27   


#define VPIN_BUTTON_L    V1
#define VPIN_BUTTON_T    V2
#define VPIN_BUTTON_C    V3

#define VPIN_HUMIDITY    V5
#define VPIN_TEMPERATURE V6

// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301


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

void sendSensor()
{
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

void mode(){
  if(switchMode == 1){ //if Auto Mode
    if(ldrVal  <= maxL){
      digitalWrite(27, HIGH); 
      Blynk.virtualWrite(V1, 1);
    }else if(ldrVal > maxL){
      digitalWrite(27, LOW);
      Blynk.virtualWrite(V1, 0);
    }
  
    if(temperature1  >= maxT){
      digitalWrite(35, HIGH); 
      Blynk.virtualWrite(V2, 1);
    }else if(temperature1 < maxT){
      digitalWrite(35, LOW);
      Blynk.virtualWrite(V2, 0);
    }
  }
}
void setup()
{
  Serial.begin(9600);

  pinMode(pinFan, OUTPUT);
  digitalWrite(pinFan, state_t);

  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, state_l);

  Blynk.begin(auth, ssid, pass);
  
  dht.begin();
  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);
  
}

void loop(){

  timer.run();
  Blynk.run();
  mode(); 
}
