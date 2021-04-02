#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

  //Libreria de la esp32-cam, debe ser descargada previamente como se especifica en el git
  #include "esp_camera.h"

  //Librerias para acceso a wifi y conexion com blynk
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <BlynkSimpleEsp32.h>

  //Libreria para el sensor de temperatura
  #include <DHT.h>

  //Usadas por varios sistemas
  bool read_movement(int PIR);

  //Luces
  void turn_light(int bomb, bool cond);
  void graduate_light(int bomb, float val);
  void auto_lights(int *bomb, int *PIR, int *fot, int n);

  //Seguridad
  void takePhoto();
  void alarm_and_notify(int BUZZER, int *bomb, int n);
  void turn_off(int BUZZER, int *bomb, int n);
  void alarm(int trig, int echo);

  //Temperatura
  float take_temp(DHT dht);
  void reduce_temp(int vent);
  void increase_temp(bool cond);
  void regulate_temp(DHT dht, int vent);

  //Modos
  void modo_fiesta(int R, int G, int B);
  void modo_seguro(int BUZZER, int *bomb, int *PIR, int n);
  void modo_cine(int *bomb, int *pers, int n, int m)

#endif
