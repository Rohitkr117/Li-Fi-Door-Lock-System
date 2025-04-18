#define TRANSMIT_LED 12
#define SAMPLING_TIME 40

char* text = "ACSL";
const char preamble = 0x2A; // 10101010 in binary (Sync Pattern)

// Declaration
bool transmit_data = true;
int bytes_counter;
int total_bytes;

void setup() {
  pinMode(TRANSMIT_LED, OUTPUT);
  total_bytes = strlen(text);
}

void loop() {
  while (transmit_data) {
    transmit_byte(preamble); // Send synchronization preamble

    for (int i = 0; i < total_bytes; i++) {
      transmit_byte(text[i]); // Send actual data
    }

    transmit_data = false;
  }

  transmit_data = true;
  delay(1000);
}

void transmit_byte(char data_byte) {
  digitalWrite(TRANSMIT_LED, LOW); 
  delay(SAMPLING_TIME);

  for (int i = 0; i < 8; i++) {
    digitalWrite(TRANSMIT_LED, (data_byte >> i) & 0x01);
    delay(SAMPLING_TIME);
  }

  digitalWrite(TRANSMIT_LED, HIGH); // Return to IDLE state
  delay(SAMPLING_TIME);
}
