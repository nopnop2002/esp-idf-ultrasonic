#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "sh1107.h"
#include "font8x8_basic.h"

#define TAG "SH1107"

void sh1107_init(SH1107_t * dev, int width, int height)
{
	if (dev->_address == SPIAddress) {
		spi_init(dev, width, height);
	} else {
		i2c_init(dev, width, height);
	}
}

void sh1107_show_buffer(SH1107_t * dev)
{
	if (dev->_address == SPIAddress) {
		for (int page=0; page<dev->_pages;page++) {
			spi_display_image(dev, page, 0, dev->_page[page]._segs, dev->_width);
		}
	} else {
		for (int page=0; page<dev->_pages;page++) {
			i2c_display_image(dev, page, 0, dev->_page[page]._segs, dev->_width);
		}
	}
}

void sh1107_set_buffer(SH1107_t * dev, uint8_t * buffer)
{
	int index = 0;
	for (int page=0; page<dev->_pages;page++) {
		memcpy(&dev->_page[page]._segs, &buffer[index], 64);
		index = index + 64;
	}
}

void sh1107_get_buffer(SH1107_t * dev, uint8_t * buffer)
{
	int index = 0;
	for (int page=0; page<dev->_pages;page++) {
		memcpy(&buffer[index], &dev->_page[page]._segs, 64);
		index = index + 64;
	}
}

void sh1107_display_image(SH1107_t * dev, int page, int seg, uint8_t * images, int width)
{
	if (dev->_address == SPIAddress) {
		spi_display_image(dev, page, seg, images, width);
	} else {
		i2c_display_image(dev, page, seg, images, width);
	}
	memcpy(&dev->_page[page]._segs[seg], images, width);
}

void sh1107_display_text(SH1107_t * dev, int row, int col, char * text, int text_len, bool invert)
{
	int _length = text_len;
	int _width = 0;
	int _height = 0;
	int _row = 0;
	int _col = 0;
	int _rowadd = 0;
	int _coladd = 0;
	if (dev->_direction == DIRECTION0) {
		_width = dev->_width / 8;
		_height = dev->_height / 8;
		_row = row;
		_col = col*8;
		_rowadd = 0;
		_coladd = 8;
	} else if (dev->_direction == DIRECTION90) {
		_width = dev->_height / 8;
		_height = dev->_width / 8;
		_row = col;
		_col = (dev->_width-8) - (row*8);
		_rowadd = 1;
		_coladd = 0;
	} else if (dev->_direction == DIRECTION180) {
		_width = dev->_width / 8;
		_height = dev->_height / 8;
		_row = _height - row - 1;
		_col = (dev->_width-8) - (col*8);
		_rowadd = 0;
		_coladd = -8;
	} else if (dev->_direction == DIRECTION270) {
		_width = dev->_height / 8;
		_height = dev->_width / 8;
		_row = (dev->_pages-1) - col;
		_col = row*8;
		_rowadd = -1;
		_coladd = 0;
	}
	if (row >= _height) return;
	if (col >= _width) return;
	if (col + text_len > _width) _length = _width - col;
	ESP_LOGD(TAG, "_direction=%d _width=%d _height=%d _length=%d _row=%d _col=%d", 
		dev->_direction, _width, _height, _length, _row, _col);

	uint8_t image[8];
	for (int i=0; i<_length; i++) {
		memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
		if (invert) sh1107_invert(image, 8);
		sh1107_rotate_image(image, dev->_direction);
		sh1107_display_image(dev, _row, _col, image, 8);
		_row = _row + _rowadd;
		_col = _col + _coladd;
	}
}

void sh1107_clear_screen(SH1107_t * dev, bool invert)
{
	uint8_t zero[64];
	if (invert) {
		memset(zero, 0xff, sizeof(zero));
	} else {
		memset(zero, 0x00, sizeof(zero));
	}
	for (int page = 0; page < dev->_pages; page++) {
		memcpy(&dev->_page[page]._segs[0], zero, 64);
		if (dev->_address == SPIAddress) {
			spi_display_image(dev, page, 0, zero, dev->_width);
		} else {
			i2c_display_image(dev, page, 0, zero, dev->_width);
		}
	}
}

void sh1107_clear_line(SH1107_t * dev, int row, bool invert)
{
	char space[1];
	space[0] = 0x20;
	if (dev->_direction == DIRECTION0 || dev->_direction == DIRECTION90) {
		int _width = dev->_width / 8;
		for (int col=0;col<_width;col++) {
			sh1107_display_text(dev, row, col, space, 1, invert);
		}
	} else {
		int _width = dev->_height / 8;
		for (int col=0;col<_width;col++) {
			sh1107_display_text(dev, row, col, space, 1, invert);
		}
	}
}

void sh1107_contrast(SH1107_t * dev, int contrast)
{
	if (dev->_address == SPIAddress) {
		spi_contrast(dev, contrast);
	} else {
		i2c_contrast(dev, contrast);
	}
}

