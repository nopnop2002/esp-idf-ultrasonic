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

#include "axp192.h"
#include "st7735s.h"
#include "fontx.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 160
#define OFFSET_X 26
#define OFFSET_Y 1
#define GPIO_MOSI 15
#define GPIO_SCLK 13
#define GPIO_CS 5
#define GPIO_DC 23
#define GPIO_RESET 18
#define DISPLAY_LENGTH 10
#define GPIO_INPUT_A GPIO_NUM_37
#define GPIO_INPUT_B GPIO_NUM_39

extern QueueHandle_t xQueueCmd;

void buttonA(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	CMD_t cmdBuf;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_A);
	gpio_set_direction(GPIO_INPUT_A, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_A);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(NULL), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_A);
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
	CMD_t cmdBuf;
	cmdBuf.command = CMD_CLEAR;
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
	while(1) {
		vTaskDelay(100);
	}
	vTaskDelete(NULL);
}

void tft(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	// Power on
	AXP192_Initialize(I2C_NUM_0);
	AXP192_PowerOn();
	AXP192_ScreenBreath(11);

	// set font file
	FontxFile fxG[2];
	InitFontx(fxG,"/spiffs/ILGH16XB.FNT",""); // 8x16Dot Gothic
	FontxFile fxM[2];
	InitFontx(fxM,"/spiffs/ILMH16XB.FNT",""); // 8x16Dot Mincyo

	// get font width & height
	uint8_t buffer[FontxGlyphBufSize];
	uint8_t fontWidth;
	uint8_t fontHeight;
	GetFontx(fxG, 0, buffer, &fontWidth, &fontHeight);
	ESP_LOGI(pcTaskGetName(NULL), "fontWidth=%d fontHeight=%d",fontWidth,fontHeight);

	// Initialize save area
	typedef struct {
		bool enable;
		uint16_t color;;
		char line[DISPLAY_LENGTH+1];
	} SAVE_t;

	int maxRow = (SCREEN_HEIGHT - fontHeight) / fontHeight;
	ESP_LOGI(pcTaskGetName(NULL), "SCREEN_HEIGHT=%d fontHeight=%d maxRow=%d", SCREEN_HEIGHT, fontHeight, maxRow);
	SAVE_t save[maxRow];
	for(int i=0;i<maxRow;i++) {
		save[i].enable = false;
		memset(save[i].line, 0, DISPLAY_LENGTH+1);
	}

	// Setup Screen
	ST7735_t dev;
	spi_master_init(&dev, GPIO_MOSI, GPIO_SCLK, GPIO_CS, GPIO_DC, GPIO_RESET);
	lcdInit(&dev, SCREEN_WIDTH, SCREEN_HEIGHT, OFFSET_X, OFFSET_Y);
	ESP_LOGI(pcTaskGetName(NULL), "Setup Screen done");

	// Initial Screen
	uint16_t color = CYAN;
	uint8_t ascii[DISPLAY_LENGTH+1];
	lcdFillScreen(&dev, BLACK);
	lcdSetFontDirection(&dev, 0);
	strcpy((char *)ascii, "DISTANCE  ");
	lcdDrawString(&dev, fxG, 0, fontHeight-1, ascii, RED);

	bool enabled = false;
	int currentRow = 0;
	CMD_t cmdBuf;

	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGD(pcTaskGetName(NULL),"cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_START) {
			enabled = true;
			strcpy((char *)ascii, "DISTANCE  ");
			lcdDrawString(&dev, fxG, 0, fontHeight-1, ascii, BLACK);
			lcdDrawString(&dev, fxG, 0, fontHeight-1, ascii, YELLOW);

		} else if (cmdBuf.command == CMD_STOP) {
			enabled = false;
			strcpy((char *)ascii, "DISTANCE  ");
			lcdDrawString(&dev, fxG, 0, fontHeight-1, ascii, BLACK);
			lcdDrawString(&dev, fxG, 0, fontHeight-1, ascii, RED);

		} else if (cmdBuf.command == CMD_CLEAR) {
			if (enabled) continue;
			for(int i=0;i<maxRow;i++) {
				save[i].enable = false;
				memset(save[i].line, 0, DISPLAY_LENGTH+1);
				ESP_LOGD(pcTaskGetName(NULL), "%d enable=%d line=[%s]",i, save[i].enable, save[i].line);
				lcdDrawFillRect(&dev, 0, fontHeight*(i+1), SCREEN_WIDTH-1, fontHeight*(i+2)-1, BLACK);
			}
			currentRow = 0;

		} else if (cmdBuf.command == CMD_MEASURE) {
			if (!enabled) continue;
			sprintf((char *)ascii, "%"PRIu32" cm", cmdBuf.distance);
			if (currentRow < maxRow) {
				ESP_LOGD(pcTaskGetName(NULL), "currentRow=%d maxRow=%d", currentRow, maxRow);
				save[currentRow].enable = true;
				save[currentRow].color = color;
				strcpy(save[currentRow].line, (char*)ascii);
				currentRow++;
			} else {
				for(int j=0;j<maxRow-1;j++) {
					save[j].enable = save[j+1].enable;
					save[j].color = save[j+1].color;
					strcpy(save[j].line, save[j+1].line);
				}
				save[maxRow-1].enable = true;
				save[maxRow-1].color = color;
				strcpy(save[maxRow-1].line, (char*)ascii);
			}
				
			for(int i=0;i<maxRow;i++) {
				ESP_LOGD(pcTaskGetName(NULL), "%d enable=%d line=[%s]",i, save[i].enable, save[i].line);
				if (save[i].enable == 0) continue;
				lcdDrawFillRect(&dev, 0, fontHeight*(i+1), SCREEN_WIDTH-1, fontHeight*(i+2)-1, BLACK);
				lcdDrawString(&dev, fxM, 0, fontHeight*(i+2)-1, (uint8_t *)save[i].line, save[i].color);
			}
		}
	}

	// Never reach here
	vTaskDelete(NULL);
}

