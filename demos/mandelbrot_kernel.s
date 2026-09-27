; mandelbrot. one launch is one iteration: z = z^2 + c
; registers persist between launches, so z (r8, r9) and the iteration count
; (r7) carry over. launch it once per iteration
; memory: 0 framebuffer address (int), 4 left (float), 8 top (float),
; 12 multiplier (float), 16 width (float), 20 1/width (float),
; 24 color step (int, per channel increments packed as 0xRRGGBB00)
; pixel i is written to framebuffer + 4*i
;
; ---- constants: floats with low 16 bits of 0 are built with LI, SHL ----
LI r1 16
; r2 = 1.0
LI r2 16256
SHL r2 r1 r2
; r3 = -1.0
LI r3 49024
SHL r3 r1 r3
; r5 = 0.5
LI r5 16128
SHL r5 r1 r5
; ---- thread id -> pixel center (px, py) ----
; int -> float: the bits 0x4B000000 + n are the float 2^23 + n
LI r4 19200
SHL r4 r1 r4
LI r1 1
FMA rT r1 r4
CMOV r4 r4 r10
; r4 = 2^23
LI r4 19200
LI r1 16
SHL r4 r1 r4
; r10 = tid + 0.5
FMA r4 r3 r10
FMA r5 r2 r10
; r11 = row = round((tid + 0.5) / W - 0.5)
; rounds by adding and removing 1.5 * 2^23. the sum stays in [2^23, 2^24)
; where floats are spaced 1 apart
LI r11 0
FMA r5 r3 r11
LI r1 20
LW r1 r0 r6
FMA r10 r6 r11
LI r6 19264
LI r1 16
SHL r6 r1 r6
FMA r6 r2 r11
FMA r6 r3 r11
; r10 = px = tid + 0.5 - W * row, r11 = py = row + 0.5
LI r12 0
FMA r11 r3 r12
LI r1 16
LW r1 r0 r6
FMA r12 r6 r10
FMA r5 r2 r11
; ---- c = (left + px * multiplier, top - py * multiplier) ----
; r12 = real part
LI r1 4
LW r1 r0 r12
LI r1 12
LW r1 r0 r6
FMA r10 r6 r12
; r10 = imaginary part
LI r13 0
FMA r6 r3 r13
LI r1 8
LW r1 r0 r10
FMA r11 r13 r10
; ---- z = z^2 + c ----
; r11 = zr^2 - zi^2 + cr
CMOV r12 r12 r11
FMA r8 r8 r11
LI r13 0
FMA r9 r3 r13
FMA r9 r13 r11
; r10 = 2 * zr * zi + ci
FMA r8 r9 r10
FMA r8 r9 r10
CMOV r10 r10 r9
CMOV r11 r11 r8
; ---- count iterations where |z|^2 <= 4 ----
; once z escapes it grows to inf or nan. both compare above 4 as
; unsigned ints
LI r11 0
FMA r8 r8 r11
FMA r9 r9 r11
; r6 = 4.0
LI r6 16512
LI r1 16
SHL r6 r1 r6
; sign flag is set when |z|^2 <= 4. r6 = that flag
CMP r11 r6 r0
CMOV rF rF r6
LI r1 1
SHR r6 r1 r6
FMA r6 r1 r7
; ---- color: count * color step if z escaped, black if it hasn't yet ----
LI r1 24
LW r1 r0 r2
LI r4 0
FMA r7 r2 r4
CMP r6 r0 r0
CMOV r4 r0 r5
LI r1 0
LW r1 r0 r2
LI r1 4
FMA rT r1 r2
SW r5 r0 r2
HALT r0 r0 r0
