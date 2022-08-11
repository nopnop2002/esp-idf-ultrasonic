#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "ultrasonic.h"
#include "driver/dac.h"

#define MAX_DISTANCE_CM 500 // 5m max

#define CONFIG_STACK    0
#define CONFIG_STICKC   1
#define CONFIG_STICK    0

#if CONFIG_STACK
#include "ili9340.h"
#include "fontx.h"
#endif

#if CONFIG_STICKC
#include "axp192.h"
#include "st7735s.h"
#include "fontx.h"
#endif

#if CONFIG_STICK
#include "sh1107.h"
#include "font8x8_basic.h"
#endif

#if CONFIG_STACK
#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	240
#define CS_GPIO		14
#define DC_GPIO		27
#define RESET_GPIO	33
#define BL_GPIO		32
#define DISPLAY_LENGTH	26
#define GPIO_INPUT_A	GPIO_NUM_39
#define GPIO_INPUT_B	GPIO_NUM_38
#define GPIO_INPUT_C	GPIO_NUM_37
#define GPIO_TRIGGER	22
#define GPIO_ECHO	21
#endif


#if CONFIG_STICKC
#define SCREEN_WIDTH	80
#define SCREEN_HEIGHT	160
#define DISPLAY_LENGTH	10
#define GPIO_INPUT	GPIO_NUM_37
#define GPIO_TRIGGER	33
#define GPIO_ECHO	32
#endif

#if CONFIG_STICK
#define DISPLAY_LENGTH	8
#define GPIO_INPUT	GPIO_NUM_35
#define GPIO_BUZZER	GPIO_NUM_26
#define GPIO_TRIGGER	13
#define GPIO_ECHO	25
#endif

#define CMD_START	100
#define CMD_STOP	200
#define CMD_MEASURE	300
#define CMD_CLEAR	400

QueueHandle_t xQueueCmd;

static const char *TAG = "ULTRASONIC";

typedef struct {
	uint16_t command;
	uint32_t distance;
	TaskHandle_t taskHandle;
} CMD_t;

void ultrasonic(void *pvParamters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.command = CMD_MEASURE;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	ultrasonic_sensor_t sensor = {
		.trigger_pin = GPIO_TRIGGER,
		.echo_pin = GPIO_ECHO
	};

	ultrasonic_init(&sensor);

	while (true) {
		uint32_t distance;
		esp_err_t res = ultrasonic_measure_cm(&sensor, MAX_DISTANCE_CM, &distance);
		if (res != ESP_OK) {
			printf("Error: ");
			switch (res) {
				case ESP_ERR_ULTRASONIC_PING:
					printf("Cannot ping (device is in invalid state)\n");
					break;
				case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
					printf("Ping timeout (no device found)\n");
					break;
				case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
					printf("Echo timeout (i.e. distance too big)\n");
					break;
				default:
					printf("%d\n", res);
			}
		} else {
			printf("Distance: %d cm, %.02f m\n", distance, distance / 100.0);
			cmdBuf.distance = distance;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}


#if CONFIG_STICK || CONFIG_STICKC
void buttonStick(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT);
	gpio_set_direction(GPIO_INPUT, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			ESP_LOGI(pcTaskGetName(0),"diffTick=%d",diffTick);
			cmdBuf.command = CMD_START;
			if (diffTick > 200) cmdBuf.command = CMD_STOP;
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}
#endif

#if CONFIG_STACK
void buttonA(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.command = CMD_START;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_A);
	gpio_set_direction(GPIO_INPUT_A, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_A);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			while(1) {
				level = gpio_get_level(GPIO_INPUT_A);
				if (level == 1) break;
				vTaskDelay(1);
			}
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}

void buttonB(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.command = CMD_STOP;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_B);
	gpio_set_direction(GPIO_INPUT_B, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_B);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			while(1) {
				level = gpio_get_level(GPIO_INPUT_B);
				if (level == 1) break;
				vTaskDelay(1);
			}
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}

void buttonC(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	CMD_t cmdBuf;
	cmdBuf.command = CMD_CLEAR;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_C);
	gpio_set_direction(GPIO_INPUT_C, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_C);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			while(1) {
				level = gpio_get_level(GPIO_INPUT_C);
				if (level == 1) break;
				vTaskDelay(1);
			}
			xQueueSend(xQueueCmd, &cmdBuf, 0);
		}
		vTaskDelay(1);
	}
}
#endif

