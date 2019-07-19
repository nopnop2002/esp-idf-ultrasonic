# esp-idf-ultrasonic
Ultrasonic distance sensor for M5 series

I forked from [here](https://github.com/UncleRus/esp-idf-lib/tree/master/examples/ultrasonic).   

---

# Hardware requirements
Ultrasonic sensor like US-015/US-025/US-026/US-100/HC-SR04/HY-SRF05.

![HY-SRF05-3](https://user-images.githubusercontent.com/6020549/61570755-a67eff80-aac9-11e9-9e9c-19e946fae39f.JPG)

---

# Wirering
Use the GROVE port to connect the sensor to the M5.

|Ultrasonic||GROVE Port|
|:-:|:-:|:-:|
|GND|--|GND(Black)|
|Echo|--|SDA(White)|
|Trig|--|SCL(Yellow)|
|VCC|--|VCC(Red)|

