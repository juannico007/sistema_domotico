
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

char auth[] = "envom_tX8Wm5f4RQhZtImbiXby06WDTo";

char ssid[] = "CLARO_WIFI50B";
char pass[] = "CLARO4C6";

#define DHTPIN 13          

#define DHTTYPE DHT11     // DHT 11


DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
}

void setup()
{
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);

  dht.begin();

  // funcion llamada cada sec
  timer.setInterval(1000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
}
