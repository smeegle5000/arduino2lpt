import serial, time

CHUNK = 32
ser = serial.Serial('COM13', 250000, timeout=5)
time.sleep(2)

with open('FILE3.EXE', 'rb') as f:
    data = f.read()

size = len(data)
pad = (-size) % CHUNK
data += b'\x00' * pad

ser.write(size.to_bytes(4, 'little'))
ack = ser.read(1)
if not ack:
    print("Timeout on header")
    exit()

for i in range(0, len(data), CHUNK):
    ser.write(data[i:i+CHUNK])
    ack = ser.read(1)
    if not ack:
        print(f"Timeout at chunk {i // CHUNK}")
        break

ser.close()
print("Done")
