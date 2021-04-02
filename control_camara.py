# Transferencia de imagenes por usb

import image, network, omv, rpc, sensor, struct

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

# Turn off the frame buffer connection to the IDE from the OpenMV Cam side.
#
# This needs to be done when manually compressing jpeg images at higher quality
# so that the OpenMV Cam does not try to stream them to the IDE using a fall back
# mechanism if the JPEG image is too large to fit in the IDE JPEG frame buffer on the OpenMV Cam.

omv.disable_fb(True)

#Establezco la conexion I2C de la camara como slave
interface = rpc.rpc_i2c_slave()

################################################################
# Call Backs
################################################################

# Establece el formato de los pixeles y el tamaño del frame, toma la foto
# y retorna el tamaño del buffer jpg para guardar la imagen
#
# Recibe un string con el formato de pixeles y el tamaño del frame
def jpeg_image_snapshot(data):
    pixformat, framesize = bytes(data).decode().split(",")
    sensor.set_pixformat(eval(pixformat))
    sensor.set_framesize(eval(framesize))
    img = sensor.snapshot().compress(quality=90)
    return struct.pack("<I", img.size())

def jpeg_image_read_cb():
    interface.put_bytes(sensor.get_fb().bytearray(), 5000) # timeout

# Leo datos del buffer de la imagen dado un offset y un tamaño a leer
# Si la estructura pasada esta vacia agenda una lectura.
#
# data contiene tamaño y offset.
def jpeg_image_read(data):
    if not len(data):
        interface.schedule_callback(jpeg_image_read_cb)
        return bytes()
    else:
        offset, size = struct.unpack("<II", data)
        return memoryview(sensor.get_fb().bytearray())[offset:offset+size]

# Registro call backs.

interface.register_callback(jpeg_image_snapshot)
interface.register_callback(jpeg_image_read)

# Once all call backs have been registered we can start
# processing remote events. interface.loop() does not return.

interface.loop()
