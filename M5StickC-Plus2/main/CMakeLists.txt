set(component_srcs main.c m5stickC-Plus2.c st7789.c fontx.c sgm2578.c ultrasonic.c)

idf_component_register(SRCS "${component_srcs}" INCLUDE_DIRS ".")
