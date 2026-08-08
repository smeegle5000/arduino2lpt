#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <time.h>

#define DATA 0x378
#define STAT 0x379
#define CTRL 0x37A

int main() {
    FILE *f;
    unsigned char b, hdr[4];
    unsigned long size, i;
    time_t start, last;

    outp(CTRL, 0x80);

    f = fopen("recieved.txt", "wb");
    setvbuf(f, NULL, _IOFBF, 8192);

    for (i = 0; i < 4; i++) {
        while (!(inp(STAT) & 0x40));
        hdr[i] = inp(DATA);
	printf("Header byte %lu: %02X\n", i, hdr[i]);
        outp(CTRL, 0x81);
        while (inp(STAT) & 0x40);
        outp(CTRL, 0x80);
    }
    size = *(unsigned long *)hdr;
    printf("Size: %lu\n", size);

    start = last = time(NULL);
    for (i = 0; i < size; i++) {
        while (!(inp(STAT) & 0x40));
        b = inp(DATA);
        fputc(b, f);
        outp(CTRL, 0x81);
        while (inp(STAT) & 0x40);
        outp(CTRL, 0x80);

        if ((i & 0x3FF) == 0) {          // check time only every 1024 bytes
            if (time(NULL) - last >= 2) {
                printf("Received %lu / %lu\n", i + 1, size);
                last = time(NULL);
            }
        }
    }

    fclose(f);
    printf("Complete: %lu bytes, %.1f bytes/sec\n", size, size / (double)(time(NULL) - start));
    return 0;
}