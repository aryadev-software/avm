;;; fib.asm: A program that generates the fibonacci numbers up to a
;;; very large bound (~UINT64_MAX).  Using the registers to store the
;;; pairs of fibonacci numbers, we ensure only a finite amount of
;;; memory is necessary for this program to function, unlike a pure
;;; stack version.

  ;; Setup entrypoint
  global main
main:
  ;; Setup iterator i
  push.word 1
  mov.word 2
  ;; Setup initial REG[0] = 1 and REG[1] = 1
  push.word 1
  mov.word 0
  push.word 1
  mov.word 1

  ;; Print REG[0] and REG[1]
loopback:
  call print_pair

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
  ;; Jump to `loopback`
  jump.if.byte loopback
  halt

print_pair:
  push.byte '\t'
  print.char
  call print_i
  push.byte ':'
  print.char
  push.byte ' '
  print.char
  call print_reg_0
  push.byte '\n'
  print.char
  call increment_i
  push.byte '\t'
  print.char
  call print_i
  push.byte ':'
  print.char
  push.byte ' '
  print.char
  call print_reg_1
  push.byte '\n'
  print.char
  call increment_i
  ret

increment_i:
  push.reg.word 2
  push.word 1
  plus.word
  mov.word 2
  ret

print_i:
  push.reg.word 2
  print.word
  ret

print_reg_0:
  push.reg.word 0
  print.word
  ret

print_reg_1:
  push.reg.word 1
  print.word
  ret
