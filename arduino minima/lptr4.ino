#define READY_PIN 11
#define ACK_PIN   2
#define CHUNK 512

void setup() {
  Serial.begin(500000);

  for (int pin = 3; pin <= 10; pin++)
    pinMode(pin, OUTPUT);

  pinMode(READY_PIN, OUTPUT);
  pinMode(ACK_PIN, INPUT);

  digitalWrite(READY_PIN, LOW);

  while (!Serial)
    ;
}

void sendByte(byte value) {
  for (int pin = 3; pin <= 10; pin++) {
    digitalWrite(pin, value & 1);
    value >>= 1;
  }

  digitalWrite(READY_PIN, HIGH);

  while (digitalRead(ACK_PIN))
    ;

  digitalWrite(READY_PIN, LOW);

  while (!digitalRead(ACK_PIN))
    ;
}

void readBlock(byte *buf, int n) {
  int c = 0;

  while (c < n) {
    while (!Serial.available())
      ;

    buf[c++] = Serial.read();
  }
}

void loop() {
  static byte buf[CHUNK];

  readBlock(buf, CHUNK);

  for (int i = 0; i < CHUNK; i++)
    sendByte(buf[i]);

  Serial.write(1);
}