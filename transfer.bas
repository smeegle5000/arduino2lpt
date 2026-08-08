OUT &H37A, &H80
OPEN "recieved.txt" FOR BINARY AS #1

DIM size AS LONG
DIM c AS STRING * 1
DIM b(3) AS INTEGER

FOR i = 0 TO 3
  DO WHILE (INP(&H379) AND &H40) = 0
  LOOP
  b(i) = INP(&H378)
  PRINT "Header byte "; i; ": "; HEX$(b(i))
  OUT &H37A, &H81
  DO WHILE (INP(&H379) AND &H40) <> 0
  LOOP
  OUT &H37A, &H80
NEXT i
size = CVL(CHR$(b(0)) + CHR$(b(1)) + CHR$(b(2)) + CHR$(b(3)))
PRINT "Size: "; size

startTime! = TIMER
lastPrint! = TIMER

FOR i = 0 TO size - 1
  DO WHILE (INP(&H379) AND &H40) = 0
  LOOP
  b% = INP(&H378)
  c = CHR$(b%)
  PUT #1, , c
  OUT &H37A, &H81
  DO WHILE (INP(&H379) AND &H40) <> 0
  LOOP
  OUT &H37A, &H80
  IF TIMER - lastPrint! >= 2 THEN
  PRINT "Recieved "; i + 1; " / "; size; " bytes"
  lastPrint! = TIMER
  END IF

NEXT i

CLOSE #1

PRINT "Complete: "; size; " bytes"; r
elapsed! = TIMER - startTime!
PRINT "Speed: "; INT((i + 1) / elapsed!); " bytes/sec"




