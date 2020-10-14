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
    BLEDevice::init("Nesh-O-Matic");
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
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x02,        // Usage (Mouse)
0xA1, 0x01,        // Collection (Application)
//0x85, 0x01,        //   Report ID (1)
0x09, 0x01,        //   Usage (Pointer)
0xA1, 0x00,        //   Collection (Physical)
0x05, 0x09,        //     Usage Page (Button)
0x19, 0x01,        //     Usage Minimum (0x01)
0x29, 0x03,        //     Usage Maximum (0x03)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x95, 0x03,        //     Report Count (3)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x75, 0x05,        //     Report Size (5)
0x95, 0x01,        //     Report Count (1)
0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x09, 0x30,        //     Usage (X)
0x16, 0xC8, 0xFB,  //     Logical Minimum (-1080)
0x36, 0x00, 0x00,  //     Physical Minimum (0)
0x26, 0x38, 0x04,  //     Logical Maximum (1080)
0x46, 0x38, 0x04,  //     Physical Maximum (1080)
0x09, 0x31,        //     Usage (Y)   
0x16, 0x80, 0xF8,  //     Logical Minimum (-1920)
0x36, 0x00, 0x00,  //     Physical Minimum (0)
0x26, 0x80, 0x07,  //     Logical Maximum (1920)
0x46, 0x80, 0x07,  //     Physical Maximum (1920)
0x75, 0x10,        //     Report Size (16)
0x95, 0x02,        //     Report Count (2)
0x81, 0x06,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xC0,              // End Collection
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

void moveTo(uint16_t x, uint16_t y) {
  unsigned char buffer[] = {0, x & 0x00FF, (x & 0xFF00) >> 8, y & 0x00FF, (y & 0xFF00) >> 8};
 
  inputMouse->setValue(buffer,sizeof(buffer));
  inputMouse->notify();
}

void loop() {
  if(connected){
        delay(2000);
        Serial.println("go to origin");
        x = -4000;
        y = -4000;
        moveTo(x, y);
        delay(1000);
        
        Serial.println("go to 5,0");
        x = 5;
        y = 0;
        moveTo(x, y);
        delay(1000);
        
        Serial.println("go to 5,5");
        x = 0;
        y = 5;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 10,5");
        x = 5;
        y = 0;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 10,10");
        x = 0;
        y = 5;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 15,10");
        x = 5;
        y = 0;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 15,15");
        x = 0;
        y = 5;
        moveTo(x, y);
        delay(1000);
        
        Serial.println("go to 25,15");
        x = 10;
        y = 0;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 25,25");
        x = 0;
        y = 10;
        moveTo(x, y);
        delay(1000);
        
        Serial.println("go to 45,25");
        x = 20;
        y = 0;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 45,45");
        x = 0;
        y = 20;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 100,45");
        x = 55;
        y = 0;
        moveTo(x, y);
        delay(1000);

        
        Serial.println("go to 100,100");
        x = 0;
        y = 55;
        moveTo(x, y);
        delay(1000);

       
        Serial.println("go to 150,150");
        x = 50;
        y = 50;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 200,200");
        x = 50;
        y = 50;
        moveTo(x, y);
        delay(1000);
        
        Serial.println("go to 300,300");
        x = 100;
        y = 100;
        moveTo(x, y);
        delay(1000);
    }
  delay(50);
}
