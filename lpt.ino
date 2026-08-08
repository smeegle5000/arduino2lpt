#define READY_BIT 3
#define ACK_BIT   2
#define CHUNK 32

uint32_t totalSize = 0;
uint32_t sentCount = 0;

void setup() {
  delay(5000);
  Serial.begin(250000);
  DDRD |= 0xF8;
  DDRB |= 0x0F;
  PORTB &= ~(1 << READY_BIT);
}

void sendByte(byte value) {
  PORTD = (PORTD & 0x07) | ((value & 0x1F) << 3);
  PORTB = (PORTB & 0xF8) | ((value >> 5) & 0x07);

  PORTB |= (1 << READY_BIT);
  while (PIND & (1 << ACK_BIT));
  PORTB &= ~(1 << READY_BIT);
  while (!(PIND & (1 << ACK_BIT)));
}

void readBlock(byte *buf, int n) {
  int c = 0;
  while (c < n) {
    while (!Serial.available());
    buf[c++] = Serial.read();
  }
}

void loop() {
  static byte hdr[4];
  static byte buf[CHUNK];
  static bool gotHeader = false;

  if (!gotHeader) {
    readBlock(hdr, 4);
    totalSize = (uint32_t)hdr[0] | ((uint32_t)hdr[1] << 8) |
                ((uint32_t)hdr[2] << 16) | ((uint32_t)hdr[3] << 24);
    for (int i = 0; i < 4; i++) sendByte(hdr[i]);
    Serial.write(1);
    gotHeader = true;
    return;
  }

  readBlock(buf, CHUNK);
  for (int i = 0; i < CHUNK; i++) {
    if (sentCount < totalSize) {
      sendByte(buf[i]);
      sentCount++;
    }
    // else: padding byte, drained from serial, never sent over parallel
  }
  Serial.write(1);
}