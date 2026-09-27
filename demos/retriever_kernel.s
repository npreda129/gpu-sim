; 3d renderer. one launch draws one face; launch it numFaces times
; memory layout and conventions are documented in demos/retriever.h
; registers: r0 = 0, r1 = address temp, r5 = 1.0f, r6 = int 1, r7 = -1.0f
; r8-r13 are float, but int registers can hold float bits for FMA operands
;
; ---- globals ----
LI r6 1
LI r1 64
LW r1 r0 r5
LI r1 68
LW r1 r0 r7
LI r1 156
SW r0 r0 r1
; ---- sin and cos of yaw (taylor series, angle must be in [-pi, pi]) ----
LI r1 16
LW r1 r0 r8
LI r9 0
FMA r8 r8 r9
; sin(x) = x * P(x^2)
LI r1 104
LW r1 r0 r10
LI r1 100
LW r1 r0 r11
FMA r9 r10 r11
LI r1 96
LW r1 r0 r10
FMA r9 r11 r10
LI r1 92
LW r1 r0 r11
FMA r9 r10 r11
LI r1 88
LW r1 r0 r10
FMA r9 r11 r10
LI r1 64
LW r1 r0 r11
FMA r9 r10 r11
LI r12 0
FMA r8 r11 r12
LI r1 132
SW r12 r0 r1
LI r13 0
FMA r12 r7 r13
LI r1 148
SW r13 r0 r1
; cos(x) = Q(x^2)
LI r1 128
LW r1 r0 r10
LI r1 124
LW r1 r0 r11
FMA r9 r10 r11
LI r1 120
LW r1 r0 r10
FMA r9 r11 r10
LI r1 116
LW r1 r0 r11
FMA r9 r10 r11
LI r1 112
LW r1 r0 r10
FMA r9 r11 r10
LI r1 108
LW r1 r0 r11
FMA r9 r10 r11
LI r1 64
LW r1 r0 r10
FMA r9 r11 r10
LI r1 136
SW r10 r0 r1
; ---- sin and cos of pitch (taylor series, angle must be in [-pi, pi]) ----
LI r1 20
LW r1 r0 r8
LI r9 0
FMA r8 r8 r9
; sin(x) = x * P(x^2)
LI r1 104
LW r1 r0 r10
LI r1 100
LW r1 r0 r11
FMA r9 r10 r11
LI r1 96
LW r1 r0 r10
FMA r9 r11 r10
LI r1 92
LW r1 r0 r11
FMA r9 r10 r11
LI r1 88
LW r1 r0 r10
FMA r9 r11 r10
LI r1 64
LW r1 r0 r11
FMA r9 r10 r11
LI r12 0
FMA r8 r11 r12
LI r1 140
SW r12 r0 r1
LI r13 0
FMA r12 r7 r13
LI r1 152
SW r13 r0 r1
; cos(x) = Q(x^2)
LI r1 128
LW r1 r0 r10
LI r1 124
LW r1 r0 r11
FMA r9 r10 r11
LI r1 120
LW r1 r0 r10
FMA r9 r11 r10
LI r1 116
LW r1 r0 r11
FMA r9 r10 r11
LI r1 112
LW r1 r0 r10
FMA r9 r11 r10
LI r1 108
LW r1 r0 r11
FMA r9 r10 r11
LI r1 64
LW r1 r0 r10
FMA r9 r11 r10
LI r1 144
SW r10 r0 r1
; ---- vertex 0: world -> camera -> screen ----
; r4 = address of vertex = VERT_ADDR + 12 * face[k]
LI r1 52
LW r1 r0 r2
LI r3 0
FMA r2 r6 r3
LW r3 r0 r3
LI r1 56
LW r1 r0 r4
LI r1 12
FMA r3 r1 r4
; r8, r9, r10 = vertex - camera
LW r4 r0 r8
LI r1 4
LW r1 r0 r1
FMA r1 r7 r8
LI r1 4
FMA r1 r6 r4
LW r4 r0 r9
LI r1 8
LW r1 r0 r1
FMA r1 r7 r9
LI r1 4
FMA r1 r6 r4
LW r4 r0 r10
LI r1 12
LW r1 r0 r1
FMA r1 r7 r10
; yaw: r11 = cy*dx - sy*dz, r12 = sy*dx + cy*dz
LI r11 0
LI r1 136
LW r1 r0 r2
FMA r2 r8 r11
LI r1 148
LW r1 r0 r3
FMA r3 r10 r11
LI r12 0
FMA r2 r10 r12
LI r1 132
LW r1 r0 r3
FMA r3 r8 r12
; pitch: r8 = cp*dy - sp*r12, r10 = sp*dy + cp*r12
LI r8 0
LI r1 144
LW r1 r0 r2
FMA r2 r9 r8
LI r1 152
LW r1 r0 r3
FMA r3 r12 r8
LI r10 0
FMA r2 r12 r10
LI r1 140
LW r1 r0 r3
FMA r3 r9 r10
; camera space x, y, z = r11, r8, r10. INVALID += (z < near)
LI r9 0
LI r1 48
LW r1 r0 r2
FMA r2 r7 r9
FMA r10 r5 r9
LI r2 31
SHR r9 r2 r2
LI r1 156
LW r1 r0 r3
FMA r2 r6 r3
SW r3 r0 r1
; r12 = 1/r10: bit-trick guess, then 3 newton steps r += r*(1 - x*r)
LI r1 80
LW r1 r0 r2
LI r1 84
LW r1 r0 r3
FMA r10 r3 r2
CMOV r2 r2 r12
LI r13 0
FMA r10 r7 r13
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
; r13 = focal / z. screen x = HALF_W + x*r13, screen y = HALF_H - y*r13
LI r13 0
LI r1 44
LW r1 r0 r2
FMA r2 r12 r13
LI r1 36
LW r1 r0 r9
FMA r11 r13 r9
LI r1 160
SW r9 r0 r1
LI r1 40
LW r1 r0 r9
LI r11 0
FMA r13 r7 r11
FMA r11 r8 r9
LI r1 164
SW r9 r0 r1
; 1/z is linear in screen space, so it is what gets interpolated
LI r1 168
SW r12 r0 r1
; ---- vertex 1: world -> camera -> screen ----
; r4 = address of vertex = VERT_ADDR + 12 * face[k]
LI r1 52
LW r1 r0 r2
LI r3 4
FMA r2 r6 r3
LW r3 r0 r3
LI r1 56
LW r1 r0 r4
LI r1 12
FMA r3 r1 r4
; r8, r9, r10 = vertex - camera
LW r4 r0 r8
LI r1 4
LW r1 r0 r1
FMA r1 r7 r8
LI r1 4
FMA r1 r6 r4
LW r4 r0 r9
LI r1 8
LW r1 r0 r1
FMA r1 r7 r9
LI r1 4
FMA r1 r6 r4
LW r4 r0 r10
LI r1 12
LW r1 r0 r1
FMA r1 r7 r10
; yaw: r11 = cy*dx - sy*dz, r12 = sy*dx + cy*dz
LI r11 0
LI r1 136
LW r1 r0 r2
FMA r2 r8 r11
LI r1 148
LW r1 r0 r3
FMA r3 r10 r11
LI r12 0
FMA r2 r10 r12
LI r1 132
LW r1 r0 r3
FMA r3 r8 r12
; pitch: r8 = cp*dy - sp*r12, r10 = sp*dy + cp*r12
LI r8 0
LI r1 144
LW r1 r0 r2
FMA r2 r9 r8
LI r1 152
LW r1 r0 r3
FMA r3 r12 r8
LI r10 0
FMA r2 r12 r10
LI r1 140
LW r1 r0 r3
FMA r3 r9 r10
; camera space x, y, z = r11, r8, r10. INVALID += (z < near)
LI r9 0
LI r1 48
LW r1 r0 r2
FMA r2 r7 r9
FMA r10 r5 r9
LI r2 31
SHR r9 r2 r2
LI r1 156
LW r1 r0 r3
FMA r2 r6 r3
SW r3 r0 r1
; r12 = 1/r10: bit-trick guess, then 3 newton steps r += r*(1 - x*r)
LI r1 80
LW r1 r0 r2
LI r1 84
LW r1 r0 r3
FMA r10 r3 r2
CMOV r2 r2 r12
LI r13 0
FMA r10 r7 r13
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
; r13 = focal / z. screen x = HALF_W + x*r13, screen y = HALF_H - y*r13
LI r13 0
LI r1 44
LW r1 r0 r2
FMA r2 r12 r13
LI r1 36
LW r1 r0 r9
FMA r11 r13 r9
LI r1 172
SW r9 r0 r1
LI r1 40
LW r1 r0 r9
LI r11 0
FMA r13 r7 r11
FMA r11 r8 r9
LI r1 176
SW r9 r0 r1
; 1/z is linear in screen space, so it is what gets interpolated
LI r1 180
SW r12 r0 r1
; ---- vertex 2: world -> camera -> screen ----
; r4 = address of vertex = VERT_ADDR + 12 * face[k]
LI r1 52
LW r1 r0 r2
LI r3 8
FMA r2 r6 r3
LW r3 r0 r3
LI r1 56
LW r1 r0 r4
LI r1 12
FMA r3 r1 r4
; r8, r9, r10 = vertex - camera
LW r4 r0 r8
LI r1 4
LW r1 r0 r1
FMA r1 r7 r8
LI r1 4
FMA r1 r6 r4
LW r4 r0 r9
LI r1 8
LW r1 r0 r1
FMA r1 r7 r9
LI r1 4
FMA r1 r6 r4
LW r4 r0 r10
LI r1 12
LW r1 r0 r1
FMA r1 r7 r10
; yaw: r11 = cy*dx - sy*dz, r12 = sy*dx + cy*dz
LI r11 0
LI r1 136
LW r1 r0 r2
FMA r2 r8 r11
LI r1 148
LW r1 r0 r3
FMA r3 r10 r11
LI r12 0
FMA r2 r10 r12
LI r1 132
LW r1 r0 r3
FMA r3 r8 r12
; pitch: r8 = cp*dy - sp*r12, r10 = sp*dy + cp*r12
LI r8 0
LI r1 144
LW r1 r0 r2
FMA r2 r9 r8
LI r1 152
LW r1 r0 r3
FMA r3 r12 r8
LI r10 0
FMA r2 r12 r10
LI r1 140
LW r1 r0 r3
FMA r3 r9 r10
; camera space x, y, z = r11, r8, r10. INVALID += (z < near)
LI r9 0
LI r1 48
LW r1 r0 r2
FMA r2 r7 r9
FMA r10 r5 r9
LI r2 31
SHR r9 r2 r2
LI r1 156
LW r1 r0 r3
FMA r2 r6 r3
SW r3 r0 r1
; r12 = 1/r10: bit-trick guess, then 3 newton steps r += r*(1 - x*r)
LI r1 80
LW r1 r0 r2
LI r1 84
LW r1 r0 r3
FMA r10 r3 r2
CMOV r2 r2 r12
LI r13 0
FMA r10 r7 r13
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
CMOV r5 r5 r9
FMA r13 r12 r9
FMA r12 r9 r12
; r13 = focal / z. screen x = HALF_W + x*r13, screen y = HALF_H - y*r13
LI r13 0
LI r1 44
LW r1 r0 r2
FMA r2 r12 r13
LI r1 36
LW r1 r0 r9
FMA r11 r13 r9
LI r1 184
SW r9 r0 r1
LI r1 40
LW r1 r0 r9
LI r11 0
FMA r13 r7 r11
FMA r11 r8 r9
LI r1 188
SW r9 r0 r1
; 1/z is linear in screen space, so it is what gets interpolated
LI r1 192
SW r12 r0 r1
; ---- edge 0 (v1 -> v2): E(p) = A*px + B*py + C ----
; A = ay - by
LI r1 176
LW r1 r0 r8
LI r1 188
LW r1 r0 r2
FMA r2 r7 r8
; B = bx - ax
LI r1 184
LW r1 r0 r9
LI r1 172
LW r1 r0 r3
FMA r3 r7 r9
; C = -(A*ax + B*ay)
LI r10 0
FMA r8 r3 r10
LI r1 176
LW r1 r0 r2
FMA r9 r2 r10
LI r11 0
FMA r10 r7 r11
LI r1 196
SW r8 r0 r1
LI r1 200
SW r9 r0 r1
LI r1 204
SW r11 r0 r1
; r12 = twice the signed area = E0(v0). positive means front facing
CMOV r11 r11 r12
LI r1 160
LW r1 r0 r2
FMA r8 r2 r12
LI r1 164
LW r1 r0 r2
FMA r9 r2 r12
; ---- edge 1 (v2 -> v0): E(p) = A*px + B*py + C ----
; A = ay - by
LI r1 188
LW r1 r0 r8
LI r1 164
LW r1 r0 r2
FMA r2 r7 r8
; B = bx - ax
LI r1 160
LW r1 r0 r9
LI r1 184
LW r1 r0 r3
FMA r3 r7 r9
; C = -(A*ax + B*ay)
LI r10 0
FMA r8 r3 r10
LI r1 188
LW r1 r0 r2
FMA r9 r2 r10
LI r11 0
FMA r10 r7 r11
LI r1 208
SW r8 r0 r1
LI r1 212
SW r9 r0 r1
LI r1 216
SW r11 r0 r1
; ---- edge 2 (v0 -> v1): E(p) = A*px + B*py + C ----
; A = ay - by
LI r1 164
LW r1 r0 r8
LI r1 176
LW r1 r0 r2
FMA r2 r7 r8
; B = bx - ax
LI r1 172
LW r1 r0 r9
LI r1 160
LW r1 r0 r3
FMA r3 r7 r9
; C = -(A*ax + B*ay)
LI r10 0
FMA r8 r3 r10
LI r1 164
LW r1 r0 r2
FMA r9 r2 r10
LI r11 0
FMA r10 r7 r11
LI r1 220
SW r8 r0 r1
LI r1 224
SW r9 r0 r1
LI r1 228
SW r11 r0 r1
; ---- INVALID += (area < 0) + (area == 0) ----
LI r4 31
SHR r12 r4 r2
CMP r12 r0 r0
CMOV rF rF r3
SHR r3 r6 r3
FMA r3 r6 r2
LI r1 156
LW r1 r0 r3
FMA r2 r6 r3
SW r3 r0 r1
; r13 = 1/r12: bit-trick guess, then 3 newton steps r += r*(1 - x*r)
LI r1 80
LW r1 r0 r2
LI r1 84
LW r1 r0 r3
FMA r12 r3 r2
CMOV r2 r2 r13
LI r9 0
FMA r12 r7 r9
CMOV r5 r5 r8
FMA r9 r13 r8
FMA r13 r8 r13
CMOV r5 r5 r8
FMA r9 r13 r8
FMA r13 r8 r13
CMOV r5 r5 r8
FMA r9 r13 r8
FMA r13 r8 r13
; w_k / area, so 1/z at a pixel is E0*w0' + E1*w1' + E2*w2'
LI r10 0
LI r1 168
LW r1 r0 r2
FMA r2 r13 r10
LI r1 232
SW r10 r0 r1
LI r10 0
LI r1 180
LW r1 r0 r2
FMA r2 r13 r10
LI r1 236
SW r10 r0 r1
LI r10 0
LI r1 192
LW r1 r0 r2
FMA r2 r13 r10
LI r1 240
SW r10 r0 r1
; ---- per pixel: thread id -> pixel center (px, py) ----
; int -> float: the bits 0x4B000000 + n are the float 2^23 + n
LI r1 76
LW r1 r0 r2
FMA rT r6 r2
CMOV r2 r2 r8
LW r1 r0 r3
FMA r3 r7 r8
LI r1 72
LW r1 r0 r4
FMA r4 r5 r8
; row = round((tid + 0.5) / W - 0.5), rounding by adding and removing
; 1.5 * 2^23. the sum stays in [2^23, 2^24) where floats are spaced 1 apart
LI r9 0
FMA r4 r7 r9
LI r1 32
LW r1 r0 r2
FMA r8 r2 r9
LI r2 19264
LI r1 16
SHL r2 r1 r2
FMA r2 r5 r9
FMA r2 r7 r9
; px = tid + 0.5 - W * row, py = row + 0.5. move them into r3, r4
LI r10 0
FMA r9 r7 r10
LI r1 28
LW r1 r0 r2
FMA r10 r2 r8
FMA r4 r5 r9
CMOV r8 r8 r3
CMOV r9 r9 r4
; r8, r9, r10 = E0, E1, E2 at this pixel
LI r1 204
LW r1 r0 r8
LI r1 196
LW r1 r0 r2
FMA r2 r3 r8
LI r1 200
LW r1 r0 r2
FMA r2 r4 r8
LI r1 216
LW r1 r0 r9
LI r1 208
LW r1 r0 r2
FMA r2 r3 r9
LI r1 212
LW r1 r0 r2
FMA r2 r4 r9
LI r1 228
LW r1 r0 r10
LI r1 220
LW r1 r0 r2
FMA r2 r3 r10
LI r1 224
LW r1 r0 r2
FMA r2 r4 r10
; r11 = 1/z at this pixel
LI r11 0
LI r1 232
LW r1 r0 r2
FMA r8 r2 r11
LI r1 236
LW r1 r0 r2
FMA r9 r2 r11
LI r1 240
LW r1 r0 r2
FMA r10 r2 r11
; r3 = fail count = INVALID + sign bits of E0, E1, E2
LI r1 156
LW r1 r0 r3
LI r12 31
SHR r8 r12 r2
FMA r2 r6 r3
SHR r9 r12 r2
FMA r2 r6 r3
SHR r10 r12 r2
FMA r2 r6 r3
; depth test. positive floats compare correctly as unsigned ints
; larger 1/z is closer. the sign flag is set when new <= old
LI r1 24
LW r1 r0 r4
LI r2 4
FMA rT r2 r4
LW r4 r0 r8
CMP r11 r8 r0
CMOV rF rF r2
SHR r2 r6 r2
FMA r2 r6 r3
; r9 = face color, r1 = pixel address, r13 = old pixel
LI r1 52
LW r1 r0 r1
LI r2 12
FMA r2 r6 r1
LW r1 r0 r9
LI r1 0
LW r1 r0 r1
LI r2 4
FMA rT r2 r1
LW r1 r0 r13
; zero flag set when nothing failed: write the new color and depth
CMP r3 r0 r0
CMOV r9 r13 r13
SW r13 r0 r1
CMOV r11 r8 r8
SW r8 r0 r4
; ---- advance to the next face ----
LI r1 52
LW r1 r0 r2
LI r3 16
FMA r3 r6 r2
SW r2 r0 r1
HALT r0 r0 r0
