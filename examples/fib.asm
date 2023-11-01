;;; fib.asm: A program that generates the fibonacci numbers up to a
;;; very large bound (~UINT64_MAX).  Using the registers to store the
;;; pairs of fibonacci numbers, we ensure only a finite amount of
;;; memory is necessary for this program to function, unlike a pure
;;; stack version.
  ;; Setup initial REG[0] = 1 and REG[1] = 1
  push.word 1
  mov.word 0
  push.word 1
  mov.word 1

  ;; Print REG[0] and REG[1]
  ;; Here is the loop back point `#`
  push.byte '\t'
  print.char
  push.reg.word 0
  print.word
  push.byte '\n'
  print.char

  push.byte '\t'
  print.char
  push.reg.word 1
  print.word
  push.byte '\n'
  print.char

  ;;  REG[0] += REG[1]
  push.reg.word 0
  push.reg.word 1
  plus.word
  mov.word 0

  ;;  REG[1] += REG[0]
  push.reg.word 0
  push.reg.word 1
  plus.word
  mov.word 1

  push.word 12200160415121876738
  push.reg.word 1
  gte.word
  ;; Jump to `#`
  jump.if.byte 4
  halt
