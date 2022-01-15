set(component_srcs ultrasonic_main.c sh1107.c ultrasonic.c)

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS ".")
