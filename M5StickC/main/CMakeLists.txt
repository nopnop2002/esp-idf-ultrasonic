set(component_srcs main.c m5stickC.c st7735s.c fontx.c axp192.c ultrasonic.c)

idf_component_register(SRCS "${component_srcs}" INCLUDE_DIRS ".")