// delay = 0 : display with no wait
// delay > 0 : display with wait
// delay < 0 : no display
void sh1107_wrap_arround(SH1107_t * dev, sh1107_scroll_type_t scroll, int start, int end, int8_t delay)
{
	if (scroll == SCROLL_RIGHT) {
		int _start = start; // 0 to 15
		int _end = end; // 0 to 15
		if (_end >= dev->_pages) _end = dev->_pages - 1;
		uint8_t wk;
		//for (int page=0;page<dev->_pages;page++) {
		for (int page=_start;page<=_end;page++) {
			wk = dev->_page[page]._segs[63];
			for (int seg=63;seg>0;seg--) {
				dev->_page[page]._segs[seg] = dev->_page[page]._segs[seg-1];
			}
			dev->_page[page]._segs[0] = wk;
		}

	} else if (scroll == SCROLL_LEFT) {
		int _start = start; // 0 to 15
		int _end = end; // 0 to 15
		if (_end >= dev->_pages) _end = dev->_pages - 1;
		uint8_t wk;
		//for (int page=0;page<dev->_pages;page++) {
		for (int page=_start;page<=_end;page++) {
			wk = dev->_page[page]._segs[0];
			for (int seg=0;seg<63;seg++) {
				dev->_page[page]._segs[seg] = dev->_page[page]._segs[seg+1];
			}
			dev->_page[page]._segs[63] = wk;
		}

	} else if (scroll == SCROLL_UP) {
		int _start = start; // 0 to {width-1}
		int _end = end; // 0 to {width-1}
		if (_end >= dev->_width) _end = dev->_width - 1;
		uint8_t wk0;
		uint8_t wk1;
		uint8_t wk2;
		uint8_t save[64];
		// Save pages 0
		for (int seg=0;seg<64;seg++) {
			save[seg] = dev->_page[0]._segs[seg];
		}
		// Page0 to Page15
		for (int page=0;page<dev->_pages-1;page++) {
			//for (int seg=0;seg<64;seg++) {
			for (int seg=_start;seg<=_end;seg++) {
				wk0 = dev->_page[page]._segs[seg];
				wk1 = dev->_page[page+1]._segs[seg];
				if (seg == 0) {
					ESP_LOGD(TAG, "b page=%d wk0=%02x wk1=%02x", page, wk0, wk1);
				}
				wk0 = wk0 >> 1;
				wk1 = wk1 & 0x01;
				wk1 = wk1 << 7;
				wk2 = wk0 | wk1;
				if (seg == 0) {
					ESP_LOGD(TAG, "a page=%d wk0=%02x wk1=%02x wk2=%02x", page, wk0, wk1, wk2);
				}
				dev->_page[page]._segs[seg] = wk2;
			}
		}
		// Page15
		int pages = dev->_pages-1;
		//for (int seg=0;seg<64;seg++) {
		for (int seg=_start;seg<=_end;seg++) {
			wk0 = dev->_page[pages]._segs[seg];
			wk1 = save[seg];
			wk0 = wk0 >> 1;
			wk1 = wk1 & 0x01;
			wk1 = wk1 << 7;
			wk2 = wk0 | wk1;
			dev->_page[pages]._segs[seg] = wk2;
		}

	} else if (scroll == SCROLL_DOWN) {
		int _start = start; // 0 to {width-1}
		int _end = end; // 0 to {width-1}
		if (_end >= dev->_width) _end = dev->_width - 1;
		uint8_t wk0;
		uint8_t wk1;
		uint8_t wk2;
		uint8_t save[64];
		// Save pages 15
		int pages = dev->_pages-1;
		for (int seg=0;seg<64;seg++) {
			save[seg] = dev->_page[pages]._segs[seg];
		}
		// Page15 to Page1
		for (int page=pages;page>0;page--) {
			//for (int seg=0;seg<64;seg++) {
			for (int seg=_start;seg<=_end;seg++) {
				wk0 = dev->_page[page]._segs[seg];
				wk1 = dev->_page[page-1]._segs[seg];
				if (seg == 0) {
					ESP_LOGD(TAG, "b page=%d wk0=%02x wk1=%02x", page, wk0, wk1);
				}
				wk0 = wk0 << 1;
				wk1 = wk1 & 0x80;
				wk1 = wk1 >> 7;
				wk2 = wk0 | wk1;
				if (seg == 0) {
					ESP_LOGD(TAG, "a page=%d wk0=%02x wk1=%02x wk2=%02x", page, wk0, wk1, wk2);
				}
				dev->_page[page]._segs[seg] = wk2;
			}
		}
		// Page0
		for (int seg=_start;seg<=_end;seg++) {
			wk0 = dev->_page[0]._segs[seg];
			wk1 = save[seg];
			wk0 = wk0 << 1;
			wk1 = wk1 & 0x80;
			wk1 = wk1 >> 7;
			wk2 = wk0 | wk1;
			dev->_page[0]._segs[seg] = wk2;
		}

	}

	if (delay >= 0) {
		for (int page=0;page<dev->_pages;page++) {
			if (dev->_address == SPIAddress) {
				spi_display_image(dev, page, 0, dev->_page[page]._segs, 64);
			} else {
				i2c_display_image(dev, page, 0, dev->_page[page]._segs, 64);
			}
			if (delay) vTaskDelay(delay);
		}
	}
}

