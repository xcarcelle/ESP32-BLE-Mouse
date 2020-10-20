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
    inputMouse = hid->inputReport(5); // <-- input REPORTID from report map

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
0x09, 0x02,        // Usage (Pen)
0xA1, 0x01,        // Collection (Application)
0x85, 0x05,        //   Report ID (4)
0x05, 0x0D,        //   Usage Page (Digitizer)
0x09, 0x01,        //   Usage (Digitizer)
0x09, 0x3f,        // USAGE (Azimuth[Orientation])
0x09, 0x48,        // USAGE (Width)
0x09, 0x49,        // USAGE (Height)  
0xA1, 0x00,        //   Collection (Physical)
0x09, 0x42,        //     Usage (Tip Switch)
0x09, 0x32,        //     Usage (In Range)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x95, 0x03,        //     Report Count (3)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x75, 0x05,        //     Report Size (5)
0x95, 0x01,        //     Report Count (1)
0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
//0x16, 0x00, 0x00,  //     Logical Minimum (0)
0x36, 0x00, 0x00,  //     Physical Minimum (0)
// 0x26, 0x80, 0x07,  //     Logical Maximum (1920)
// 0x46, 0x80, 0x07,  //     Physical Maximum (1920)
0x26, 0x38, 0x04,  //     Logical Maximum (1080)
0x46, 0x38, 0x04,  //     Physical Maximum (-1)
0x09, 0x30,        //     Usage (X)
0x16, 0x00, 0x00,  //     Logical Minimum (0)
0x36, 0x00, 0x00,  //     Physical Minimum (0)
// 0x26, 0x38, 0x04,  //     Logical Maximum (1080)
// 0x46, 0x38, 0x04,  //     Physical Maximum (1080)
0x26, 0x80, 0x07,  //     Logical Maximum (1920)
0x46, 0x80, 0x07,  //     Physical Maximum (-1)
0x09, 0x31,        //     Usage (Y)
0x75, 0x10,        //     Report Size (16)
0x95, 0x02,        //     Report Count (2)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
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

void moveTo(uint16_t x, uint16_t y) {
  //unsigned char buffer[] = {5, 1, x & 0x00FF, (x & 0xFF00) >> 8, y & 0x00FF, (y & 0xFF00) >> 8};
  unsigned char buffer[] = {5, 1, 5, 80, 0, 10};
  Serial.print("x : ");Serial.print(buffer[2]);Serial.print(" ");Serial.println(buffer[3]);
  Serial.print("y : ");Serial.print(buffer[4]);Serial.print(" ");Serial.println(buffer[5]);
  //Serial.print("values : ");Serial.print(y & 0x00FF);Serial.println((y & 0xFF00) >> 8);


 
  inputMouse->setValue(buffer,sizeof(buffer));
  inputMouse->notify();
}

void reset() {
  unsigned char buffer[] = {5, 1, 0, 0, 0, 0};
 
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
        x = 1;
        y = 1;
        reset();
        delay(1000);
        
        Serial.println("go to 2,2");
        x = 4;
        y = 4;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 50,50");
        x = 50;
        y = 50;
        reset();
        delay(1000);

        /*
        Serial.println("go to 100,100");
        x = 100;
        y = 100;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 100,0");
        x = 100;
        y = 0;
        moveTo(x, y);
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
        x = 1900;
        y = 1000;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 1079,0");
        x = 1079;
        y = 0;
        moveTo(x, y);
        delay(1000);

        Serial.println("go to 0, 1079");
        x = 0;
        y = 1079;
        moveTo(x, y);
        delay(1000);
        
        Serial.println("go to 1079,1079");
        x = 1079;
        y = 1079;
        moveTo(x, y);
        delay(1000);*/
    }
  delay(50);
}
