#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#define relay LED_BUILTIN
#define enrollHoldPeriod 3500
const bool relayTrig = HIGH;

HardwareSerial fSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fSerial);



void setup() {
  Serial.begin(115200);

  pinMode(relay,OUTPUT);
  digitalWrite(relay, !relayTrig);

  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1000); }
  }
  
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  finger.getTemplateCount();
  
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    enrollFinger(0);
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
  delay(1000);
  Serial.println("Flash");
  finger.LEDcontrol(FINGERPRINT_LED_FLASHING,10,FINGERPRINT_LED_RED,10);
  delay(3000);

  

}

void loop()                     // run over and over again
{
  
  finger.LEDcontrol(FINGERPRINT_LED_ON,200,FINGERPRINT_LED_PURPLE);
  
  int id = getFingerprintID();
  if (id == -2){
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING,10,FINGERPRINT_LED_RED);
    delay(600);
    while (!(finger.getImage() == FINGERPRINT_NOFINGER)){
      delay(1);
    }
  } else if(id >= 0){
    finger.LEDcontrol(FINGERPRINT_LED_BREATHING,40,FINGERPRINT_LED_BLUE);
    digitalWrite(relay, relayTrig);
    delay(500);
    digitalWrite(relay, !relayTrig);
    delay(150);
    unsigned long time = millis();

    while (finger.getImage() != FINGERPRINT_NOFINGER ) {
      if (getFingerprintID() == id){
        if(millis() - time > enrollHoldPeriod){
          finger.LEDcontrol(FINGERPRINT_LED_BREATHING,50,FINGERPRINT_LED_RED);
          Serial.println("Remove Finger");
          while (finger.getImage() != FINGERPRINT_NOFINGER ){
            delay(1);
          }
          finger.getParameters();
          enrollFinger(finger.templateCount);
          return;
        }
        delay(50);
 

      }

    }
  } 


  delay(50);
         //don't ned to run this at full speed.
  
}

int getFingerprintID() {
  int p = finger.getImage();
  verifyImageRead(p);
  if (p != FINGERPRINT_OK){
    return -1;
  }

  p = finger.image2Tz();
  verifyImageConverted(p);
  if (p != FINGERPRINT_OK){
    return -2;
  }
  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return -2;
  } else {
    Serial.println("Unknown error");
    return -1;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}


uint8_t enrollFinger(int id){
  int p = -1;
  for (int i = 1; i < 6; i++){
    finger.LEDcontrol(FINGERPRINT_LED_BREATHING,50,FINGERPRINT_LED_PURPLE);
    while (p != FINGERPRINT_OK) {
      p = finger.getImage();
      verifyImageRead(p);
    }
    p = finger.image2Tz(i);
    verifyImageConverted(p);
    if (p != FINGERPRINT_OK){
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING,20,FINGERPRINT_LED_RED,10);
      delay(300);
      return p;
    }
    Serial.println("Remove finger");
    finger.LEDcontrol(FINGERPRINT_LED_ON,200,FINGERPRINT_LED_BLUE);
    delay(1000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
      p = finger.getImage();
    }
  }
  p = finger.createModel();
  verifyModelCreated(p);
  if (p != FINGERPRINT_OK){
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING,20,FINGERPRINT_LED_RED,10);
    delay(300);
    return p;
  }
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);

  if (p != FINGERPRINT_OK) {
    finger.LEDcontrol(FINGERPRINT_LED_FLASHING,20,FINGERPRINT_LED_RED,10);
    delay(300);
    return p;
  }
  return true;
}
void verifyModelStored(int p){
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.println("Unknown error");
  }
}
void verifyModelCreated(int p){
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
  } else {
    Serial.println("Unknown error");
  }
}
void verifyImageConverted(int p){
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      break;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      break;
    default:
      Serial.println("Unknown error");
      break;
  }
}
void verifyImageRead(int p){
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
  }
}


