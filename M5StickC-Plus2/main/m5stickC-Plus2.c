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

#include "sgm2578.h"
#include "st7789.h"
#include "fontx.h"

#define CONFIG_WIDTH 135
#define CONFIG_HEIGHT 240
#define CONFIG_OFFSETX 52
#define CONFIG_OFFSETY 40
#define CONFIG_MOSI_GPIO 15
#define CONFIG_SCLK_GPIO 13
#define CONFIG_CS_GPIO 5
#define CONFIG_DC_GPIO 14
#define CONFIG_RESET_GPIO 12
#define CONFIG_BL_GPIO -1
#define CONFIG_LED_GPIO 19
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
	// Power hold the SGM2578
	#define POWER_HOLD_GPIO 4
	gpio_reset_pin( POWER_HOLD_GPIO );
	gpio_set_direction( POWER_HOLD_GPIO, GPIO_MODE_OUTPUT );
	gpio_set_level( POWER_HOLD_GPIO, 1 );
	// Enable SGM2578. VLED is supplied by SGM2578
	#define SGM2578_ENABLE_GPIO 27
	sgm2578_Enable(SGM2578_ENABLE_GPIO);

	// set font file
	FontxFile fx[2];
#if CONFIG_FONT_8x16
    InitFontx(fx,"/spiffs/ILGH16XB.FNT",""); // 8x16Dot Gothic
#endif
#if CONFIG_FONT_12x24
    InitFontx(fx,"/spiffs/ILGH24XB.FNT",""); // 12x24Dot Gothic
#endif
#if CONFIG_FONT_16x32
    InitFontx(fx,"/spiffs/ILGH32XB.FNT",""); // 16x32Dot Gothic
#endif

	// get font width & height
	uint8_t fontWidth;
	uint8_t fontHeight;
	GetFontx(fx, 0, &fontWidth, &fontHeight);
	ESP_LOGI(pcTaskGetName(NULL), "fontWidth=%d fontHeight=%d",fontWidth,fontHeight);

	// Initialize save area
	typedef struct {
		bool enable;
		uint16_t color;;
		char line[DISPLAY_LENGTH+1];
	} SAVE_t;

	int maxRow = (CONFIG_HEIGHT - fontHeight) / fontHeight;
	ESP_LOGI(pcTaskGetName(NULL), "CONFIG_HEIGHT=%d fontHeight=%d maxRow=%d", CONFIG_HEIGHT, fontHeight, maxRow);
	SAVE_t save[maxRow];
	for(int i=0;i<maxRow;i++) {
		save[i].enable = false;
		memset(save[i].line, 0, DISPLAY_LENGTH+1);
	}

	// Setup Screen
	TFT_t dev;
	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
	lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);
	ESP_LOGI(pcTaskGetName(NULL), "Setup Screen done");
	lcdEnableFrameBuffer(&dev);

	// Initial Screen
	uint16_t color = CYAN;
	uint8_t ascii[DISPLAY_LENGTH+1];
	lcdFillScreen(&dev, BLACK);
	lcdSetFontDirection(&dev, 0);
	strcpy((char *)ascii, "DISTANCE  ");
	lcdDrawString(&dev, fx, 0, fontHeight-1, ascii, RED);
	lcdDrawFinish(&dev);

	bool enabled = false;
	int currentRow = 0;
	CMD_t cmdBuf;

	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGD(pcTaskGetName(NULL),"cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_START) {
			enabled = true;
			strcpy((char *)ascii, "DISTANCE  ");
			lcdDrawString(&dev, fx, 0, fontHeight-1, ascii, BLACK);
			lcdDrawString(&dev, fx, 0, fontHeight-1, ascii, YELLOW);
			lcdDrawFinish(&dev);

		} else if (cmdBuf.command == CMD_STOP) {
			enabled = false;
			strcpy((char *)ascii, "DISTANCE  ");
			lcdDrawString(&dev, fx, 0, fontHeight-1, ascii, BLACK);
			lcdDrawString(&dev, fx, 0, fontHeight-1, ascii, RED);
			lcdDrawFinish(&dev);

		} else if (cmdBuf.command == CMD_CLEAR) {
			if (enabled) continue;
			for(int i=0;i<maxRow;i++) {
				save[i].enable = false;
				memset(save[i].line, 0, DISPLAY_LENGTH+1);
				ESP_LOGD(pcTaskGetName(NULL), "%d enable=%d line=[%s]",i, save[i].enable, save[i].line);
				lcdDrawFillRect(&dev, 0, fontHeight*(i+1), CONFIG_WIDTH-1, fontHeight*(i+2)-1, BLACK);
			}
			lcdDrawFinish(&dev);
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
				lcdDrawFillRect(&dev, 0, fontHeight*(i+1), CONFIG_WIDTH-1, fontHeight*(i+2)-1, BLACK);
				lcdDrawString(&dev, fx, 0, fontHeight*(i+2)-1, (uint8_t *)save[i].line, save[i].color);
			}
			lcdDrawFinish(&dev);
		}
	}

	// Never reach here
	vTaskDelete(NULL);
}

