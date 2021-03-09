#include <Arduino_OV767X.h>                                 //Libreria de la camara
#include <SPI.h>                                            //Modulos para el controlador SD
#include <SD.h>

unsigned short pixels[640 * 480];                           // Tamaño de imagen con setup (VGA,RGB565)
const int chipSelect = 7;

void setup() {
  
  Serial.begin(9600);
  while (!Serial);

  //Verifica la SD
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {                              //Aqui se inicializa la salida de la SD
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("initialization done.");
  
  Serial.println("OV767X Camera Capture");
  Serial.println();

  if (!Camera.begin(VGA, RGB565, 1)) {                      //Aqui inicializa la camara
    Serial.println("Failed to initialize camera!");
    while (1);
  }

  //Muestra la configuracion de la camara
  Serial.println("Camera settings:");
  Serial.print("\twidth = ");
  Serial.println(Camera.width());
  Serial.print("\theight = ");
  Serial.println(Camera.height());
  Serial.print("\tbits per pixel = ");
  Serial.println(Camera.bitsPerPixel());
  Serial.println();

  //Tomara la foto al recibir una c por la entrada de momento
  Serial.println("Send the 'c' character to read a frame ...");
  Serial.println();
}

void loop() {
  if (Serial.read() == 'c') {
    Serial.println("Reading frame");
    Serial.println();
    Camera.readFrame(pixels);

    int numPixels = Camera.width() * Camera.height();

    //Nombre de la foto
    char filename[13];
    strcpy(filename, "IMAGE00.JPG");
    //Crea hasta 100 imagenes
    for (int i = 0; i < 100; i++) {
      filename[5] = '0' + i/10;
      filename[6] = '0' + i%10;
      // Lo crea si no existe en la SD, no sobreescibe
      if (! SD.exists(filename)) {
        break;
      }
    File imgFile = SD.open(filename, FILE_WRITE);
    }

    //Escribe pixel por pixel en la imagen
    if (imgFile) {
       Serial.print("Writing image to ");
       Serial.print(filename);
       for (int i = 0; i < numPixels; i++) {
         unsigned short p = pixels[i];
    
         if (p < 0x1000) {
           imgFile.print('0');
         }
    
         if (p < 0x0100) {
           imgFile.print('0');
         }
    
         if (p < 0x0010) {
           imgFile.print('0');
         }
    
         imgFile.print(p, HEX);
       }
       myFile.close();
       Serial.println("done.");
    } else {
       Serial.println("error opening test.txt");
    }
  }
}
