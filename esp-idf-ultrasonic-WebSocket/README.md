# esp-idf-ultrasonic-WebSocket
This is the repository for generic ESP32.   

![Ultrasonic-WebSocket](https://user-images.githubusercontent.com/6020549/205416156-c6431e26-35dd-45c8-a910-f4846a4db202.jpg)

I used [this](https://github.com/Molorius/esp32-websocket) component.   
This component can communicate directly with the browser.   
There is an example of using the component [here](https://github.com/Molorius/ESP32-Examples).
It's a great job.   

# Software requirements
ESP-IDF V4.4/V5.x.   
ESP-IDF V5.0 is required when using ESP32-C2.   
ESP-IDF V5.1 is required when using ESP32-C6.   


# How to build
```
git clone https://github.com/nopnop2002/esp-idf-ultrasonic
cd esp-idf-ultrasonic/esp-idf-ultrasonic-WebSocket
git clone https://github.com/Molorius/esp32-websocket components/websocket
idf.py set-target {esp32/esp32s2/esp32s3/esp32c2/esp32c3/esp32c6}
idf.py menuconfig
idf.py flash
```


# Configuration   
![config-menu](https://user-images.githubusercontent.com/6020549/205415877-7735e45d-18a2-4a23-a75a-b849e108ccb7.jpg)
![config-app](https://user-images.githubusercontent.com/6020549/205415875-b0e688da-159b-4877-98aa-5f6df9faa5b9.jpg)

## WiFi Setting
![config-wifi](https://user-images.githubusercontent.com/6020549/205415892-5ec984d8-ec50-4f06-bd79-0f457f9c000d.jpg)

You can use Static IP.   
![config-wifi-2](https://user-images.githubusercontent.com/6020549/205416000-2bae9320-3dff-42bc-8475-a6a58c80a897.jpg)

You can use the MDNS hostname instead of the IP address.   
![config-wifi-3](https://user-images.githubusercontent.com/6020549/205416016-b11ead06-f0a7-4460-8fe3-2d52f8b2acf8.jpg)


## Sensor Setting
![config-sensor](https://user-images.githubusercontent.com/6020549/205416047-5f6450c6-6fd6-47ce-8bef-507efb5c6f36.jpg)



# Wirering

|Ultrasonic||ESP32|ESP32-S2/S3|ESP32-C2/C3/C6||
|:-:|:-:|:-:|:-:|:-:|:-:|
|GND|--|GND|GND|GND||
|Echo|--|GPIO21|GPIO11|GPIO5|(*1)(*2)|
|Trig|--|GPIO22|GPIO12|GPIO6|(*2)|
|VCC|--|5V|5V|5V||

(*1)   
The ultrasonic ranging module is powered by 5V.   
So you need to level shift your Echo from 5V to 3.3V.   

(*2)   
You can change any pin using menuconfig.   



# How to use
Open browser.   
Enter the esp32 IP address in your browser's address bar.   
You can use the mDNS hostname instead of the IP address.   
Default mDNS name is esp32-server.local.   

# WEB Page
WEB page is stored in the html folder.   
You can change it as you like.   
Radial gauge example is [here](https://rawgit.com/Mikhus/canvas-gauges/master/examples/radial-component.html).   
Linear gauge example is [here](https://rawgit.com/Mikhus/canvas-gauges/master/examples/linear-component.html).