void sh1107_bitmaps(SH1107_t * dev, int xpos, int ypos, uint8_t * bitmap, int width, int height, bool invert)
{
	if ( (width % 8) != 0) {
		ESP_LOGE(TAG, "width must be a multiple of 8");
		return;
	}
	if ( (xpos % 8) != 0) {
		ESP_LOGE(TAG, "xpos must be a multiple of 8");
		return;
	}
	int _width = width / 8;
	//uint8_t wk0;
	uint8_t wk1;
	int _seg = 63 - ypos;
	int offset = 0;
	for(int _height=0;_height<height;_height++) {
		int page = (xpos / 8);
		for (int index=0;index<_width;index++) {
			//wk0 = dev->_page[page]._segs[_seg];
			wk1 = bitmap[index+offset];
			wk1 = sh1107_rotate_byte(wk1);
			if (invert) wk1 = ~wk1;
			dev->_page[page]._segs[_seg] = wk1;
			page++;
		}
		vTaskDelay(1);
		offset = offset + _width;
		_seg--;
	}

	sh1107_show_buffer(dev);
}

void sh1107_invert(uint8_t *buf, size_t blen)
{
	uint8_t wk;
	for(int i=0; i<blen; i++){
		wk = buf[i];
		buf[i] = ~wk;
	}
}


uint8_t sh1107_copy_bit(uint8_t src, int srcBits, uint8_t dst, int dstBits)
{
	ESP_LOGD(TAG, "src=%02x srcBits=%d dst=%02x dstBits=%d", src, srcBits, dst, dstBits);
	uint8_t smask = 0x01 << srcBits;
	uint8_t dmask = 0x01 << dstBits;
	uint8_t _src = src & smask;
#if 0
	if (_src != 0) _src = 1;
	uint8_t _wk = _src << dstBits;
	uint8_t _dst = dst | _wk;
#endif
	uint8_t _dst;
	if (_src != 0) {
		_dst = dst | dmask; // set bit
	} else {
		_dst = dst & ~(dmask); // clear bit
	}
	return _dst;
}

// Rotate 8-bit data
// 0x12-->0x48
uint8_t sh1107_rotate_byte(uint8_t ch1) {
	uint8_t ch2 = 0;
	for (int j=0;j<8;j++) {
		ch2 = (ch2 << 1) + (ch1 & 0x01);
		ch1 = ch1 >> 1;
	}
	return ch2;
}

void sh1107_rotate_image(uint8_t * buf, int dir)
{
	uint8_t wk[8];
	if (dir == DIRECTION0) return;
	for(int i=0; i<8; i++){
		wk[i] = buf[i];
		buf[i] = 0;
	}
	if (dir == DIRECTION90 || dir == DIRECTION270) {
		uint8_t wk2[8];
		uint8_t mask1 = 0x80;
		for(int i=0;i<8;i++) {
			wk2[i] = 0;
			ESP_LOGD(TAG, "wk[%d]=%x", i, wk[i]);
			uint8_t mask2 = 0x01;
			for(int j=0;j<8;j++) {
				if( (wk[j] & mask1) == mask1) wk2[i] = wk2[i] + mask2;
				mask2 = mask2 << 1;
			}
			mask1 = mask1 >> 1;
		}
		for(int i=0; i<8; i++){
			ESP_LOGD(TAG, "wk2[%d]=%x", i, wk2[i]);
		}

		for(int i=0;i<8;i++) {
			if (dir == DIRECTION90) {
				buf[i] = wk2[i];
			} else {
				buf[i] = sh1107_rotate_byte(wk2[7-i]);
			}
		}
	} else if (dir == DIRECTION180) {
		for(int i=0;i<8;i++) {
			buf[i] = sh1107_rotate_byte(wk[7-i]);
		}

	}
	return;
}


void sh1107_fadeout(SH1107_t * dev)
{
	void (*func)(SH1107_t * dev, int page, int seg, uint8_t * images, int width);
	if (dev->_address == SPIAddress) {
		func = spi_display_image;
	} else {
		func = i2c_display_image;
	}

	uint8_t image[1];
	for(int page=0; page<dev->_pages; page++) {
		image[0] = 0xFF;
		for(int line=0; line<8; line++) {
			image[0] = image[0] << 1;
			for(int seg=0; seg<dev->_width; seg++) {
				//sh1107_display_image(dev, page, seg, image, 1);
				(*func)(dev, page, seg, image, 1);
			}
		}
	}
}

void sh1107_direction(SH1107_t * dev, int dir)
{
	dev->_direction = dir;
}

void sh1107_dump(SH1107_t dev)
{
	printf("_address=%x\n",dev._address);
	printf("_width=%x\n",dev._width);
	printf("_height=%x\n",dev._height);
	printf("_pages=%x\n",dev._pages);
}

void sh1107_dump_page(SH1107_t * dev, int page, int seg)
{
	ESP_LOGI(TAG, "dev->_page[%d]._segs[%d]=%02x", page, seg, dev->_page[page]._segs[seg]);
}
