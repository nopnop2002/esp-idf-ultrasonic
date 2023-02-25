# How to build

```
git clone https://github.com/nopnop2002/esp-idf-ultrasonic
cd esp-idf-ultrasonic/esp-idf-ultrasonic-StackC
idf.pu menuconfig
idf.py flash --baud 115200
```

\*There is no MENU ITEM where this application is peculiar.   

__It is necessary to specify the baud rate at the time of Flash.__   

# Wirering
Use the GROVE port to connect the sensor to the M5StickC.

|Ultrasonic||GROVE Port||
|:-:|:-:|:-:|:-:|
|GND|--|GND||
|Echo|--|SDA|(*1)|
|Trig|--|SCL||
|VCC|--|VCC(5V)||

(*1)   
The ultrasonic ranging module is powered by 5V.   
So you need to level shift your Echo from 5V to 3.3V.   


# How to use

Press ButtonA (Front button) to start measurement.   
When a ButtonA (Front button) is pressed for more than 2 seconds, It stop measurement.

![ultrasonic-StickC](https://user-images.githubusercontent.com/6020549/61570526-24daa200-aac8-11e9-9c7f-8e296359d791.JPG)

