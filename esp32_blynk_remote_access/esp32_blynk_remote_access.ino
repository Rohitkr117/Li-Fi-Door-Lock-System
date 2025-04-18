#define BLYNK_TEMPLATE_ID "TMPL3n8Kx0Yha"
#define BLYNK_TEMPLATE_NAME "LiFi"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Blynk & WiFi credentials
char auth[] = "UPrzQVhKMsQQehJMCmh_YTgg2vQhlSqn";
char ssid[] = "ACS";
char pass[] = "";

// Define the pins for Serial2
#define RXD2 16   // ESP32 RX (connect to Arduino TX through voltage divider)
#define TXD2 17   // ESP32 TX (connect directly to Arduino RX)

void setup() {
  Serial.begin(9600);       // For debugging via USB
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // For communication to Arduino
  Blynk.begin(auth, ssid, pass);
}


// BLYNK HANDLERS


// V1: Update password for User 1
BLYNK_WRITE(V1) {
  String newPass = param.asString();
  if(newPass.length() == 4) {
    String command = "P,1," + newPass + "\n";
    Serial2.print(command);
  }
}

// V2: Update password for User 2
BLYNK_WRITE(V2) {
  String newPass = param.asString();
  if(newPass.length() == 4) {
    String command = "P,2," + newPass + "\n";
    Serial2.print(command);
  }
}

// V3: Update password for User 3
BLYNK_WRITE(V3) {
  String newPass = param.asString();
  if(newPass.length() == 4) {
    String command = "P,3," + newPass + "\n";
    Serial2.print(command);
    Serial.print(command);
  }
}

// V4: Toggle access for User 1 (block/unblock)
BLYNK_WRITE(V4) {
  int state = param.asInt();
  String command = "B,1," + String(state) + "\n";
  Serial2.print(command);
  Serial.print(command);
}

// V5: Toggle access for User 2
BLYNK_WRITE(V5) {
  int state = param.asInt();
  String command = "B,2," + String(state) + "\n";
  Serial2.print(command);
  Serial.print(command);
}

// V6: Toggle access for User 3
BLYNK_WRITE(V6) {
  int state = param.asInt();
  String command = "B,3," + String(state) + "\n";
  Serial2.print(command);
  Serial.print(command);
}

// V7: Emergency button to open the door immediately
BLYNK_WRITE(V7) {
  int state = param.asInt();
  if (state == 1) {
    Serial2.print("E\n");
  }
}

void loop() {
  Blynk.run();
}
