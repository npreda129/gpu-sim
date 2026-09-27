; simple kernel: copy all 1s to the memory address thread ID indexes
LI r2 255
LI r1 1
LI r4 8
LI r6 4
FMA r2 r1 r3
SHL r2 r4 r2
FMA r2 r1 r3
; SHL r2 r4 r2
; FMA r2 r1 r3
; SHL r2 r4 r2
; FMA r2 r1 r3
FMA rT r6 r5
SW r3 r1 r5
HALT r0 r0 r0
