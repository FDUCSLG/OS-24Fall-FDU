.section ".text"

.global vectortable
.balign 0x800
vectortable:
/* Current EL with sp0 */
  mov x0, 0
  b 0x1234500
.balign 0x80
  mov x0, 1
  b 0x1234500
.balign 0x80
  mov x0, 2
  b 0x1234500
.balign 0x80
  mov x0, 3
  b 0x1234500

/* Current EL with spx */
.balign 0x80
  b 0x1234500
.balign 0x80
  b 0x1234500
.balign 0x80
  mov x0, 4
  b 0x1234500
.balign 0x80
  mov x0, 5
  b 0x1234500

/* Lower EL using aarch64 */
.balign 0x80
  b 0x1234500
.balign 0x80
  b 0x1234500
.balign 0x80
  mov x0, 8
  b 0x1234500
.balign 0x80
  mov x0, 9
  b 0x1234500

/* Lower EL using aarch32 */
.balign 0x80
  mov x0, 10
  b 0x1234500
.balign 0x80
  mov x0, 11
  b 0x1234500
.balign 0x80
  mov x0, 12
  b 0x1234500
.balign 0x80
  mov x0, 13
  b 0x1234500