#if CONFIG_STACK
void tft(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
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
	ESP_LOGI(pcTaskGetName(0), "fontWidth=%d fontHeight=%d",fontWidth,fontHeight);

	// Setup Screen
	TFT_t dev;
	spi_master_init(&dev, CS_GPIO, DC_GPIO, RESET_GPIO, BL_GPIO);
	lcdInit(&dev, 0x9341, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	ESP_LOGI(pcTaskGetName(0), "Setup Screen done");

	int lines = (SCREEN_HEIGHT - fontHeight) / fontHeight;
	ESP_LOGD(pcTaskGetName(0), "SCREEN_HEIGHT=%d fontHeight=%d lines=%d", SCREEN_HEIGHT, fontHeight, lines);
	int ymax = (lines+1) * fontHeight;
	ESP_LOGD(pcTaskGetName(0), "ymax=%d",ymax);

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
		ESP_LOGI(pcTaskGetName(0),"cmdBuf.command=%d", cmdBuf.command);
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
			sprintf((char *)ascii, "%d cm", cmdBuf.distance);
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

	// nerver reach
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
#endif

#if CONFIG_STICKC
void tft(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
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
	ESP_LOGI(pcTaskGetName(0), "fontWidth=%d fontHeight=%d",fontWidth,fontHeight);

	// Initialize save area
	typedef struct {
		bool enable;
		uint16_t color;;
		char line[DISPLAY_LENGTH+1];
	} SAVE_t;

	int lines = (SCREEN_HEIGHT - fontHeight) / fontHeight;
	ESP_LOGD(pcTaskGetName(0), "SCREEN_HEIGHT=%d fontHeight=%d lines=%d", SCREEN_HEIGHT, fontHeight, lines);
	SAVE_t save[10];
	for(int i=0;i<lines;i++) {
		save[i].enable = false;
	}

	// Setup Screen
	ST7735_t dev;
	spi_master_init(&dev);
	lcdInit(&dev, SCREEN_WIDTH, SCREEN_HEIGHT);
	ESP_LOGI(pcTaskGetName(0), "Setup Screen done");

	// Initial Screen
	uint16_t color = CYAN;
	uint8_t ascii[DISPLAY_LENGTH+1];
	lcdFillScreen(&dev, BLACK);
	lcdSetFontDirection(&dev, 0);
	strcpy((char *)ascii, "DISTANCE  ");
	lcdDrawString(&dev, fxG, 0, fontHeight-1, ascii, RED);

	bool enabled = false;
	CMD_t cmdBuf;

	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(0),"cmdBuf.command=%d", cmdBuf.command);
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

		} else if (cmdBuf.command == CMD_MEASURE) {
			if (!enabled) continue;
			sprintf((char *)ascii, "%d cm", cmdBuf.distance);
			int last = -1;
			bool redraw = false;
			for(int j=(lines-1);j>=0;j--) {
				if (save[j].enable == false) last = j;
			}
			if (last == -1) {
				last = lines-1;
				redraw = true;
				for(int j=0;j<lines-1;j++) {
					save[j].enable = save[j+1].enable;
					save[j].color = save[j+1].color;
					strcpy(save[j].line, save[j+1].line);
				}
			}
			save[last].enable = true;
			save[last].color = color;
			strcpy(save[last].line,  (char*)ascii);
		
			if (redraw) {
				//lcdDrawFillRect(dev, 0, fontHeight, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
				for(int j=0;j<lines;j++) {
					ESP_LOGD(pcTaskGetName(0), "enable[%d]=%d",j, save[j].enable);
					lcdDrawFillRect(&dev, 0, fontHeight*(j+1), SCREEN_WIDTH-1, fontHeight*(j+2)-1, BLACK);
					lcdDrawString(&dev, fxM, 0, fontHeight*(j+2)-1, (uint8_t *)save[j].line, save[j].color);
				}
			} else {
				lcdDrawString(&dev, fxM, 0, fontHeight*(last+2)-1, (uint8_t *)save[last].line, save[last].color);
			}
		}
	}

	// nerver reach
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
#endif


#if CONFIG_STICK
void tft(void *pvParameters)
{
	// Setup Screen
	SH1107_t dev;
	spi_master_init(&dev);
	spi_init(&dev, 64, 128);
	ESP_LOGI(pcTaskGetName(0), "Setup Screen done");

	// Initial Screen
	char ascii[DISPLAY_LENGTH+1];
	clear_screen(&dev, false);
	display_contrast(&dev, 0xff);
	display_text(&dev, 0, "DISTANCE", 8, false);
	software_scroll(&dev, 1, 15);

	bool enabled = false;
	CMD_t cmdBuf;

	while(1) {
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(0),"cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_START) {
			enabled = true;
			display_text(&dev, 0, "DISTANCE", 8, true);

		} else if (cmdBuf.command == CMD_STOP) {
			enabled = false;
			display_text(&dev, 0, "DISTANCE", 8, false);

		} else if (cmdBuf.command == CMD_MEASURE) {
			if (!enabled) continue;
			sprintf(ascii, "%d cm", cmdBuf.distance);
			scroll_text(&dev, ascii, strlen((char *)ascii), false);
		}
	}

	// nerver reach
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
#endif

#if CONFIG_STACK || CONFIG_STICKC
static void SPIFFS_Directory(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(TAG,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}
#endif

void app_main()
{
#if CONFIG_STACK || CONFIG_STICKC

	ESP_LOGI(TAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 6,
		.format_if_mount_failed =true
	};

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is anall-in-one convenience function.
	esp_err_t ret =esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret ==ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret== ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}

	SPIFFS_Directory("/spiffs");

#endif

	/* Create Queue */
	xQueueCmd = xQueueCreate( 10, sizeof(CMD_t) );
	configASSERT( xQueueCmd );

#if CONFIG_STACK
	xTaskCreate(buttonA, "START", 1024*2, NULL, 2, NULL);
	xTaskCreate(buttonB, "STOP", 1024*2, NULL, 2, NULL);
	xTaskCreate(buttonC, "CLEAR", 1024*2, NULL, 2, NULL);
#endif

#if CONFIG_STICKC
	// Power on
	i2c_master_init();
	AXP192_PowerOn();
	xTaskCreate(buttonStick, "BUTTON", 1024*2, NULL, 2, NULL);
#endif

#if CONFIG_STICK
	// Disable DAC channel 1 (GPIO 25)
	dac_output_disable(DAC_CHANNEL_1);
	xTaskCreate(buttonStick, "BUTTON", 1024*2, NULL, 2, NULL);
#endif

	xTaskCreate(ultrasonic, "ultrasonic", 1024*2, NULL, 2, NULL);
	xTaskCreate(tft, "TFT", 1024*4, NULL, 2, NULL);
}

