#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"


BLEHIDDevice* hid;
BLECharacteristic* inputMouse;


bool connected = false;

class MyCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer){
    connected = true;
    BLE2902* desc = (BLE2902*)inputMouse->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(true);
  }

  void onDisconnect(BLEServer* pServer){
    connected = false;
    BLE2902* desc = (BLE2902*)inputMouse->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(false);
  }
};



void taskServer(void*){
    BLEDevice::init("UAT-TEST-8732168");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyCallbacks());

    hid = new BLEHIDDevice(pServer);
    inputMouse = hid->inputReport(4); // <-- input REPORTID from report map

    std::string name = "Neshius Industries Corp.";
    hid->manufacturer()->setValue(name);

    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    hid->hidInfo(0x00,0x07);

  BLESecurity *pSecurity = new BLESecurity();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

// http://www.keil.com/forum/15671/usb-mouse-with-scroll-wheel/
// Wheel Mouse - simplified version - 5 button, vertical and horizontal wheel
//
// Input report - 5 bytes
//
//     Byte | D7      D6      D5      D4      D3      D2      D1      D0
//    ------+---------------------------------------------------------------------
//      0   |  0       0       0    Forward  Back    Middle  Right   Left (Buttons)
//      1   |                             X
//      2   |                             Y
//      3   |                       Vertical Wheel
//      4   |                    Horizontal (Tilt) Wheel
//
// Feature report - 1 byte
//
//     Byte | D7      D6      D5      D4   |  D3      D2  |   D1      D0
//    ------+------------------------------+--------------+----------------
//      0   |  0       0       0       0   |  Horizontal  |    Vertical
//                                             (Resolution multiplier)
//
// Reference
//    Wheel.docx in "Enhanced Wheel Support in Windows Vista" on MS WHDC
//    http://www.microsoft.com/whdc/device/input/wheel.mspx
//


const uint8_t reportMapMouse[] = {
0x05, 0x0D,        // Usage Page (Digitizer)
0x09, 0x04,        // Usage (Touch Screen)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0x09, 0x22,        //   Usage (Finger)
0xA1, 0x02,        //   Collection (Logical)
0x09, 0x42,        //     Usage (Tip Switch)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x95, 0x01,        //     Report Count (1)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x07,        //     Report Count (7)
0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0x09, 0x51,        //     Usage (0x51)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x30,        //     Usage (Tip Pressure)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x75, 0x10,        //     Report Size (16)
0x95, 0x01,        //     Report Count (1)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x55, 0x0F,        //     Unit Exponent (-1)
0x65, 0x11,        //     Unit (System: SI Linear, Length: Centimeter)
0x26, 0x00, 0x04,  //     Logical Maximum (1024)
0x35, 0x00,        //     Physical Minimum (0)
0x46, 0xFF, 0xFF,  //     Physical Maximum (-1)
0x09, 0x30,        //     Usage (X)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x46, 0xFF, 0xFF,  //     Physical Maximum (-1)
0x26, 0x58, 0x02,  //     Logical Maximum (600)
0x09, 0x31,        //     Usage (Y)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0x05, 0x0D,        //   Usage Page (Digitizer)
0x55, 0x0C,        //   Unit Exponent (-4)
0x66, 0x01, 0x10,  //   Unit (System: SI Linear, Time: Seconds)
0x47, 0xFF, 0xFF, 0x00, 0x00,  //   Physical Maximum (65534)
0x27, 0xFF, 0xFF, 0x00, 0x00,  //   Logical Maximum (65534)
0x75, 0x10,        //   Report Size (16)
0x95, 0x01,        //   Report Count (1)
0x09, 0x56,        //   Usage (0x56)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x0D,        //   Usage Page (Digitizer)
0x09, 0x54,        //   Usage (0x54)
0x25, 0x7F,        //   Logical Maximum (127)
0x95, 0x01,        //   Report Count (1)
0x75, 0x08,        //   Report Size (8)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x85, 0x02,        //   Report ID (2)
0x09, 0x55,        //   Usage (0x55)
0x95, 0x01,        //   Report Count (1)
0x25, 0x02,        //   Logical Maximum (2)
0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection

// 72 bytes
};

    hid->reportMap((uint8_t*)reportMapMouse, sizeof(reportMapMouse));
    hid->startServices();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setAppearance(HID_DIGITAL_PEN);
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->start();
    hid->setBatteryLevel(100);

    ESP_LOGD(LOG_TAG, "Advertising started!");
    delay(portMAX_DELAY);
  
};

uint16_t x = 0;
uint16_t y = 0;

void setup() {
  
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);
}

void click() {
  unsigned char buffer[] = {1, 0, 0, 0, 0};
 
  inputMouse->setValue(buffer,sizeof(buffer));
  inputMouse->notify();
  unsigned char buffer2[] = {0, 0, 0, 0, 0};
 
  inputMouse->setValue(buffer2,sizeof(buffer2));
  inputMouse->notify();
}

void moveTo(uint16_t x, uint16_t y) {
  //unsigned char buffer[] = {4, x & 0x00FF, (x & 0xFF00) >> 8, y & 0x00FF, (y & 0xFF00) >> 8};
  unsigned char buffer[] = {4, y & 0x00FF, (y & 0xFF00) >> 8, x & 0x00FF, (x & 0xFF00) >> 8};
 
  inputMouse->setValue(buffer,sizeof(buffer));
  inputMouse->notify();
}

void loop() {
  if(connected){
        Serial.println("go to 0,0");
        x = 0;
        y = 0;
        moveTo(x, y);
        delay(1000);
        
        Serial.println("go to 1,1");
        x = 50000;
        y = 10000;
        moveTo(x, y);
        delay(1000);
        
        Serial.println("go to 2,2");
        x = 5000;
        y = 3000;
        moveTo(x, y);
        click();
        delay(1000);
        /*
        Serial.println("go to 50,50");
        x = 50;
        y = 50;
        moveTo(x, y);
        click();
        delay(1000);

        Serial.println("go to 100,100");
        x = 100;
        y = 100;
        moveTo(x, y);
        click();
        delay(1000);

        Serial.println("go to 100,0");
        x = 100;
        y = 0;
        moveTo(x, y);
        click();
        delay(1000);

        Serial.println("go to 200,200");
        x = 200;
        y = 200;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 200,0");
        x = 200;
        y = 0;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 500,500");
        x = 500;
        y = 500;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 500,0");
        x = 500;
        y = 0;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 1000,1000");
        x = 1000;
        y = 1000;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 1000,0");
        x = 1000;
        y = 0;
        moveTo(x, y);
        delay(1000);

        
        Serial.println("go to 1050,1050");
        x = 1050;
        y = 1050;
        moveTo(x, y);
        click();
        delay(1000);

        Serial.println("go to 1079,0");
        x = 1079;
        y = 0;
        moveTo(x, y);
        click();
        delay(1000);

        Serial.println("go to 0, 1079");
        x = 0;
        y = 1079;
        moveTo(x, y);
        click();
        delay(1000);
        
        Serial.println("go to 1079,1079");
        x = 1079;
        y = 1079;
        moveTo(x, y);
        click();
        delay(1000);*/
    }
  delay(50);
}
