#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
#include <Ticker.h>

#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#include "appconfig.h"
#include "appinfo.h"

#pragma region[Variables]

bool isColorLedOn;
int redColorValue = 0;
int greenColorValue = 0;
int blueColorValue = 0;

int enableAutoFirmwareUpdate;

//Ticker stripTicker;
BlynkTimer timer;

HTTPClient httpClient; //Object of class HTTPClient
String apiToken;
String latestFirmwareFileUrl;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LED_COUNT, CHIP_DIGITAL_PIN_STATUS_LED, NEO_GRB + NEO_KHZ800);

#pragma endregion

#pragma region[Initialization]

void initializePins()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void initializeSerial()
{
  // Start serial and initialize stdout
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.print("Serial successfully initiated...");
  delay(2000);
}

void initiallizeWifiAndBlynk()
{
  // Config Blynk server
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_USERNAME, WIFI_PASSWORD);
  Serial.print("Blynk started...");
}

void initializeLocalOTA()
{
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA mode ready...");
  Serial.print("Use IP ");
  Serial.print(WiFi.localIP());
  Serial.println(" to upload new firmware");
}

void initializeEEPROM()
{
  // EEPROM.begin(512);

  // EEPROM.get(addrMoistureThresholdUpper, moistureThresholdUpper); //Get the upper moisture threshold
  // EEPROM.get(addrMoistureThresholdLower, moistureThresholdLower); //Get the upper moisture threshold
  // EEPROM.get(addrEnableAlarm, enableAlarm);
  // EEPROM.get(addrEnablestrip, enablestrip);
  // EEPROM.get(addrEnableAutoFirmwareUpdate, enableAutoFirmwareUpdate);
}

#pragma endregion

void setColor(int r, int g, int b)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, strip.Color(r, g, b));
    // OR: strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + shift) & 255));
  }
  strip.show();
}

void onOffLedStrip(bool on)
{
  isColorLedOn = on;
  if (on == true)
  {
    setColor(0, 0, 255);
  }
  else
  {
    setColor(0, 0, 0);
  }
  // Uncomment the following line, if a seperate RGB is powered by realy
  //digitalWrite(RELAY_TWO_PIN, param.asInt());
}

#pragma region[Blynk]

void updateBlynkCloud()
{
  // Send data to blynk cloud
  // Blynk.virtualWrite(BLYNK_VIRTUAL_PIN_OUT_MOIS_SENSOR_CURRENT, currentMoistureLevelRecorded);
  // Blynk.virtualWrite(BLYNK_VIRTUAL_PIN_OUT_MOIS_SENSOR_MIN, minMoistureLevelRecorded);
  // Blynk.virtualWrite(BLYNK_VIRTUAL_PIN_OUT_MOIS_SENSOR_MAX, maxMoistureLevelRecorded);
}

BLYNK_CONNECTED()
{
  Blynk.syncAll();
  Blynk.virtualWrite(BLYNK_VIRTUAL_PIN_OUT_MAC, WiFi.macAddress());
  Blynk.virtualWrite(BLYNK_VIRTUAL_PIN_OUT_DEVICE, DEVICE_TYPE);
  Blynk.virtualWrite(BLYNK_VIRTUAL_PIN_OUT_FW_VERSION, CURRENT_FIRMWARE_VERSION);
  Serial.println("Synced with Blynk server...");
}

BLYNK_WRITE(BLYNK_VIRTUAL_PIN_IN_STRIP_ON_OFF)
{
  onOffLedStrip(param.asInt());
}

// LED Color Light set rgb
BLYNK_WRITE(BLYNK_VIRTUAL_PIN_IN_STRIP_RED)
{
  if (isColorLedOn == true)
  {
    // get a RED channel value
    int r = param.asFloat();
    redColorValue = r;

    // get a GREEN channel value
    int g = greenColorValue;
    // get a BLUE channel value
    int b = blueColorValue;

    setColor(r, g, b);
  }
}

// LED Color Light set rgb
BLYNK_WRITE(BLYNK_VIRTUAL_PIN_IN_STRIP_GREEN)
{
  if (isColorLedOn == true)
  {
    // get a RED channel value
    int g = param.asFloat();
    greenColorValue = g;

    // get a GREEN channel value
    int r = redColorValue;
    // get a BLUE channel value
    int b = blueColorValue;

    setColor(r, g, b);
  }
}

// LED Color Light set rgb
BLYNK_WRITE(BLYNK_VIRTUAL_PIN_IN_STRIP_BLUE)
{
  if (isColorLedOn == true)
  {
    // get a RED channel value
    int b = param.asFloat();
    blueColorValue = b;

    // get a GREEN channel value
    int g = greenColorValue;
    // get a BLUE channel value
    int r = redColorValue;

    setColor(r, g, b);
  }
}

BLYNK_WRITE(BLYNK_VIRTUAL_PIN_IN_STRIP_PATTERNS)
{
  int patternId = param.asInt();
  if (isColorLedOn == true)
  {
    //int patternIndex = param.asInt();
    setPattern(patternId);
  }
}

#pragma endregion

#pragma region[Firmware Update]

void requestApiToken()
{
  String apiTokenRequest = String(FIRMWARE_UPDATE_BASE_URL);
  apiTokenRequest.concat("/users/authenticate");
  //HTTPClient client;
  httpClient.begin(apiTokenRequest);
  httpClient.addHeader("Content-Type", "application/json");
  httpClient.addHeader("accept", "*/*");

  Serial.print("Requesting API token from: ");
  Serial.println(apiTokenRequest.c_str());

  int statusCode = httpClient.POST("{ \"Username\": \"test\", \"Password\": \"test\",}");
  if (statusCode == 200)
  {
    const size_t capacity = JSON_OBJECT_SIZE(6) + 233;
    DynamicJsonDocument doc(capacity);
    String response = httpClient.getString();

    DeserializationError error = deserializeJson(doc, response);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    // Extract values
    JsonObject obj = doc.as<JsonObject>();
    Serial.print(F("Response Token: "));
    apiToken = obj["token"].as<String>();
    Serial.println(apiToken.c_str());
  }
  else
  {
    Serial.print("Firmware version check failed, got HTTP response code ");
    Serial.println(statusCode);
  }

  httpClient.end();
}

