#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "cmd.h"

#include "sh1107.h"
#include "font8x8_basic.h"

#define CONFIG_MOSI_GPIO 23
#define CONFIG_SCLK_GPIO 18
#define CONFIG_CS_GPIO 14
#define CONFIG_DC_GPIO 27
#define CONFIG_SPI_RESET_GPIO 33
#define DISPLAY_LENGTH 8
#define GPIO_INPUT GPIO_NUM_35
#define GPIO_BUZZER GPIO_NUM_26
#define GPIO_TRIGGER 13
#define GPIO_ECHO 25

extern QueueHandle_t xQueueCmd;

void buttonA(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	CMD_t cmdBuf;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT);
	gpio_set_direction(GPIO_INPUT, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(NULL), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			ESP_LOGI(pcTaskGetName(NULL),"diffTick=%"PRIu32, diffTick);
			cmdBuf.command = CMD_START;
			if (diffTick > 200) cmdBuf.command = CMD_STOP;
			if (xQueueSend(xQueueCmd, &cmdBuf, 0) != pdPASS) {
				ESP_LOGE(pcTaskGetName(NULL), "xQueueSend fail");
				break;
			}
		}
		vTaskDelay(1);
	}
	vTaskDelete(NULL);
}

void buttonB(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	while(1) {
		vTaskDelay(100);
	}
	vTaskDelete(NULL);
}

void buttonC(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	while(1) {
		vTaskDelay(100);
	}
	vTaskDelete(NULL);
}

void tft(void *pvParameters)
{
	// Allocate memory
	uint8_t *buffer = (uint8_t *)malloc(16*64); // 16 page 64 pixel
	if (buffer == NULL) {
		ESP_LOGE(pcTaskGetName(NULL), "malloc failed");
		while(1) { vTaskDelay(1); }
	}

	// Setup Screen
	SH1107_t dev;
	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_SPI_RESET_GPIO);
	sh1107_init(&dev, 64, 128);
	sh1107_contrast(&dev, 0xff);
	ESP_LOGI(pcTaskGetName(NULL), "Setup Screen done");

	// Initial Screen
	char ascii[DISPLAY_LENGTH+1];
	sh1107_clear_screen(&dev, false);
	sh1107_display_text(&dev, 0, 0, "DISTANCE", 8, false);

	bool enabled = false;
	int currentRow = 1;
	CMD_t cmdBuf;

	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGD(pcTaskGetName(NULL),"cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_START) {
			enabled = true;
			strcpy(ascii, "DISTANCE");
			sh1107_display_text(&dev, 0, 0, ascii, strlen(ascii), true);

		} else if (cmdBuf.command == CMD_STOP) {
			enabled = false;
			strcpy(ascii, "DISTANCE");
			sh1107_display_text(&dev, 0, 0, ascii, strlen(ascii), false);

		} else if (cmdBuf.command == CMD_MEASURE) {
			if (!enabled) continue;
			sprintf(ascii, "%"PRIu32" cm", cmdBuf.distance);
			if (currentRow < 16) {
				sh1107_display_text(&dev, currentRow, 0, ascii, strlen(ascii), false);
				currentRow++;
			} else {
				sh1107_get_buffer(&dev, buffer);
				int index = 64;
				for (int page=1;page<15;page++) {
					ESP_LOGD(pcTaskGetName(NULL), "index=%d", index);
					memcpy(&buffer[index], &buffer[index+64], 64);
					index = index + 64;
				}
				memset(&buffer[15*64], 0x00, 64);
#if 0
				index = 0;
				for (int page=0;page<16;page++) {
					ESP_LOGI("", "page=%d", page);
					ESP_LOG_BUFFER_HEXDUMP("", &buffer[index], 64, ESP_LOG_INFO);
					index = index + 64;
				}
#endif
				sh1107_set_buffer(&dev, buffer);
				sh1107_show_buffer(&dev);
				sh1107_display_text(&dev, 15, 0, ascii, strlen(ascii), false);
			}
		}
	}

	// Never reach here
	vTaskDelete(NULL);
}

