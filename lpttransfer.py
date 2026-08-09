import serial, sys, os, time, struct, zlib

PORT = 'COM13'
BAUD = 250000
CHUNK = 512
BINARY_UNITS = False   # False = 1000-based (KB/MB), True = 1024-based (KiB/MiB)

SYNC_WORD = bytes([0xAB,0xCD,0xEF,0x12,0x34,0x56,0x78,0x90])
HEADER_SIZE = 64

def build_header(filepath, filesize, crc):
    base = os.path.basename(filepath)
    name, ext = os.path.splitext(base)
    ext = ext.lstrip('.')

    name_b = name.upper().encode('ascii', 'ignore')[:8].ljust(8, b'\x00')
    ext_b  = ext.upper().encode('ascii', 'ignore')[:3].ljust(3, b'\x00')

    hdr  = struct.pack('<I', filesize)      # 0-3   size
    hdr += b'\x00' * 4                      # 4-7   reserved
    hdr += name_b                           # 8-15  filename
    hdr += b'\x00' * 32                     # 16-47 reserved
    hdr += ext_b                            # 48-50 ext
    hdr += b'\x00' * 5                      # 51-55 reserved
    hdr += struct.pack('<I', crc)           # 56-59 crc
    hdr += struct.pack('<I', crc)           # 60-63 crc dup
    assert len(hdr) == HEADER_SIZE
    return hdr

def fmt_size(n):
    unit = 1024 if BINARY_UNITS else 1000
    label = "iB" if BINARY_UNITS else "B"
    if n < 10000:
        return f"{n} B"
    elif n < 10000 * unit:
        return f"{n/unit:.2f} K{label}"
    else:
        return f"{n/(unit*unit):.2f} M{label}"

def fmt_speed(bps):
    unit = 1024 if BINARY_UNITS else 1000
    label = "iB" if BINARY_UNITS else "B"
    if bps < 1000:
        return f"{bps:.0f} {label}/s"
    else:
        return f"{bps/unit:.2f} K{label}/s"

def draw_progress(sent, total, start_time):
    width = 30
    frac = sent / total if total else 1
    filled = int(width * frac)
    bar = "#" * filled + "-" * (width - filled)
    elapsed = time.time() - start_time
    speed = sent / elapsed if elapsed > 0 else 0
    line = f"\r[{bar}] {frac*100:5.1f}%  ({fmt_size(sent)}/{fmt_size(total)})  [{fmt_speed(speed)}]"
    sys.stdout.write(line.ljust(90))
    sys.stdout.flush()

def main():
    if len(sys.argv) < 2:
        print("Drag a file onto this script to send it.")
        input("Press Enter to exit...")
        return

    filepath = sys.argv[1]
    with open(filepath, 'rb') as f:
        data = f.read()

    filesize = len(data)
    crc = zlib.crc32(data) & 0xFFFFFFFF
    header = build_header(filepath, filesize, crc)

    stream = SYNC_WORD + header + data
    pad_len = (-len(stream)) % CHUNK
    stream += b'\x00' * pad_len

    print(f"File: {os.path.basename(filepath)}  {fmt_size(filesize)}  CRC32: {crc:08X}")
    print(f"Sync+header: {len(SYNC_WORD)+len(header)} bytes, pad: {pad_len} bytes, "
          f"total stream: {len(stream)} bytes ({len(stream)//CHUNK} chunks)")

    ser = serial.Serial()
    ser.port = PORT
    ser.baudrate = BAUD
    ser.timeout = 10
    ser.dtr = False
    ser.open()
    time.sleep(0.1)
    ser.reset_input_buffer()

    header_len = len(SYNC_WORD) + len(header)
    sent = 0
    start_time = time.time()

    for i in range(0, len(stream), CHUNK):
        chunk = stream[i:i+CHUNK]
        ser.write(chunk)
        ack = ser.read(1)
        if not ack:
            print(f"\nTimeout at chunk {i // CHUNK}")
            ser.close()
            return
        sent = min(i + CHUNK, len(stream))
        file_progress = max(0, min(filesize, sent - header_len))
        draw_progress(file_progress, filesize, start_time)

    ser.close()
    print("\nDone.")

if __name__ == "__main__":
    main()
