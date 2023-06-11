#include "BLEDevice.h"
//#include "BLEScan.h"
#include <Fri3dMatrix.h>
#include <Fri3dButtons.h>

Fri3dMatrix matrix = Fri3dMatrix();
Fri3dButtons buttons = Fri3dButtons();

// UUIDs were generated randomly using https://www.uuidgenerator.net/
static BLEUUID FRI3D_SERVICE_UUID("ea5c0c62-079e-11ee-be56-0242ac120002");
static BLEUUID   FRI3D_COLOR_UUID("086aece6-079f-11ee-be56-0242ac120002");
static BLEUUID    FRI3D_PUSH_UUID("3ed75c2e-079f-11ee-be56-0242ac120002");

class AgentAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE found: ");
    Serial.print(advertisedDevice.toString().c_str());
    Serial.print(" = ");
    if (advertisedDevice.haveName()) Serial.println(advertisedDevice.getName().c_str());
    else Serial.println("<noname>");

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(FRI3D_SERVICE_UUID))
    {
      Serial.println("GOTCHA!");
    }

  }
};

void init_bluetooth() {
  BLEDevice::init("");


}

void setup() {
  Serial.begin(115200);
  Serial.println("Code for Ingress agent on Fri3d Badge 2018 (wolf)");
  
  init_bluetooth();
  for (int i=0; i<20; ++i)
  {
    matrix.setPixel( rand() % 14, rand() % 5, 1 );
  }
}

void loop() {
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AgentAdvertisedDeviceCallbacks());
  pBLEScan->start(5, false);

  String scanDone = "SCAN COMPLETE CHECK SERIAL MONITOR";
  Serial.println(scanDone.c_str());
  int x = -14; //there are 14 columns of LEDs, start at outside
  while(!(buttons.getButton(0) || buttons.getButton(1))) {
    matrix.clear();
    matrix.drawString( -x, scanDone );
    x++;
    if (x>((int)scanDone.length() * 4)) x=-14;
    delay(100);
  }
}
