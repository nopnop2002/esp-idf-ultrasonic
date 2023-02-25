# esp-idf-ultrasonic-Stick
This is the repository for M5Stick.   

# How to build

```
git clone https://github.com/nopnop2002/esp-idf-ultrasonic
cd esp-idf-ultrasonic/esp-idf-ultrasonic-Stick
idf.py menuconfig
idf.py flash
```

\*There is no MENU ITEM where this application is peculiar.   

# Wirering
Use the GROVE port to connect the sensor to the M5Stick.

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

Press button to start measurement.   
When a button is pressed for more than 2 seconds, It stop measurement.

![ultrasonic-Stick](https://user-images.githubusercontent.com/6020549/61570488-dcbb7f80-aac7-11e9-80ac-45a7152b3e14.JPG)

