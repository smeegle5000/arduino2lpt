# arduino2lpt
file transfer program from a modern computer to a dos computer with a parallel port using an arduino as a middle man

wiring is:

|db25|arduino|
|-----|--------|
|1|D2
|2|D3
|3|D4
|4|D5
|5|D6
|6|D7
|7|D8
|8|D9
|9|D10
|10|D11
|11|D12
|12|D13
|13|A0
|14|A1
|15|A2
|16|A3
|17|A4
|18-25|Ground

<pre>
testing found a speed of approx 1900-2000 bytes per second (somewhat on par with 19200 serial)
i believe that the arduino end is the bottleneck currently, next steps would be to try a faster board like a pi pico
the transfer protocol isnt the greatest, but it seems very reliable at least, it does need a bidirectional capable port in this state

i am on a toshiba 386 laptop
IF YOU DONT HAVE A TOSHIBA LAPTOP THIS WILL NOT WORK OUT OF THE BOX
my toshiba uses bit 7 of the control reg (0x37A 80) to enable bidirectional mode on the lpt port, most other computers apparently use bit 5 (0x37A 20), this will need to be updated in one of the files before using
also be aware of possible inverted output on db25 pin 1 (arduino pin D2)

lpt.ino is the arduino sketch, i personally used a pro mini
lpttransfer.py is the program to dice a file up and send it to the arduino
transfer.bas is an older proof of concept c:/dos/qbasic version for testing, i'm not entirely sure if it still works
file2.cpp is the dos program, source
FILE3.EXE is file2.cpp, compiled using borland cpp 3.1
</pre>
