#include <BLEDevice.h>
//#include <BLEScan.h>
#include <Fri3dMatrix.h>
#include <Fri3dButtons.h>
#include <Fri3dBuzzer.h>

#include "ingress.h"

Fri3dMatrix matrix = Fri3dMatrix();
Fri3dButtons buttons = Fri3dButtons();
Fri3dBuzzer buzzer = Fri3dBuzzer();

// UUIDs were generated randomly using https://www.uuidgenerator.net/
static BLEUUID FRI3D_SERVICE_UUID("ea5c0c62-079e-11ee-be56-0242ac120002");
static BLEUUID   FRI3D_COLOR_UUID("086aece6-079f-11ee-be56-0242ac120002");
static BLEUUID    FRI3D_PUSH_UUID("3ed75c2e-079f-11ee-be56-0242ac120002");

BLEAdvertisedDevice *closestBeacon;
BLERemoteService *closestService;
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
  showColor();
}

bool ConnectToBeacon()
{
  Serial.println("attempting to connect...");
  BLEClient*  pClient  = BLEDevice::createClient();
  pClient->connect(closestBeacon);
  closestService = pClient->getService(FRI3D_SERVICE_UUID);
  if (closestService == nullptr) {
    Serial.print("Failed to find our service FRI3D_SERVICE_UUID");
    pClient->disconnect();
    closestService = nullptr;
    return false;
  }
  else {
    Serial.print("service found: ");
    Serial.println(closestService->toString().c_str());
  }

  BLERemoteCharacteristic *colorChar = closestService->getCharacteristic(FRI3D_COLOR_UUID);
  if (colorChar == nullptr) {
    Serial.println("Failed to find FRI3D_COLOR_UUID");
  }
  else {
    Serial.print("color value = ");
    Serial.println(colorChar->readValue().c_str());
  }


  return true;
}

bool findClosestBeacon()
{
  //first, reset distance so we can try and find closest beacon:
  closestRSSI = -1000;

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AgentAdvertisedDeviceCallbacks());
  pBLEScan->start(5, false);

  if (closestRSSI > -1000) return true;
  return false;
}

void showColor()
{
  matrix.clear();
  matrix.drawString(0, "1=");
  if (myColor == CAMP_RED) matrix.drawCharacter(7, 'R');
  else if (myColor == CAMP_BLUE) matrix.drawCharacter(7, 'B');
  else matrix.drawCharacter(7, 'w');
}

void switchColor()
{
  if (myColor == CAMP_RED) myColor = CAMP_BLUE;
  else if (myColor == CAMP_BLUE) myColor = CAMP_RED;
  else myColor = CAMP_BLUE;
}

bool Invade()
{
  if (closestBeacon == nullptr) {
    Serial.println("Invasion impossible; no beacon available");
    return false;
  }
  if (closestService == nullptr) {
    Serial.println("Invasion impossible; no service for beacon");
    return false;
  }

  BLERemoteCharacteristic *pushChar = closestService->getCharacteristic(FRI3D_PUSH_UUID);
  if (pushChar == nullptr) {
    Serial.println("Failed to find FRI3D_PUSH_UUID");
    return false;
  }
  else {
    Serial.print("can read? "); Serial.println(pushChar->canRead());
    Serial.print("can write? "); Serial.println(pushChar->canWrite());
    Serial.print("can write no response? "); Serial.println(pushChar->canWriteNoResponse());
    pushChar->writeValue(&myColor, 1);
  }
  return true;
}

void loop() {
  if (buttons.getButton(0)) {
    switchColor();
    matrix.clear();
    delay(200);
    showColor();
  }

  if (buttons.getButton(1)) {
    matrix.clear();
    if (
         findClosestBeacon()
      && ConnectToBeacon()
      && Invade()
     )
    {
      Serial.println("Successfully invaded beacon");
      buzzer.setFrequency(1000);
      buzzer.setVolume(200);
      delay(50);
      buzzer.setFrequency(10000);
      buzzer.setVolume(200);
      delay(50);
      buzzer.setVolume(0);
      showColor();
      delay(200);
    }
    else {
      buzzer.setFrequency(10000);
      buzzer.setVolume(200);
      delay(50);
      buzzer.setFrequency(100);
      buzzer.setVolume(200);
      delay(50);
      buzzer.setVolume(0);
      showColor();
      delay(200);
    }
  }
}
