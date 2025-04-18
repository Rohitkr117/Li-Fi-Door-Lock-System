#include <Servo.h>

#define LDR_PIN A0
#define LED_PIN 13           // LED or unlock indicator
#define SAMPLING_TIME 40
#define PREAMBLE '*'         // Preamble character for LiFi communication

#define PASSWORD_LENGTH 4
#define NUM_USERS 3

// Servo and button configuration
#define SERVO_PIN 9          // Servo control pin
#define LOCK_BUTTON_PIN 7    // Button to lock the door

// Variables for LiFi detection
bool previous_state = true;
bool current_state = true;
char receivedPassword[PASSWORD_LENGTH + 1];
int receivedIndex = 0;
bool authenticationComplete = false;
bool preambleDetected = false;

// Stored user credentials and their active state (true = enabled, false = blocked)
char userPasswords[NUM_USERS][PASSWORD_LENGTH + 1] = { "ACSL", "PICO", "1234" };
bool userActive[NUM_USERS] = { true, true, true };

// Instantiate the servo
Servo doorServo;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  // Set up the lock button with internal pullup (button pressed = LOW)
  pinMode(LOCK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(6, OUTPUT);
  
  Serial.begin(9600);
  
  // Attach servo and set door locked initially (90° = locked)
  doorServo.attach(SERVO_PIN);
  doorServo.write(115);
}

void loop() {
  // Process incoming Serial commands from NodeMCU
  digitalWrite(6, HIGH);
  if (Serial.available() > 0) {
    processSerialCommand();
  }
  
  // Check the lock button state (active LOW)
  if (digitalRead(LOCK_BUTTON_PIN) == LOW) {
    // Debounce: small delay and check again
    delay(50);
    if (digitalRead(LOCK_BUTTON_PIN) == LOW) {
      lockDoor();
      // Wait until button is released
      while (digitalRead(LOCK_BUTTON_PIN) == LOW) { delay(10); }
    }
  }
  
  // If an emergency command has granted access, or if authentication succeeded, skip LiFi processing
  if (authenticationComplete) {
    return;
  }
  
  // LiFi detection process (using LDR sensor)
  current_state = get_ldr();
  if (!current_state && previous_state) { // falling edge detected
    char receivedChar = get_byte();
    
    if (receivedChar == PREAMBLE) {
      preambleDetected = true;
      receivedIndex = 0;
    } else if (preambleDetected && receivedIndex < PASSWORD_LENGTH) {
      receivedPassword[receivedIndex++] = receivedChar;
    }
    
    if (preambleDetected && receivedIndex == PASSWORD_LENGTH) {
      receivedPassword[PASSWORD_LENGTH] = '\0';  // Null-terminate string
      checkPassword();
      preambleDetected = false;
      receivedIndex = 0;
    }
  }
  
  // digitalWrite(LED_PIN, current_state);
  previous_state = current_state;
}

// Function to read LDR sensor value (simulate digital state)
bool get_ldr() {
  return analogRead(LDR_PIN) > 900;
}

// Function to receive a byte via LiFi (bit-by-bit using LDR)
char get_byte() {
  char data_byte = 0;
  delay(SAMPLING_TIME * 1.5);
  for (int i = 0; i < 8; i++) {
    data_byte |= (char)get_ldr() << i;
    delay(SAMPLING_TIME);
  }
  return data_byte;
}

// Check the received password against stored credentials
void checkPassword() {
  Serial.print("Received Password: ");
  Serial.println(receivedPassword);
  bool accessGranted = false;
  
  for (int i = 0; i < NUM_USERS; i++) {
    if (userActive[i] && strcmp(receivedPassword, userPasswords[i]) == 0) {
      Serial.print("Access Granted for User ");
      Serial.println(i + 1);
      accessGranted = true;
      authenticationComplete = true;
      unlockDoor();
      break;
    }
  }
  if (!accessGranted) {
    Serial.println("Access Denied");
  }
}

// Process Serial commands sent from NodeMCU
void processSerialCommand() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  if (command.length() == 0) return;
  
  char cmdType = command.charAt(0);
  if (cmdType == 'P') {  // Password update command: "P,<UserID>,<NewPassword>"
    int firstComma = command.indexOf(',');
    int secondComma = command.indexOf(',', firstComma + 1);
    if (firstComma > 0 && secondComma > firstComma) {
      int userId = command.substring(firstComma + 1, secondComma).toInt();
      String newPass = command.substring(secondComma + 1);
      if (userId >= 1 && userId <= NUM_USERS && newPass.length() == PASSWORD_LENGTH) {
        newPass.toCharArray(userPasswords[userId - 1], PASSWORD_LENGTH + 1);
        Serial.print("Updated password for User ");
        Serial.print(userId);
        Serial.print(": ");
        Serial.println(userPasswords[userId - 1]);
      }
    }
  }
  else if (cmdType == 'B') {  // Block/Unblock command: "B,<UserID>,<state>"
    int firstComma = command.indexOf(',');
    int secondComma = command.indexOf(',', firstComma + 1);
    if (firstComma > 0 && secondComma > firstComma) {
      int userId = command.substring(firstComma + 1, secondComma).toInt();
      int state = command.substring(secondComma + 1).toInt();
      if (userId >= 1 && userId <= NUM_USERS) {
        userActive[userId - 1] = (state == 1);
        Serial.print("User ");
        Serial.print(userId);
        Serial.print(" access ");
        Serial.println(userActive[userId - 1] ? "Enabled" : "Blocked");
      }
    }
  }
  else if (cmdType == 'E') {  // Emergency command: "E"
    grantEmergencyAccess();
  }
}

// Immediately grant access when an emergency command is received
void grantEmergencyAccess() {
  Serial.println("Emergency: Access Granted Immediately");
  authenticationComplete = true;
  unlockDoor();
  // Additional hardware logic for unlocking (e.g., driving a relay) can be added here.
}

// Unlock door: set servo to 0° (unlocked)
void unlockDoor() {
  Serial.println("Door Unlocked");
  doorServo.write(0);
}

// Lock door: set servo to 90° (locked) and allow password entry again
void lockDoor() {
  Serial.println("Door Locked");
  doorServo.write(115);
  // Re-enable LiFi processing by resetting authentication flag
  authenticationComplete = false;
}
