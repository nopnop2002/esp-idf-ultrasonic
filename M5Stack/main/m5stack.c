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

#include "ili9340.h"
#include "fontx.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define CONFIG_MOSI_GPIO 23
#define CONFIG_SCLK_GPIO 18
#define CONFIG_TFT_CS_GPIO 14
#define CONFIG_DC_GPIO 27
#define CONFIG_RESET_GPIO 33
#define CONFIG_BL_GPIO 32
#define DISPLAY_LENGTH 26
#define GPIO_INPUT_A GPIO_NUM_39
#define GPIO_INPUT_B GPIO_NUM_38
#define GPIO_INPUT_C GPIO_NUM_37

extern QueueHandle_t xQueueCmd;

void buttonA(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	CMD_t cmdBuf;
	cmdBuf.command = CMD_START;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_A);
	gpio_set_direction(GPIO_INPUT_A, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_A);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(NULL), "Push Button");
			while(1) {
				level = gpio_get_level(GPIO_INPUT_A);
				if (level == 1) break;
				vTaskDelay(1);
			}
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
	CMD_t cmdBuf;
	cmdBuf.command = CMD_STOP;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_B);
	gpio_set_direction(GPIO_INPUT_B, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_B);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(NULL), "Push Button");
			while(1) {
				level = gpio_get_level(GPIO_INPUT_B);
				if (level == 1) break;
				vTaskDelay(1);
			}
			if (xQueueSend(xQueueCmd, &cmdBuf, 0) != pdPASS) {
				ESP_LOGE(pcTaskGetName(NULL), "xQueueSend fail");
				break;
			}
		}
		vTaskDelay(1);
	}
	vTaskDelete(NULL);
}

void buttonC(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	CMD_t cmdBuf;
	cmdBuf.command = CMD_CLEAR;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_C);
	gpio_set_direction(GPIO_INPUT_C, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_C);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(NULL), "Push Button");
			while(1) {
				level = gpio_get_level(GPIO_INPUT_C);
				if (level == 1) break;
				vTaskDelay(1);
			}
			if (xQueueSend(xQueueCmd, &cmdBuf, 0) != pdPASS) {
				ESP_LOGE(pcTaskGetName(NULL), "xQueueSend fail");
				break;
			}
		}
		vTaskDelay(1);
	}
	vTaskDelete(NULL);
}

void tft(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	// set font file
	FontxFile fxG[2];
	InitFontx(fxG,"/spiffs/ILGH24XB.FNT",""); // 12x24Dot Gothic
	FontxFile fxM[2];
	InitFontx(fxM,"/spiffs/ILMH24XB.FNT",""); // 12x24Dot Mincyo

	// get font width & height
	uint8_t buffer[FontxGlyphBufSize];
	uint8_t fontWidth;
	uint8_t fontHeight;
	GetFontx(fxG, 0, buffer, &fontWidth, &fontHeight);
	ESP_LOGI(pcTaskGetName(NULL), "fontWidth=%d fontHeight=%d",fontWidth,fontHeight);

	// Setup Screen
	TFT_t dev;
	int MISO_GPIO = -1;
	int XPT_CS_GPIO = -1;
	int XPT_IRQ_GPIO = -1;
	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_TFT_CS_GPIO, CONFIG_DC_GPIO,
		CONFIG_RESET_GPIO, CONFIG_BL_GPIO, MISO_GPIO, XPT_CS_GPIO, XPT_IRQ_GPIO);
	lcdInit(&dev, 0x9341, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	ESP_LOGI(pcTaskGetName(NULL), "Setup Screen done");

	int lines = (SCREEN_HEIGHT - fontHeight) / fontHeight;
	ESP_LOGD(pcTaskGetName(NULL), "SCREEN_HEIGHT=%d fontHeight=%d lines=%d", SCREEN_HEIGHT, fontHeight, lines);
	int ymax = (lines+1) * fontHeight;
	ESP_LOGD(pcTaskGetName(NULL), "ymax=%d",ymax);

	// Initial Screen
	uint8_t ascii[DISPLAY_LENGTH+1];
	lcdFillScreen(&dev, BLACK);
	lcdSetFontDirection(&dev, 0);

	// Reset scroll area
	lcdSetScrollArea(&dev, 0, 0x0140, 0);

	strcpy((char *)ascii, "Ultrasonic DISTANCE");
	lcdDrawString(&dev, fxG, 0, fontHeight-1, ascii, RED);
	strcpy((char *)ascii, "Stop ");
	uint16_t xstatus = 20*fontWidth;
	lcdDrawString(&dev, fxG, xstatus, fontHeight-1, ascii, RED);

	uint16_t vsp = fontHeight*2;
	uint16_t ypos = (fontHeight*2) - 1;
	uint16_t current = 0;
	bool enabled = false;
	CMD_t cmdBuf;

	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGD(pcTaskGetName(NULL),"cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_START) {
			enabled = true;
			strcpy((char *)ascii, "Stop ");
			lcdDrawString(&dev, fxG, xstatus, fontHeight-1, ascii, BLACK);
			strcpy((char *)ascii, "Start");
			lcdDrawString(&dev, fxG, xstatus, fontHeight-1, ascii, BLUE);

		} else if (cmdBuf.command == CMD_STOP) {
			enabled = false;
			strcpy((char *)ascii, "Start");
			lcdDrawString(&dev, fxG, xstatus, fontHeight-1, ascii, BLACK);
			strcpy((char *)ascii, "Stop ");
			lcdDrawString(&dev, fxG, xstatus, fontHeight-1, ascii, RED);

		} else if (cmdBuf.command == CMD_CLEAR) {
			if (enabled) continue;
			lcdDrawFillRect(&dev, 0, fontHeight, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
			// Reset scroll area
			lcdSetScrollArea(&dev, 0, 0x0140, 0);
			vsp = fontHeight*2;
			ypos = (fontHeight*2) - 1;
			current = 0;

		} else if (cmdBuf.command == CMD_MEASURE) {
			if (!enabled) continue;
			sprintf((char *)ascii, "%"PRIu32" cm", cmdBuf.distance);
			if (current < lines) {
				lcdDrawString(&dev, fxM, 0, ypos, ascii, CYAN);
			} else {
				lcdDrawFillRect(&dev, 0, ypos-fontHeight, SCREEN_WIDTH-1, ypos, BLACK);
				lcdSetScrollArea(&dev, fontHeight, (SCREEN_HEIGHT-fontHeight), 0);
				lcdScroll(&dev, vsp);
				vsp = vsp + fontHeight;
				if (vsp > ymax) vsp = fontHeight*2;
				lcdDrawString(&dev, fxM, 0, ypos, ascii, CYAN);
			}
			current++;
			ypos = ypos + fontHeight;
			if (ypos > ymax) ypos = (fontHeight*2) - 1;
		}
	}

	// Never reach here
	vTaskDelete(NULL);
}
