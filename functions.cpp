#include "functions.h"

//---------------------------------FUNCIONES USADAS EN VARIOS SISTEMAS---------------------------------

bool read_movement(int PIR){
  //Funcion para obtener el movimiento de un sensor PIR
  return digitalRead(8);
}

//-------------------------------------------------------------------------------------------------------

//---------------------------------FUNCIONES RELACIONADAS CON LAS LUCES----------------------------------

void turn_light(int bomb, bool cond){
  /*Funcion para encender una luz si se cumple alguna condicion
    Recibe el pin del bombillo y la condicion
    Ej de condicion: movimiento o input del usuario
  */
  if(cond)
    digitalWrite(bomb, HIGH);
  else
    digitalWrite(bomb, LOW);
}

void graduate_light(int bomb, float val){
  /*Funcion para graduar la intensidad de las Luces
  Recibe el pin del bombillo a graduar y el valor
  El valor puede ser dado con un potenciometro o desde la aplicacion
  */
  analogWrite(led, val);
}

void auto_lights(int *bomb, int *PIR, int *fot, int n){
  /*Automatiza la luz, las enciende si detecta movimiento y hay poca iluminacion
    Recibe arreglos con los pines a los que estan conectados cada bombillo con su respectivo sensor PIR y fotocelda*/

  //Falta agregar la condicion del tiempo que lleva prendido, es decir que si dura 2 minutos prendido sin detectar movimiento se apague
  for(int i = 0; i < n; i++)
    turn_light(bomb[i], read_movement(PIR[i]) && analogRead(fot[i]) < 550);
}


//-------------------------------------------------------------------------------------------------------

//---------------------------------FUNCIONES RELACIONADAS CON EL SISTEMA DE SEGURIDAD--------------------
void takePhoto(){
  //Enciende un Led para indicar que esta tomando la foto y procede a tomarla
  digitalWrite(LED, HIGH);
  delay(200);
  //Creo una imagen y un link de acceso para que la aplicacion acceda a ella
  //Falta cambiar este link de acceso para que tenga la fecha y hora en que se tomo la foto
  uint32_t randomNum = random(50000);
  Serial.println("http://" + local_IP + "/capture?_cb=" + (String)randomNum);
  Blynk.setProperty(V1, "urls", "http://" + local_IP + "/capture?_cb=" + (String)randomNum); //El widget de la aplicacion accede a la url de la imagen
  digitalWrite(LED, LOW);
  delay(1000);
}

void alarm_and_notify(int BUZZER, int *bomb, int n){
  //Enciende la alarma, prende luces, toma foto del intruso y notifica al usuario
  //Recibe el pin al que esta conectado el buzzer, un arreglo con los pines de los bombillos de la casa y el numero de bombillos
  Blynk.notify("Alerta: Hay alguien en casa");
  for(int i = 0; i < n; i++)
    turn_light(bomb[i], true);                      //Esta condicion debe ser cambiada por la toma de hora para que si es de dia no se encienda la luz
  takePhoto();
  tone(BUZZER, 500);
}

void turn_off(int BUZZER, int *bomb, int n){
  //Apaga luces y hace que la alarma deje de sonar
  //Recibe el pin al que esta conectado el buzzer, un arreglo con los pines de los bombillos de la casa y el numero de bombillos
  for(int i = 0; i < n; i++)
    turn_light(bomb[i], false);
  noTone(BUZZER, 500);
}

void alarm(int trig, int echo, int alarm){
  //Enciende la alarma si el sensor de ultrasonido detecta un objeto en su rango
  //Recibe dos pines que estan conectados al sensor de ultrasonido y otro pin que le manda la señal a la alarma
  digitalWrite(trig,HIGH) ;
  delay(1);
  digitalWrite(trig,LOW);
  //Prendemos el pin trig para que envie la señal que llegara al receptor echo
  tiempo=pulseIn(echo,HIGH);
  distancia=tiempo/58.2;
  //Usamos formulas para saber a que distancia se encuentra un objeto
  delay(1);
  if (distancia>=2 and distancia<335){
  	digitalWrite(alarm,HIGH);
  }
  else{
  	digitalWrite(alarm,LOW);
  //Encendemos la alarma si hay algo entre los limites del sensor de ultrasonido que va entre 2 cm a 335 cm
}

//-------------------------------------------------------------------------------------------------------


//---------------------------------FUNCIONES RELACIONADAS CON EL SISTEMA DE TEMPERATURA------------------
float take_temp(DHT dht){
  // Funcion para la toma de temperatura, recibe un objeto de tipo sensor dht
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();

  // Comprobamos si ha habido algún error en la lectura
  if (isnan(t)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return -100;                //Retorna un valor de error para luego terminar el programa
  }
}

  void reduce_temp(int vent, bool cond){
    /*Funcion para reducir la temperatura con el ventilador si se cumple una condicion
      o apagarlo de lo contrario
      recibe el pin al que esta conectado el ventilador y la condicion,
      Ej de condicion: Estar por debajo de una temperatura o un input del usuario
    */
    if(cond)
      digitalWrite(vent,HIGH);
    else
      digitalWrite(vent,LOW);
  }

  void increase_temp(bool cond){
    //Funcion para incrementar la temperatura si se cumple una condicion
    //Falta por implementar
  }

  void regulate_temp(DHT dht, int vent){
    //Funcion para regular la temperatura del hogar, recibe un objeto sensor DHT y el pin del ventilador
    float t = take_temp(DHT dht);
    reduce_temp(vent, t>23);
    increase_temp(t < 19);
  }

//-------------------------------------------------------------------------------------------------------

//---------------------------------FUNCIONES RELACIONADAS CON LOS MODOS--------------------------------
void modo_fiesta(int R, int G, int B) {
  /* Ejecuta la iluminacion en casa con distintos colores para dar ambientacion de fiesta
     Recibe los pines correspondientes a los que esta conectado el led RGB*/
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

void modo_seguro(int BUZZER, int *bomb, int *PIR, int n){
  //Verifica el movimiento en los sensores para activar la alarma
  //Recibe un arreglo con los pines de los bombillos, un arreglo con los pines de los sensores de movimiento y el numero de sensores
  for(int i = 0; i < n; i++){
    if (read_movement(PIR[i]))
      alarm_and_notify(BUZZER, bomb, n);
  }
}

void modo_cine(int *bomb, int *pers, int n, int m){
  //Funcion que apaga todas las cules y baja todas las cortinas
  //Recibe un arreglo con todos los pines de las luces y otro con los pines de las persianas y sus respectivos tamaños
  for(int i = 0; i < n; i++){
    turn_light(bomb[i], false);
  }
  for(int i = 0; i < m; i++){
    close_louver(pers[i]);
  }
}
//-------------------------------------------------------------------------------------------------------
