# arduino2lpt
file transfer program from a modern computer to a dos computer with a parallel port using an arduino as a middle man

i highly encourage anyone to make changes, i know there is a lot of speed being left on the table

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
testing found a speed of approx 10600 bytes per second on a pro mini, 230000 bytes per second on an uno r4 minima
i believe that the arduino end is the bottleneck currently, next steps would be to try a faster board like a pi pico
the transfer protocol isnt the greatest, but it seems very reliable at least.
it does need a bidirectional capable port in this state, i hope to add alternate modes in the future
at this time the arduino sets data bits, sets a ready pin, dos sees the ready pin, reads byte, sets a ready pin itself, arduino reads dos ready pin, drops its own ready, clears data bus and both sides do their thing to prepare for the next packet

i am on a toshiba 386 laptop
IF YOU DONT HAVE A TOSHIBA LAPTOP THIS WILL NOT WORK OUT OF THE BOX
my toshiba uses bit 7 of the control reg (0x37A 80) to enable bidirectional mode on the lpt port, most other computers apparently use bit 5 (0x37A 20), this will need to be updated in one of the files before using
also be aware of possible inverted output on db25 pin 1 (arduino pin D2)

lpt.ino is the arduino sketch, i personally used a 5v arduino pro mini
lpttransfer.py is the program to dice a file up and send it to the arduino
transfer.bas is for hand typing into c:/dos/qbasic if you cannot get the exe transferred in another way, the hardcoded size value matches 8bitbi.exe reset the arduino and wait a moment before beginning transfer for best chance of success
8bitbi.cpp is the dos program, source
8bitbi.exe is 8bitbi.cpp, compiled using borland cpp 3.1
</pre>


<pre>
roadmap:
  more transfer modes, specifically a 12 bit mode, 4 bit unidirectional compatible mode, and perhaps try talking to existing dos/win apps like interlink, (Parallel Line Internet Protocol?)
  file chunking so one bad bit doesnt ruin a large transfer, speed throttling if it gets bad enough, as i found the higher speed on the newer arduino to cause some crc mismatches on large files, this may not be actually needed in the long run
  upload to pc capability
  dos side file picker, python sends a directory listing of a specified folder, dos side allows you to browse and request files, would just trigger the python to start transferring and hand off to the self contained transfer mode
  part of that includes multi file queuing, and/or automatic file/folder compression, transmission, and extraction, ideally all done outside of the transfer exe
  lpt port tester (prints out pin states and allows manual pin toggling for debugging)
  change the active registers with a setting so the whole lot doesnt have to be recompiled because toshiba wants to be special with a non-standard control bit
</pre> 
