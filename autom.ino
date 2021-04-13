#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

char auth[] = "envom_tX8Wm5f4RQhZtImbiXby06WDTo";

char ssid[] = "CLARO_WIFI50B";
char pass[] = "CLARO4C6";

#define DHTPIN 13          
#define PHTR 32

#define DHTTYPE DHT11     // DHT 11


DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

const float maxT = 20;
const float maxL = 800; 
float l, h, t; 


void sendSensor()
{
  h = dht.readHumidity();
  t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  l = analogRead(32);
  int led = digitalRead(27); 
  int fan = digitalRead(35);
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
  Blynk.virtualWrite(V1, led);
  Blynk.virtualWrite(V2, fan);
} 

void setup()
{

  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);

  dht.begin();

  timer.setInterval(1000L, sendSensor);

  pinMode(27, OUTPUT);
}

void loop()
{
  if(l <= maxL){
   digitalWrite(27, HIGH); 
   Blynk.virtualWrite(V1, 1);
  }else if(l > maxL){
    digitalWrite(27, LOW);
    Blynk.virtualWrite(V1, 0);
  }

  if(t >= maxT){
   digitalWrite(35, HIGH); 
   Blynk.virtualWrite(V2, 1);
  }else if(t < maxT){
    digitalWrite(35, LOW);
    Blynk.virtualWrite(V2, 0);
  }
  
  Blynk.run();
  timer.run();
}
