#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

HardwareSerial fSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fSerial);


void setup() {
  Serial.begin(115200);

  while (!Serial);  
  delay(100);

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    finger.getTemplateCount();
    Serial.println(finger.templateCount);
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
  delay(1000);

  finger.emptyDatabase();
  Serial.println("delete");
  finger.LEDcontrol(FINGERPRINT_LED_FLASHING,10,FINGERPRINT_LED_RED,10);
  delay(2000);
  finger.LEDcontrol(FINGERPRINT_LED_ON,200,FINGERPRINT_LED_BLUE);
  

}

void loop(){
  delay(1000);
}