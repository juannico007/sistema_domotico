// Control remoto de la cámara con el arduino
//
// utiliza la libreria rpc
//
// funciona junto con el script "image_transfer_jpg_as_the_remote_device.py" en la camara OpenMV Cam. 
// El script esta en el IDE abriendo Files -> Examples -> Remote Control. 

#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <openmvrpc.h>


// Crea un buffer que pasara mensajes entre la camara y el arduino. 
// La cantidad de datos pasadas por mensaje está limitada al tamaño del buffer.
openmv::rpc_scratch_buffer<32> scratch_buffer; 

//Establezco el protocolo I2C para controlar la camara
openmv::rpc_i2c_master interface(0x12, 100000); 

#define SSID1 "replace with your wifi ssid"
#define PWD1 "replace your wifi password"

WebServer server(80);

//Variables para poder escribir la imagen en linea
const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);

void handle_jpg(void)
{
  // Establece el formato de los pixeles y el tamaño de la imagen para ser leidos por la camara
    char pixformat_and_framesize[] = "sensor.RGB565,sensor.QQQVGA";
    uint32_t jpeg_size;

    WiFiClient client = server.client();
    
    client.write(JHEADER, jhdLen);
    
    // La funcion jpeg_image_snapshot toma una imagen jpeg, la guarda en la memoria de la camara y retorna
    // el tamaño de la imagen jpg en bytes para ser leidos por el arduino.
    Serial.println(F("Taking a pic..."));
    // Llamo la función jpeg_image_snapshot del script de la camara
    if (interface.call(F("jpeg_image_snapshot"),
                       pixformat_and_framesize, sizeof(pixformat_and_framesize) - 1, // Do not send NULL terminator
                       &jpeg_size, sizeof(jpeg_size))) {
                        
        Serial.println(F("Success"));
        if (!client.connected()) return;
        
        // jpeg_image_read toma 2 argumentos, offset: en donde va la lectura y size: el tamaño de la imagen.
        // Estos argumentos se pueden pasar usando la siguiente estructura.
        struct {
            uint32_t offset;
            uint32_t size;
        } arg;

        // Para leer el archivo tenemos que hacerlo por chunks limitados por el tamaño del buffer, este tamaño se debe modificar con pruebas.
        arg.offset = 0;
        arg.size = scratch_buffer.buffer_size();

        // Aqui leemos los chunks de la imagen en orden y los subimos.
        while (true) {
            // interface.call_no_copy() retornara un apuntador a los datos del buffer 
            // que se transmitieron. 
            char *jpg_data;
            size_t jpg_data_len;

            // Transfiere los bytes. si el metodo tiene exito jpg_data and jpg_data_len apuntaran a los datos pasados.
            Serial.print(F("Reading bytes "));
            Serial.print((arg.offset * 100) / jpeg_size);
            Serial.println(F("%"));
            if (interface.call(F("jpeg_image_read"), &arg, sizeof(arg), &jpg_data, jpg_data_len)) {
                Serial.println(F("Writing bytes..."));

                // Finalmente escribimos los datos en el servidor.
                client.write((char *)jpg_data, jpg_data_len);
                arg.offset += jpg_data_len;

                // Termina el ciclo una vez escribe la imagen
                if (arg.offset >= jpeg_size) {
                    Serial.println(F("File written"));
                    break;
                }
            } else {
                Serial.println(F("Failed!"));
                break;
            }
        }
    } else {
        Serial.println(F("Failed!"));
    }
}

void setup() {
    // Inicio la interface y un canal de debug.
    interface.begin();
    Serial.begin(115200);

    while (!Serial); // wait for Serial Monitor to connect. Needed for native USB port boards only:
    IPAddress ip;

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID1, PWD1);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(F("."));
    }
    ip = WiFi.localIP();
    Serial.println(F("WiFi connected"));
    Serial.println("");
    Serial.println(ip);
    //Serial.print("Stream Link: http://");
    //Serial.print(ip);
    //Serial.println("/mjpeg/1");
    //server.on("/mjpeg/1", HTTP_GET, handle_jpg_stream);
    server.on("/jpg", HTTP_GET, handle_jpg);
    //server.onNotFound(handleNotFound);
    server.begin();
    Serial.println(F("Initialization done."));
}

void loop() {
    server.handleClient();
}
