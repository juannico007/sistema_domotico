#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

const char* ssid = "XXXXXXXXXXXXXXX";               //Nombre de la red
const char* password = "XXXXXXXXXXXXXX";            //Contraseña
char auth[] = "XXXXXXXXXXXXXXXX";                   //Clave de autenticacion dada por blynk

// Seleccion del modelo de camara depende de la placa esp32-cam

//define CAMERA_MODEL_WROVER_KIT
//define CAMERA_MODEL_ESP_EYE
//define CAMERA_MODEL_M5STACK_PSRAM
//define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
//define CAMERA_MODEL_TTGO_T_JOURNAL


#include "camera_pins.h"      //Se hace tras definir porque los pines se asignan en funcion del modelo
#define PIR 13                //Pin del sensor PIR
#define PHOTO 14              //Pin de salida de la camara
#define LED 4                 //Pin del LED

String local_IP;
void startCameraServer();

void takePhoto()
{
  digitalWrite(LED, HIGH);
  delay(200);
  //Creo una imagen y un link de acceso para que la aplicacion acceda a ella
  uint32_t randomNum = random(50000);
  Serial.println("http://"+local_IP+"/capture?_cb="+ (String)randomNum);
  Blynk.setProperty(V1, "urls", "http://"+local_IP+"/capture?_cb="+(String)randomNum);    //Acceso
  digitalWrite(LED, LOW);
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED,OUTPUT);
  Serial.setDebugOutput(true);
  Serial.println();

  //Asignacion de los pines definidos en el archivo camera_pins.h
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Algunos modelos tienen memoria RAM Pseudoestática, lo que permite una mejor resolucion en la imagen
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Inicializa la camara con los pines asignados y si falla termina el programa
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //Sensor de toma de imagen del modulo
  sensor_t * s = esp_camera_sensor_get();
  // Los sensores empiezan volteados verticalmente y los colores muy saturados
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // Lo voltea para que quede en la orientacion adecuada
    s->set_brightness(s, 1); // Incrementa un poco el brillo
    s->set_saturation(s, -2); // Reduce la saturacion
  }
  // Ajusta el tamaño del frame para mejorar los fps
  s->set_framesize(s, FRAMESIZE_QVGA);

  //Realiza la conexion con la red WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //Inicializa la camara y conecta con Blynk
  startCameraServer();
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  local_IP = WiFi.localIP().toString();
  Serial.println("' to connect");
  Blynk.begin(auth, ssid, password);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  if(digitalRead(PIR) == LOW){
  Serial.println("Send Notification");
  Blynk.notify("Alert:Some one has been here.");
  Serial.println("Capture Photo");
  takePhoto();
  delay(3000);
  }
  if(digitalRead(PHOTO) == HIGH){
  Serial.println("Capture Photo");
  takePhoto();
  }  
}
