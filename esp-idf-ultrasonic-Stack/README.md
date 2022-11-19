# How to build

```
git clone https://github.com/nopnop2002/esp-idf-ultrasonic
cd esp-idf-ultrasonic/esp-idf-ultrasonic-Stack
idf.py menuconfig
idf.py flash
```

\*There is no MENU ITEM where this application is peculiar.   

# Wirering

|Ultrasonic||ESP32|
|:-:|:-:|:-:|
|GND|--|GND(GROVE GND)|
|Echo|--|GPIO21(GROVE SDA)|
|Trig|--|GPIO22(GROVE SCL)|
|VCC|--|5V(GROVE VCC)|

# How to use

Press ButtonA (Left button) to start measurement.   
Press ButtonB (Center button) to stop measurement.   
Press ButtonC (Right button) to clear monitor.   

![ultrasonic-Stack](https://user-images.githubusercontent.com/6020549/61570451-9108d600-aac7-11e9-9558-83f1f1b49b04.JPG)

