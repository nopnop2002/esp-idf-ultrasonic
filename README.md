# esp-idf-ultrasonic
Ultrasonic distance sensor for ESP-IDF

I forked from [here](https://github.com/UncleRus/esp-idf-lib/tree/master/examples/ultrasonic).   

# Hardware requirements
- Ultrasonic ranging module like US-015/US-025/US-026/US-100/HC-SR04/HY-SRF05.

![HY-SRF05-3](https://user-images.githubusercontent.com/6020549/61570755-a67eff80-aac9-11e9-9e9c-19e946fae39f.JPG)

- esp-idf-ultrasonic-Stick is for M5Stick

- esp-idf-ultrasonic-StickC is for M5StickC

- esp-idf-ultrasonic-Stack is for M5Stack

- esp-idf-ultrasonic-WebSocket is for generic ESP32

# Software requirement
esp-idf v4.4/v5.0.   


# Speed of sound
The speed of sound is affected by temperature and can be calculated with ```331.5+0.61*temperature```[m/sec].   
If the temperature is 20 degrees, it will be ```331.5+0.61*20=343.7```[m/sec].   
343.7[m/sec]=34370[cm/sec]=34.37[cm/millisec]=0.03437[cm/microsec]   
Counting backwards, the time required for 1 cm is 29.0951 microseconds.   
In other words, the desired distance [cm] when the temperature is 20ÅãC can be calculated by dividing the round trip time [microseconds] from the time the sound wave is emitted until it returns by ```29.0951*2 (approximately 58.2)```.
This example uses 58 as an approximation.   
If you want more accuracy, you should measure the temperature at the same time and use the measured temperature to calculate the distance.   

