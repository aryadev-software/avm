;;; factorial.asm: A program that generates the factorials of each
;;;  number from 1 to 24 (24!~=UINT64_MAX).  Using the registers to
;;;  store `n` and `n!`.

  ;; Setup entrypoint
  global main
main:
  ;; Setup initial REG[0] = 1 and REG[1] = 1
  push.word 1
  mov.word 0
  push.word 1
  mov.word 1

  ;; Print `REG[0]: REG[1]`
loopback:
  push.byte '\t'
  print.char
  push.reg.word 0
  print.word
  push.byte ':'
  print.char
  push.byte ' '
  print.char
  push.reg.word 1
  print.word
  push.byte '\n'
  print.char

  ;;  REG[0] += 1
  push.reg.word 0
  push.word 1
  plus.word
  mov.word 0

  ;;  REG[1] *= REG[0]
  push.reg.word 0
  push.reg.word 1
  mult.word
  mov.word 1

  push.word 24
  push.reg.word 0
  gte.word
  ;; Jump to `#`
  jump.if.byte loopback
  halt
