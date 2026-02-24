#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "cmd.h"
#include "ultrasonic.h"

#if defined(M5STACK)
#define CONFIG_STACK 1
#elif defined(M5STICK)
#define CONFIG_STICK 1
#elif defined(M5STICK_C)
#define CONFIG_STICKC 1
#elif defined(M5STICK_C_PLUS)
#define CONFIG_STICKC_PLUS 1
#elif defined(M5STICK_C_PLUS2)
#define CONFIG_STICKC_PLUS2 1
#endif

#if CONFIG_STACK
#define GPIO_TRIGGER 22
#define GPIO_ECHO 21
#endif

#if CONFIG_STICK
#define GPIO_TRIGGER 13
#define GPIO_ECHO 25
#endif

#if CONFIG_STICKC
#define GPIO_TRIGGER 33
#define GPIO_ECHO 32
#endif

#if CONFIG_STICKC_PLUS
#define GPIO_TRIGGER 33
#define GPIO_ECHO 32
#endif

#if CONFIG_STICKC_PLUS2
#define GPIO_TRIGGER 33
#define GPIO_ECHO 32
#endif

QueueHandle_t xQueueCmd;

static const char *TAG = "MAIN";

#define MAX_DISTANCE_CM 500 // 5m max

void ultrasonic(void *pvParamters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start. GPIO_TRIGGER=%d GPIO_ECHO=%d", GPIO_TRIGGER, GPIO_ECHO);
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
			printf("Distance: %"PRIu32" cm, %.02f m\n", distance, distance / 100.0);
			cmdBuf.distance = distance;
			if (xQueueSend(xQueueCmd, &cmdBuf, 0) != pdPASS) {
				ESP_LOGE(pcTaskGetName(NULL), "xQueueSend fail");
				break;
			}
		}
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void listSPIFFS(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}

esp_err_t mountSPIFFS(char * path, char * label, int max_files) {
	esp_vfs_spiffs_conf_t conf = {
		.base_path = path,
		.partition_label = label,
		.max_files = max_files,
		.format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret ==ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret== ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return ret;
	}

#if 0
	ESP_LOGI(TAG, "Performing SPIFFS_check().");
	ret = esp_spiffs_check(conf.partition_label);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
		return ret;
	} else {
			ESP_LOGI(TAG, "SPIFFS_check() successful");
	}
#endif

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Mount %s to %s success", path, label);
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}

	return ret;
}

void buttonA(void *pvParameters);
void buttonB(void *pvParameters);
void buttonC(void *pvParameters);
void tft(void *pvParameters);

void app_main()
{
#if CONFIG_STACK || CONFIG_STICKC || CONFIG_STICKC_PLUS || CONFIG_STICKC_PLUS2
	ESP_LOGI(TAG, "Mount SPIFFS");
	ESP_ERROR_CHECK(mountSPIFFS("/spiffs", "storage", 6));
	listSPIFFS("/spiffs/");
#endif

	// Create Queue
	xQueueCmd = xQueueCreate( 10, sizeof(CMD_t) );
	configASSERT( xQueueCmd );

	// Start task
	xTaskCreate(buttonA, "BUTTON-A", 1024*2, NULL, 2, NULL);
	xTaskCreate(buttonB, "BUTTON-B", 1024*2, NULL, 2, NULL);
	xTaskCreate(buttonC, "BUTTON-C", 1024*2, NULL, 2, NULL);
	xTaskCreate(ultrasonic, "ultrasonic", 1024*2, NULL, 2, NULL);
	xTaskCreate(tft, "TFT", 1024*4, NULL, 2, NULL);
}
