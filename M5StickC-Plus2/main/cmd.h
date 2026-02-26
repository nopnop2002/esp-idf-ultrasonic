typedef enum {CMD_START, CMD_STOP, CMD_MEASURE, CMD_CLEAR} COMMAND;

typedef struct {
	uint16_t command;
	uint32_t distance;
	TaskHandle_t taskHandle;
} CMD_t;
