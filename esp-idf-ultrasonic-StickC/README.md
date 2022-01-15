# How to build

```
git clone https://github.com/nopnop2002/esp-idf-ultrasonic
cd esp-idf-ultrasonic/esp-idf-ultrasonic-StackC
idf.pu menuconfig
idf.py flash --baud 115200
```

\*There is no MENU ITEM where this application is peculiar.   

__It is necessary to specify the baud rate at the time of Flash.__   

---

# How to use

Press ButtonA (Front button) to start measurement.   
When a ButtonA (Front button) is pressed for more than 2 seconds, It stop measurement.

![ultrasonic-StickC](https://user-images.githubusercontent.com/6020549/61570526-24daa200-aac8-11e9-9c7f-8e296359d791.JPG)

