#include <BLEDevice.h>
//#include <BLEScan.h>
#include <Fri3dMatrix.h>
#include <Fri3dButtons.h>

#include "ingress.h"

Fri3dMatrix matrix = Fri3dMatrix();
Fri3dButtons buttons = Fri3dButtons();

// UUIDs were generated randomly using https://www.uuidgenerator.net/
static BLEUUID FRI3D_SERVICE_UUID("ea5c0c62-079e-11ee-be56-0242ac120002");
static BLEUUID   FRI3D_COLOR_UUID("086aece6-079f-11ee-be56-0242ac120002");
static BLEUUID    FRI3D_PUSH_UUID("3ed75c2e-079f-11ee-be56-0242ac120002");

BLEAdvertisedDevice *closestBeacon;
int closestRSSI = -1000;
uint8_t myColor = CAMP_BLUE;

String scanDone = "NOT FOUND";

class AgentAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  private:
    void FindClosestBeacon(BLEAdvertisedDevice advertisedDevice)
    {
      if (advertisedDevice.getRSSI() > closestRSSI) {
        closestBeacon = new BLEAdvertisedDevice(advertisedDevice);
        closestRSSI = advertisedDevice.getRSSI();
      }
      else {
        Serial.print("discarded Beacon RSSI ");
        Serial.print(advertisedDevice.getRSSI());
        Serial.print(" < ");
        Serial.print(closestRSSI);
      }
    }
  public:
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE found: ");
    Serial.print(advertisedDevice.toString().c_str());
    Serial.print(" = ");
    if (advertisedDevice.haveName()) 
    {
      Serial.println(advertisedDevice.getName().c_str());
      if (advertisedDevice.getName().find("Fri3dBaconAdvertised") != std::string::npos) {
        scanDone = "NAME";
        FindClosestBeacon(advertisedDevice);
      }
    }
    else Serial.println("<noname>");

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(FRI3D_SERVICE_UUID))
    {
      scanDone = "SERVICE UUID";
      FindClosestBeacon(advertisedDevice);
    }

  }
};

void init_bluetooth() {
  BLEDevice::init("WOLFri3d");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Code for Ingress agent on Fri3d Badge 2018 (wolf)");
  
  init_bluetooth();
}

bool ConnectToBeacon()
{
  Serial.println("attempting to connect...");
  BLEClient*  pClient  = BLEDevice::createClient();
  pClient->connect(closestBeacon);
  BLERemoteService* pRemoteService = pClient->getService(FRI3D_SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service FRI3D_SERVICE_UUID");
    pClient->disconnect();
    return false;
  }
  else {
    Serial.print("service found: ");
    Serial.println(pRemoteService->toString().c_str());
  }

  BLERemoteCharacteristic *colorChar = pRemoteService->getCharacteristic(FRI3D_COLOR_UUID);
  if (colorChar == nullptr) {
    Serial.println("Failed to find FRI3D_COLOR_UUID");
  }
  else {
    Serial.print("color value = ");
    Serial.println(colorChar->readValue().c_str());
  }

  BLERemoteCharacteristic *pushChar = pRemoteService->getCharacteristic(FRI3D_PUSH_UUID);
  if (pushChar == nullptr) {
    Serial.println("Failed to find FRI3D_PUSH_UUID");
  }
  else {
    Serial.print("can read? "); Serial.println(pushChar->canRead());
    Serial.print("can write? "); Serial.println(pushChar->canWrite());
    Serial.print("can write no response? "); Serial.println(pushChar->canWriteNoResponse());
    uint8_t mijnkamp = CAMP_BLUE;
    pushChar->writeValue(&mijnkamp, 1);
  }

  pClient->disconnect();
  return false;
}

void scanAndIntrude() {
  //first, reset distance so we can try and find closest beacon:
  closestRSSI = -1000;

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AgentAdvertisedDeviceCallbacks());
  pBLEScan->start(5, false);

  if (closestRSSI > -1000) ConnectToBeacon();
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

void switchColor() {

}

void loop() {
  scanAndIntrude();
}
