set(component_srcs ultrasonic_main.c st7735s.c fontx.c axp192.c ultrasonic.c)

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS ".")
