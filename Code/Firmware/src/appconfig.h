#define FIRMWARE_UPDATE_BASE_URL "http://un-connhome-api-dev.azurewebsites.net/api"
#define FIRMWARE_UPDATE_API_USERNAME "test"
#define FIRMWARE_UPDATE_API_PASSWORD "test"

#define STRIP_LED_COUNT 60

#define FREQUENCY_IN_SECONDS_FIRMWARE_CHECK 30000L

#define CHIP_DIGITAL_PIN_SENSOR_POWER       D2
#define CHIP_DIGITAL_PIN_STATUS_LED         D4 // (GPIOxx)
#define CHIP_DIGITAL_PIN_BUZZER             D6 // (GPIOxx)
#define CHIP_ANALOG_PIN_SENSOR              A0


#define BLYNK_VIRTUAL_PIN_OUT_MAC                 0    //V0
#define BLYNK_VIRTUAL_PIN_OUT_DEVICE              1    //V1   
#define BLYNK_VIRTUAL_PIN_OUT_FW_VERSION          2    //V2
#define BLYNK_VIRTUAL_PIN_OUT_LATEST_FW_VERSION   3    //V2
#define BLYNK_VIRTUAL_PIN_IN_UPDATE_FIRMWARE      4    //V3
#define BLYNK_VIRTUAL_PIN_IN_ENABLE_AUTO_FIRMWARE 5    //V4

#define BLYNK_VIRTUAL_PIN_IN_ENABLE_ALARM         6    //V5
#define BLYNK_VIRTUAL_PIN_IN_ENABLE_STATUS_LED    7    //V6
#define BLYNK_VIRTUAL_PIN_OUT_WIDGET_LED_MOIS     8    //V7

#define BLYNK_VIRTUAL_PIN_IN_STRIP_ON_OFF           10    //V3
#define BLYNK_VIRTUAL_PIN_IN_STRIP_RED   11    //V4
#define BLYNK_VIRTUAL_PIN_IN_STRIP_GREEN   12    //V0 
#define BLYNK_VIRTUAL_PIN_IN_STRIP_BLUE       13    //V1
#define BLYNK_VIRTUAL_PIN_IN_STRIP_PATTERNS       14    //V2

#define BLYNK_AUTH_TOKEN "<BLYNK_AUTH_TOKEN>" 
#define WIFI_USERNAME "<WIFI_USERNAME>"
#define WIFI_PASSWORD "<WIFI_PASSWORD>"