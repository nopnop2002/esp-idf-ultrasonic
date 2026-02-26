# M5StickC-Plus2
This project is for M5StickC-Plus2.   

# How to build

```
git clone https://github.com/nopnop2002/esp-idf-ultrasonic
cd esp-idf-ultrasonic/M5StackC-Plus2
idf.pu menuconfig
idf.py flash --baud 115200
```

__It is necessary to specify the baud rate at the time of Flash.__   

# Configuration
![Image](https://github.com/user-attachments/assets/0fe8e137-3626-436b-a267-58d45c7a7853)   
![Image](https://github.com/user-attachments/assets/03757663-6ad2-4d9e-b936-5adfe63571e4)   
![Image](https://github.com/user-attachments/assets/90aacd1e-74c4-44f0-be82-3f5457e00fea)   


# Wiring
Use the GROVE port to connect the sensor to the M5StickC-Plus2.

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
Press ButtonB (Side button) to clear monitor.   

![Image](https://github.com/user-attachments/assets/bf8c5059-6a37-4aba-8b78-f2cc39a14447)
![Image](https://github.com/user-attachments/assets/709b59be-2268-4a17-8d53-32d473bfd463)