void updateFirmwareFromCloud()
{
  if (latestFirmwareFileUrl.length() > 0 && latestFirmwareFileUrl.endsWith(".bin"))
  {
    Serial.println("Preparing to update...");

    BearSSL::WiFiClientSecure UpdateClient;
    UpdateClient.setInsecure();

    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    t_httpUpdate_return ret = ESPhttpUpdate.update(UpdateClient, latestFirmwareFileUrl);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
    }
  }
}

void checkForNewFirmware()
{
  // Check if we have api token
  if (apiToken.length() == 0)
    requestApiToken();

  if (apiToken.length() == 0)
  {
    Serial.print("Authentication is required...");
    return;
  }

  Serial.println("Checking for firmware updates...");

  // Create http rest request
  String fwUpdateUrl = String(FIRMWARE_UPDATE_BASE_URL);
  fwUpdateUrl.concat("/updates?devicetype=");
  fwUpdateUrl.concat(DEVICE_TYPE);
  fwUpdateUrl.concat("&currentversion=");
  fwUpdateUrl.concat(CURRENT_FIRMWARE_VERSION);

  Serial.print("Requesting firmware updates from: ");
  Serial.println(fwUpdateUrl.c_str());

  httpClient.begin(fwUpdateUrl);
  httpClient.addHeader("Authorization", "Bearer " + apiToken);

  int httpCode = httpClient.GET();
  if (httpCode == 200)
  {
    const size_t capacity = JSON_OBJECT_SIZE(3) + 235;
    DynamicJsonDocument doc(capacity);
    String response = httpClient.getString();

    DeserializationError error = deserializeJson(doc, response);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    // Extract values
    JsonObject obj = doc.as<JsonObject>();

    String newVersion = obj["version"].as<String>();
    Serial.print(F("New Firmware Version: "));
    Serial.println(newVersion.c_str());

    latestFirmwareFileUrl = obj["url"].as<String>();
    Serial.print("Firmware Update URL: ");
    Serial.println(latestFirmwareFileUrl.c_str());

    // Publish new firmware version to Blynk
    Blynk.virtualWrite(BLYNK_VIRTUAL_PIN_OUT_LATEST_FW_VERSION, newVersion.c_str());

    if (enableAutoFirmwareUpdate == 1)
    {
      updateFirmwareFromCloud();
    }
  }
  else if (httpCode == 404)
  {
    Serial.print("No new fimware available...");
    Blynk.virtualWrite(BLYNK_VIRTUAL_PIN_OUT_LATEST_FW_VERSION, CURRENT_FIRMWARE_VERSION);
  }
  else
  {
    Serial.print("Firmware version check failed, got HTTP response code ");
    Serial.println(httpCode);
  }
  httpClient.end();
}

#pragma endregion

#pragma region[Private Methods]

void printSystemInfo()
{
  Serial.print("Device Type: ");
  Serial.println(DEVICE_TYPE);
  Serial.print("Firmware Version: ");
  Serial.println(CURRENT_FIRMWARE_VERSION);
}

#pragma endregion

#pragma region[NeoPixel Patterns]

void setPattern(int index)
{
  setColor(0, 0, 0);
  switch (index)
  {
  case 1: // 20:20:20 R-G-B
    colorDistribute(strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255));
    break;
  case 2: // 20:20:20 B-C(190, 0, 75)-B
    colorDistribute(strip.Color(0, 0, 255), strip.Color(190, 0, 75), strip.Color(0, 0, 255));
    break;
  case 3: //ColorWipe Green
    colorWipe(strip.Color(0, 255, 0), 100);
    break;
  case 4: //ColorWipe Blue
    colorWipe(strip.Color(0, 0, 255), 100);
    break;
  case 5: //TheatreChase Blue
    theaterChase(strip.Color(0, 0, 127), 50);
    break;
  case 6:
    rainbow(20);
    break;
  case 7:
    rainbowCycle(20);
    break;
  case 8:
    theaterChaseRainbow(50);
    break;
  }
}

void colorDistribute(uint32_t c1, uint32_t c2, uint32_t c3)
{
  for (int i = 0; i < 10; i++)
  {
    strip.setPixelColor(i, c1);
  }
  for (int i = 11; i < 20; i++)
  {
    strip.setPixelColor(i, c2);
  }
  for (int i = 21; i < 30; i++)
  {
    strip.setPixelColor(i, c3);
  }
  strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256; j++)
  {
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++)
  { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait)
{
  for (int j = 0; j < 10; j++)
  { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++)
    {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, c); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, 0); //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait)
{
  for (int j = 0; j < 256; j++)
  { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++)
    {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, Wheel((i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, 0); //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

#pragma endregion

#pragma region[Setup &Loop]

void setup()
{
  initializeSerial();
  printSystemInfo();
  initializePins();
  initializeEEPROM();

  strip.begin();

  initiallizeWifiAndBlynk();
  initializeLocalOTA();
  checkForNewFirmware();

  timer.setInterval(FREQUENCY_IN_SECONDS_FIRMWARE_CHECK, checkForNewFirmware);
}

void loop()
{
  // Handle OTA
  ArduinoOTA.handle();

  // Start Bylnk
  Blynk.run();
}

#pragma endregion