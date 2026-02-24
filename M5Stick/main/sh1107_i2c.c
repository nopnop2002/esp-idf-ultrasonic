#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "sh1107.h"

#define TAG "SH1107"

#define I2C_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000 /*!< I2C master clock frequency. no higher than 1MHz for now */

void i2c_master_init(SH1107_t * dev, int16_t sda, int16_t scl, int16_t reset)
{
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = sda,
		.scl_io_num = scl,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_FREQ_HZ
	};
	i2c_param_config(I2C_NUM, &i2c_config);
	i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, 0);

	if (reset >= 0) {
		//gpio_pad_select_gpio(reset);
		gpio_reset_pin(reset);
		gpio_set_direction(reset, GPIO_MODE_OUTPUT);
		gpio_set_level(reset, 0);
		vTaskDelay(50 / portTICK_PERIOD_MS);
		gpio_set_level(reset, 1);
	}
	dev->_address = I2CAddress;
}

void i2c_init(SH1107_t * dev, int width, int height)
{
	dev->_width = width;
	dev->_height = height;
	dev->_pages = height / 8;
	dev->_direction = DIRECTION0;
	
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, I2C_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0xAE, true);		// Turn display off
	i2c_master_write_byte(cmd, 0xDC, true);		// Set display start line
	i2c_master_write_byte(cmd, 0x00, true);		// ...value
	i2c_master_write_byte(cmd, 0x81, true);		// Set display contrast
	i2c_master_write_byte(cmd, 0x2F, true);		// ...value
	i2c_master_write_byte(cmd, 0x20, true);		// Set memory mode
	i2c_master_write_byte(cmd, 0xA0, true);		// Non-rotated display
	i2c_master_write_byte(cmd, 0xC0, true);		// Non-flipped vertical
	i2c_master_write_byte(cmd, 0xA8, true);		// Set multiplex ratio
	i2c_master_write_byte(cmd, 0x7F, true);		// ...value
	i2c_master_write_byte(cmd, 0xD3, true);		// Set display offset to zero
	i2c_master_write_byte(cmd, 0x60, true);		// ...value
	i2c_master_write_byte(cmd, 0xD5, true);		// Set display clock divider
	i2c_master_write_byte(cmd, 0x51, true);		// ...value
	i2c_master_write_byte(cmd, 0xD9, true);		// Set pre-charge
	i2c_master_write_byte(cmd, 0x22, true);		// ...value
	i2c_master_write_byte(cmd, 0xDB, true);		// Set com detect
	i2c_master_write_byte(cmd, 0x35, true);		// ...value
	i2c_master_write_byte(cmd, 0xB0, true);		// Set page address
	i2c_master_write_byte(cmd, 0xDA, true);		// Set com pins
	i2c_master_write_byte(cmd, 0x12, true);		// ...value
	i2c_master_write_byte(cmd, 0xA4, true);		// output ram to display
	i2c_master_write_byte(cmd, 0xA6, true);		// Non-inverted display
	//i2c_master_write_byte(cmd, 0xA7, true);	// Inverted display
	i2c_master_write_byte(cmd, 0xAF, true);		// Turn display on

	i2c_master_stop(cmd);

	esp_err_t espRc = i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(TAG, "OLED configured successfully");
	} else {
		ESP_LOGE(TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}


void i2c_display_image(SH1107_t * dev, int page, int seg, uint8_t * images, int width)
{
	i2c_cmd_handle_t cmd;

	if (page >= dev->_pages) return;
	if (seg >= dev->_width) return;

	uint8_t columLow = seg & 0x0F;
	uint8_t columHigh = (seg >> 4) & 0x0F;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, I2C_CONTROL_BYTE_CMD_STREAM, true);
	// Set Higher Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x10 + columHigh), true);
	// Set Lower Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x00 + columLow), true);
	// Set Page Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, 0xB0 | page, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, I2C_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, images, width, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

void i2c_contrast(SH1107_t * dev, int contrast)
{
	i2c_cmd_handle_t cmd;
	int _contrast = contrast;
	if (contrast < 0x0) _contrast = 0;
	if (contrast > 0xFF) _contrast = 0xFF;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, I2C_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x81, true);		// Set Contrast Control Register
	i2c_master_write_byte(cmd, _contrast, true);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}
