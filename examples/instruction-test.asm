;;; instruction-test.asm: A file that contains all possible opcodes in
;;; order, with proper calling convention.  Used to test lexer and
;;; parser but isn't a semantically correct program, but may be run as
;;; first instruction is halt (so program will stop immediately).
  halt
  push.byte 1
  push.hword 2
  push.word 3
  pop.byte
  pop.hword
  pop.word
  push-reg.byte 1
  push-reg.hword 2
  push-reg.word 3
  mov.byte 1
  mov.hword 2
  mov.word 3
  dup.byte 1
  dup.hword 2
  dup.word 3
  not.byte
  not.hword
  not.word
  or.byte
  or.hword
  or.word
  and.byte
  and.hword
  and.word
  xor.byte
  xor.hword
  xor.word
  eq.byte
  eq.hword
  eq.word
  plus.byte
  plus.hword
  plus.word
  print.char
  print.byte
  print.int
  print.hword
  print.long
  print.word
  jump.abs 1
  jump.stack
  jump.register 2
  jump.if.byte 1
  jump.if.hword 2
  jump.if.word 3
