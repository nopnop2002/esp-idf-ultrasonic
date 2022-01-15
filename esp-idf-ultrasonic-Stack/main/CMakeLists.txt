set(component_srcs ultrasonic_main.c ili9340.c fontx.c ultrasonic.c)

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS ".")
