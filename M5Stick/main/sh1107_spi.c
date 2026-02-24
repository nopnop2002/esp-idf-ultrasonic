#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"

#include "sh1107.h"

#define TAG "SH1107"

#ifdef CONFIG_IDF_TARGET_ESP32
#define LCD_HOST HSPI_HOST
#elif defined CONFIG_IDF_TARGET_ESP32S2
#define LCD_HOST SPI2_HOST
#elif defined CONFIG_IDF_TARGET_ESP32S3
#define LCD_HOST SPI2_HOST
#elif defined CONFIG_IDF_TARGET_ESP32C3
#define LCD_HOST SPI2_HOST
#endif

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode = 1;
//static const int SPI_Frequency = 1000000;
static const int SPI_Frequency = 8000000;

void spi_master_init(SH1107_t * dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RESET)
{
	esp_err_t ret;

	gpio_reset_pin( GPIO_CS );
	ret = gpio_set_direction( GPIO_CS, GPIO_MODE_OUTPUT );
	ESP_LOGI(TAG, "gpio_set_direction(GPIO_CS)=%d",ret);
	assert(ret==ESP_OK);
	gpio_set_level( GPIO_CS, 1 );

	gpio_reset_pin( GPIO_DC );
	ret = gpio_set_direction( GPIO_DC, GPIO_MODE_OUTPUT );
	ESP_LOGI(TAG, "gpio_set_direction(GPIO_DC)=%d",ret);
	assert(ret==ESP_OK);
	gpio_set_level( GPIO_DC, 0 );

	if (GPIO_RESET >= 0) {
		gpio_reset_pin( GPIO_RESET );
		ret = gpio_set_direction( GPIO_RESET, GPIO_MODE_OUTPUT );
		ESP_LOGI(TAG, "gpio_set_direction(GPIO_RESET)=%d",ret);
		assert(ret==ESP_OK);
		gpio_set_level( GPIO_RESET, 0 );
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
		gpio_set_level( GPIO_RESET, 1 );
	}

	spi_bus_config_t spi_bus_config = {
		.sclk_io_num = GPIO_SCLK,
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = -1,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
	};

	ret = spi_bus_initialize( LCD_HOST, &spi_bus_config, SPI_DMA_CH_AUTO );
	ESP_LOGI(TAG, "spi_bus_initialize=%d",ret);
	assert(ret==ESP_OK);

	spi_device_interface_config_t devcfg;
	memset( &devcfg, 0, sizeof( spi_device_interface_config_t ) );
	devcfg.clock_speed_hz = SPI_Frequency;
	devcfg.spics_io_num = GPIO_CS;
	devcfg.queue_size = 1;

	spi_device_handle_t handle;
	ret = spi_bus_add_device( LCD_HOST, &devcfg, &handle);
	ESP_LOGI(TAG, "spi_bus_add_device=%d",ret);
	assert(ret==ESP_OK);
	dev->_dc = GPIO_DC;
	dev->_SPIHandle = handle;
	dev->_address = SPIAddress;
}


bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t* Data, size_t DataLength )
{
	spi_transaction_t SPITransaction;
	esp_err_t ret;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Data;
		ret = spi_device_transmit( SPIHandle, &SPITransaction );
		//ESP_LOGI(TAG, "spi_device_transmit=%d",ret);
		assert(ret==ESP_OK);
	}

	return true;
}

bool spi_master_write_command(SH1107_t * dev, uint8_t Command )
{
	static uint8_t CommandByte = 0;
	CommandByte = Command;
	gpio_set_level( dev->_dc, SPI_Command_Mode );
	return spi_master_write_byte( dev->_SPIHandle, &CommandByte, 1 );
}

bool spi_master_write_data(SH1107_t * dev, const uint8_t* Data, size_t DataLength )
{
	gpio_set_level( dev->_dc, SPI_Data_Mode );
	return spi_master_write_byte( dev->_SPIHandle, Data, DataLength );
}

void spi_init(SH1107_t * dev, int width, int height)
{
	dev->_width = width;
	dev->_height = height;
	dev->_pages = height / 8;
	dev->_direction = DIRECTION0;

	spi_master_write_command(dev, 0xAE);	// Turn display off
	spi_master_write_command(dev, 0xDC);	// Set display start line
	spi_master_write_command(dev, 0x00);	// ...value
	spi_master_write_command(dev, 0x81);	// Set display contrast
	spi_master_write_command(dev, 0x2F);	// ...value
	spi_master_write_command(dev, 0x20);	// Set memory mode
	spi_master_write_command(dev, 0xA0);	// Non-rotated display
	spi_master_write_command(dev, 0xC0);	// Non-flipped vertical
	spi_master_write_command(dev, 0xA8);	// Set multiplex ratio
	spi_master_write_command(dev, 0x7F);	// ...value
	spi_master_write_command(dev, 0xD3);	// Set display offset to zero
	spi_master_write_command(dev, 0x60);	// ...value
	spi_master_write_command(dev, 0xD5);	// Set display clock divider
	spi_master_write_command(dev, 0x51);	// ...value
	spi_master_write_command(dev, 0xD9);	// Set pre-charge
	spi_master_write_command(dev, 0x22);	// ...value
	spi_master_write_command(dev, 0xDB);	// Set com detect
	spi_master_write_command(dev, 0x35);	// ...value
	spi_master_write_command(dev, 0xB0);	// Set page address
	spi_master_write_command(dev, 0xDA);	// Set com pins
	spi_master_write_command(dev, 0x12);	// ...value
	spi_master_write_command(dev, 0xA4);	// output ram to display
	spi_master_write_command(dev, 0xA6);	// Non-inverted display
	//spi_master_write_command(dev, 0xA7);	// Inverted display
	spi_master_write_command(dev, 0xAF);	// Turn display on
}

void spi_display_image(SH1107_t * dev, int page, int col, uint8_t * images, int width)
{
	if (page >= dev->_pages) return;
	if (col >= dev->_width) return;

	uint8_t columLow = col & 0x0F;
	uint8_t columHigh = (col >> 4) & 0x0F;

	// Set Higher Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, (0x10 + columHigh));
	// Set Lower Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, (0x00 + columLow));
	// Set Page Start Address for Page Addressing Mode
	spi_master_write_command(dev, 0xB0 | page);

	spi_master_write_data(dev, images, width);

}

void spi_contrast(SH1107_t * dev, int contrast) {
	int _contrast = contrast;
	if (contrast < 0x0) _contrast = 0;
	if (contrast > 0xFF) _contrast = 0xFF;

	spi_master_write_command(dev, 0x81);	// Set Contrast Control Register
	spi_master_write_command(dev, _contrast);
}
