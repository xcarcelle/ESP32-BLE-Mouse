#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"

#include "BleConnectionStatus.h"
#include "BleMouse.h"

#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG ""
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "BLEDevice";
#endif

static const uint8_t _hidReportDescriptor[] = {
    USAGE_PAGE(1),      0x01,         // Generic Desktop
    USAGE(1),           0x02,         // Mouse
    COLLECTION(1),      0x01,         // Application
    USAGE(1),           0x01,         //  Pointer
    COLLECTION(1),      0x00,         //  Physical
    USAGE_PAGE(1),      0x09,         //   Buttons
    USAGE_MINIMUM(1),   0x01,
    USAGE_MAXIMUM(1),   0x03,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_COUNT(1),    0x03,         //   3 bits (Buttons)
    REPORT_SIZE(1),     0x01,
    HIDINPUT(1),           0x02,         //   Data, Variable, Absolute
    REPORT_COUNT(1),    0x01,         //   5 bits (Padding)
    REPORT_SIZE(1),     0x05,
    HIDINPUT(1),           0x01,         //   Constant
    USAGE_PAGE(1),      0x01,         //   Generic Desktop
    USAGE(1),           0x30,         //   X
    //PHYSICAL_MINIMUM(2), 0x00, 0x00,         //  0
    //PHYSICAL_MAXIMUM(2), 0x80, 0x07,   //  1920
    //LOGICAL_MINIMUM(2), 0x81, 0x08,    //  -1920
    //LOGICAL_MAXIMUM(2), 0x80, 0x07,    //  1920
    USAGE(1),           0x31,         //   Y
    //PHYSICAL_MINIMUM(2), 0x00, 0x00,         //  0
    //PHYSICAL_MAXIMUM(2), 0x80, 0x07,   //  1920
    //LOGICAL_MINIMUM(2), 0x81, 0x08,    //  -1920
    //LOGICAL_MAXIMUM(2), 0x80, 0x07,    //  1920
    // USAGE(1),           0x38,         //   Wheel
    PHYSICAL_MINIMUM(2), 0x00, 0x00,         //  0
    PHYSICAL_MAXIMUM(2), 0xff, 0x7f,   //  32767
    LOGICAL_MINIMUM(2), 0x01, 0x80,    //  -32767
    LOGICAL_MAXIMUM(2), 0xff, 0x7f,    //  32767
    UNIT(2), 0x00, 0x00,         //  No unit
    REPORT_SIZE(1),     0x10,         //   Three bytes
    REPORT_COUNT(1),    0x02,
    HIDINPUT(1),           0x06,         //   Data, Variable, Relative
    END_COLLECTION(0),
    END_COLLECTION(0),
};

BleMouse::BleMouse(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel) :
    _buttons(0),
    hid(0)
{
  this->deviceName = deviceName;
  this->deviceManufacturer = deviceManufacturer;
  this->batteryLevel = batteryLevel;
  this->connectionStatus = new BleConnectionStatus();
}

void BleMouse::begin(void)
{
  xTaskCreate(this->taskServer, "server", 20000, (void *)this, 5, NULL);
}

void BleMouse::end(void)
{
}

void BleMouse::click(uint8_t b)
{
  _buttons = b;
  move(0,0,0,0);
  _buttons = 0;
  move(0,0,0,0);
}

//void BleMouse::move(signed char x, signed char y, signed char wheel, signed char hWheel)
//160=(1080/2)=540, 120=(810/2)=405
void BleMouse::move(int x, int y, signed char wheel, signed char hWheel)
{
  if (this->isConnected())
  {
    //uint8_t m[5];
    unsigned char m[5];
    int _x = (int) ((1024l * ((long) x)) / 1080); // if 10000l limit = 5000
    int _y = (int) ((1024l * ((long) y)) / 1080);
    //int _x = x;
    //int _y = y;
    ESP_LOGI("----------------------------------");
    ESP_LOGI("POSITION", "x : %d , y : %d", x, y);
    ESP_LOGI("POSITION", "_x : %d , _y : %d", _x, _y);
    ESP_LOGI("----------------------------------");
    m[0] = _buttons;
    m[1] = _x & 0xFF;
    m[2] = (_x >> 8) & 0xFF;
    m[3] = _y & 0xFF;
    m[4] = (_y >> 8) & 0xFF;
    //m[1] = x;
    //m[2] = y;
    //m[3] = wheel;
    //m[4] = hWheel;
    this->inputMouse->setValue(m, 5);
    this->inputMouse->notify();
  }
}

void BleMouse::buttons(uint8_t b)
{
  if (b != _buttons)
  {
    _buttons = b;
    move(0,0,0,0);
  }
}

void BleMouse::press(uint8_t b)
{
  buttons(_buttons | b);
}

void BleMouse::release(uint8_t b)
{
  buttons(_buttons & ~b);
}

bool BleMouse::isPressed(uint8_t b)
{
  if ((b & _buttons) > 0)
    return true;
  return false;
}

bool BleMouse::isConnected(void) {
  return this->connectionStatus->connected;
}

void BleMouse::setBatteryLevel(uint8_t level) {
  this->batteryLevel = level;
  if (hid != 0)
      this->hid->setBatteryLevel(this->batteryLevel);
}

void BleMouse::taskServer(void* pvParameter) {
  BleMouse* bleMouseInstance = (BleMouse *) pvParameter; //static_cast<BleMouse *>(pvParameter);
  BLEDevice::init(bleMouseInstance->deviceName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(bleMouseInstance->connectionStatus);

  bleMouseInstance->hid = new BLEHIDDevice(pServer);
  bleMouseInstance->inputMouse = bleMouseInstance->hid->inputReport(1); // <-- input REPORTID from report map
  bleMouseInstance->connectionStatus->inputMouse = bleMouseInstance->inputMouse;

  bleMouseInstance->hid->manufacturer()->setValue(bleMouseInstance->deviceManufacturer);

  bleMouseInstance->hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  bleMouseInstance->hid->hidInfo(0x00,0x02);

  BLESecurity *pSecurity = new BLESecurity();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  bleMouseInstance->hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  bleMouseInstance->hid->startServices();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_MOUSE);
  pAdvertising->addServiceUUID(bleMouseInstance->hid->hidService()->getUUID());
  pAdvertising->start();
  bleMouseInstance->hid->setBatteryLevel(bleMouseInstance->batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY); //delay(portMAX_DELAY);
}
