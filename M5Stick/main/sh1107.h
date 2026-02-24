#ifndef MAIN_SH1107_H_
#define MAIN_SH1107_H_

#include "driver/spi_master.h"

/* Control byte for i2c
Co : bit 8 : Continuation Bit
 * 1 = no-continuation (only one byte to follow)
 * 0 = the controller should expect a stream of bytes.
D/C# : bit 7 : Data/Command Select bit
 * 1 = the next byte or byte stream will be Data.
 * 0 = a Command byte or byte stream will be coming up next.
 Bits 6-0 will be all zeros.
Usage:
0x80 : Single Command byte
0x00 : Command Stream
0xC0 : Single Data byte
0x40 : Data Stream
*/
#define I2C_CONTROL_BYTE_CMD_SINGLE    0x80
#define I2C_CONTROL_BYTE_CMD_STREAM    0x00
#define I2C_CONTROL_BYTE_DATA_SINGLE   0xC0
#define I2C_CONTROL_BYTE_DATA_STREAM   0x40


#define DIRECTION0		0
#define DIRECTION90		1
#define DIRECTION180	2
#define DIRECTION270	3

#define I2CAddress 0x3C
#define SPIAddress 0xFF

typedef enum {
	SCROLL_RIGHT = 1,
	SCROLL_LEFT = 2,
	SCROLL_DOWN = 3,
	SCROLL_UP = 4,
	SCROLL_STOP = 5
} sh1107_scroll_type_t;

typedef struct {
	bool _valid; // Not using it anymore
	int _segLen; // Not using it anymore
	uint8_t _segs[64];
} PAGE_t;

typedef struct {
	int _address;
	int _width;
	int _height;
	int _pages;
	int _direction;
	int16_t _dc;
	spi_device_handle_t _SPIHandle;
	PAGE_t _page[16];
} SH1107_t;

void sh1107_init(SH1107_t * dev, int width, int height);
void sh1107_show_buffer(SH1107_t * dev);
void sh1107_set_buffer(SH1107_t * dev, uint8_t * buffer);
void sh1107_get_buffer(SH1107_t * dev, uint8_t * buffer);
void sh1107_display_image(SH1107_t * dev, int page, int seg, uint8_t * images, int width);
void sh1107_display_text(SH1107_t * dev, int row, int col, char * text, int text_len, bool invert);
void sh1107_clear_screen(SH1107_t * dev, bool invert);
void sh1107_clear_line(SH1107_t * dev, int row, bool invert);
void sh1107_contrast(SH1107_t * dev, int contrast);
void sh1107_wrap_arround(SH1107_t * dev, sh1107_scroll_type_t scroll, int start, int end, int8_t delay);
void sh1107_bitmaps(SH1107_t * dev, int xpos, int ypos, uint8_t * bitmap, int width, int height, bool invert);
void sh1107_invert(uint8_t *buf, size_t blen);
uint8_t sh1107_copy_bit(uint8_t src, int srcBits, uint8_t dst, int dstBits);
uint8_t sh1107_rotate_byte(uint8_t ch1);
void sh1107_rotate_image(uint8_t * buf, int dir);
void sh1107_fadeout(SH1107_t * dev);
void sh1107_direction(SH1107_t * dev, int dir);
void sh1107_dump(SH1107_t dev);
void sh1107_dump_page(SH1107_t * dev, int page, int seg);

void spi_master_init(SH1107_t * dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RESET);
bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t* Data, size_t DataLength );
bool spi_master_write_command(SH1107_t * dev, uint8_t Command );
bool spi_master_write_data(SH1107_t * dev, const uint8_t* Data, size_t DataLength );
void spi_init(SH1107_t * dev, int width, int height);
void spi_display_image(SH1107_t * dev, int page, int col, uint8_t * images, int width);
void spi_contrast(SH1107_t * dev, int contrast);

void i2c_master_init(SH1107_t * dev, int16_t sda, int16_t scl, int16_t reset);
void i2c_init(SH1107_t * dev, int width, int height);
void i2c_display_image(SH1107_t * dev, int page, int seg, uint8_t * images, int width);
void i2c_contrast(SH1107_t * dev, int contrast);


#endif /* MAIN_SH1107_H_ */
