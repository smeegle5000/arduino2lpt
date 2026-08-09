#define READY_BIT 3   // PORTB bit3 = D11
#define ACK_BIT   2   // PORTD bit2 = D2
#define CHUNK 512

void setup() {
  Serial.begin(250000);
  DDRD |= 0xF8;                // D3-D7 output
  DDRB |= 0x0F;                // D8-D11 output
  PORTB &= ~(1 << READY_BIT);  // READY low
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
  static byte buf[CHUNK];
  readBlock(buf, CHUNK);
  for (int i = 0; i < CHUNK; i++) sendByte(buf[i]);
  Serial.write(1);
}
