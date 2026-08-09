#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <time.h>
#include <string.h>

#define DATA 0x378
#define STAT 0x379
#define CTRL 0x37A
#define HEADER_SIZE 64
#define CHUNK_SIZE 512
#define TIMEOUT_SECS 30
#define SYNC_LEN 8
const unsigned char SYNC_WORD[SYNC_LEN] = {0xAB,0xCD,0xEF,0x12,0x34,0x56,0x78,0x90};

unsigned long crc_table[256];

void crc32_init(void) {
    unsigned long c;
    int n, k;
    for (n = 0; n < 256; n++) {
        c = (unsigned long)n;
        for (k = 0; k < 8; k++)
            c = (c & 1) ? (0xEDB88320UL ^ (c >> 1)) : (c >> 1);
        crc_table[n] = c;
    }
}

unsigned long crc32_update(unsigned long crc, unsigned char b) {
    return crc_table[(crc ^ b) & 0xFF] ^ (crc >> 8);
}

/* HEADER wait only: timeout + ESC. returns 0-255, -1 ESC, -2 timeout */
int recvByteHeader(void) {
    static unsigned int spin = 0;
    time_t start = time(NULL);

    while (!(inp(STAT) & 0x40)) {
        if ((spin++ & 0xFF) == 0) {
            if (kbhit() && getch() == 0x1B) return -1;
            if (time(NULL) - start >= TIMEOUT_SECS) return -2;
        }
    }
    int b = inp(DATA);
    outp(CTRL, 0x81);
    while (inp(STAT) & 0x40);
    outp(CTRL, 0x80);
    return b;
}

int waitForSync(void) {
    unsigned char window[SYNC_LEN];
    int filled = 0;
    int b;

    memset(window, 0, SYNC_LEN);

    while (1) {
        b = recvByteHeader();
        if (b == -1) return -1;
        if (b == -2) return -2;

        /* shift window left, append new byte */
        memmove(window, window + 1, SYNC_LEN - 1);
        window[SYNC_LEN - 1] = (unsigned char)b;
        if (filled < SYNC_LEN) filled++;

        if (filled == SYNC_LEN && memcmp(window, SYNC_WORD, SYNC_LEN) == 0)
            return 0;
    }
}

int main() {
    unsigned char hdr[HEADER_SIZE];
    unsigned long size, i, crc_a, crc_b, computed_crc;
    char fname[13];
    FILE *f;
    int b;
    time_t xfer_start, xfer_end;
    double elapsed;

    crc32_init();
    outp(CTRL, 0x80);

    {
        int s = waitForSync();
        if (s == -1) { printf("Cancelled (ESC) waiting for sync.\n"); return 1; }
        if (s == -2) { printf("Timed out waiting for sync.\n"); return 1; }
    }

    for (i = 0; i < HEADER_SIZE; i++) {
        b = recvByteHeader();
        if (b == -1) { printf("Cancelled (ESC) waiting for header.\n"); return 1; }
        if (b == -2) { printf("Timed out waiting for header.\n"); return 1; }
        hdr[i] = (unsigned char)b;
    }

    size = (unsigned long)hdr[0] | ((unsigned long)hdr[1]<<8) |
           ((unsigned long)hdr[2]<<16) | ((unsigned long)hdr[3]<<24);
    crc_a = (unsigned long)hdr[56] | ((unsigned long)hdr[57]<<8) |
            ((unsigned long)hdr[58]<<16) | ((unsigned long)hdr[59]<<24);
    crc_b = (unsigned long)hdr[60] | ((unsigned long)hdr[61]<<8) |
            ((unsigned long)hdr[62]<<16) | ((unsigned long)hdr[63]<<24);

    if (crc_a != crc_b) {
        printf("Header corrupt: CRC copies mismatch (%08lX vs %08lX)\n", crc_a, crc_b);
        printf("Refusing transfer.\n");
        return 1;
    }

    sprintf(fname, "%.8s.%.3s", &hdr[8], &hdr[48]);
    printf("Receiving: %s (%lu bytes) (%08lX)\n", fname, size, crc_a);

    f = fopen(fname, "wb");
    setvbuf(f, NULL, _IOFBF, 8192);

    xfer_start = time(NULL);

    for (i = 0; i < size; i++) {
        while (!(inp(STAT) & 0x40));
        b = inp(DATA);
        fputc(b, f);
        outp(CTRL, 0x81);
        while (inp(STAT) & 0x40);
        outp(CTRL, 0x80);
    }

    xfer_end = time(NULL);
    elapsed = (double)(xfer_end - xfer_start);

    {
        unsigned long total_len = 8UL + HEADER_SIZE + size;   /* sync + header + payload */
        unsigned long pad_len = (CHUNK_SIZE - (total_len % CHUNK_SIZE)) % CHUNK_SIZE;
        unsigned long p;
        for (p = 0; p < pad_len; p++) {
            while (!(inp(STAT) & 0x40));
            inp(DATA);                 /* discard */
            outp(CTRL, 0x81);
            while (inp(STAT) & 0x40);
            outp(CTRL, 0x80);
        }
    }

    fclose(f);

    if (elapsed > 0) {
        printf("%lu bytes in %.0f sec = %.1f bytes/sec\n",
               size, elapsed, size / elapsed);
    } else {
        printf("Complete!\n");
    }

    /* second pass: compute CRC from disk */
    computed_crc = 0xFFFFFFFFUL;
    f = fopen(fname, "rb");
    setvbuf(f, NULL, _IOFBF, 8192);
    {
        int c;
        while ((c = fgetc(f)) != EOF) {
            computed_crc = crc32_update(computed_crc, (unsigned char)c);
        }
    }
    fclose(f);
    computed_crc ^= 0xFFFFFFFFUL;

    if (computed_crc == crc_a) {
        printf("Complete: %s, CRC OK (%08lX)\n", fname, computed_crc);
    } else {
        printf("CRC MISMATCH: expected %08lX, got %08lX\n", crc_a, computed_crc);
    }
    return 0;
}