set(component_srcs main.c m5stick.c sh1107.c sh1107_i2c.c sh1107_spi.c ultrasonic.c)

idf_component_register(SRCS "${component_srcs}" INCLUDE_DIRS ".")
