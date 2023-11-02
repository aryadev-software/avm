  malloc.byte 3
  mov.word 0
  push.reg.word 0
  push.byte 'a'
  mset.byte 0
  push.reg.word 0
  push.byte 'b'
  mset.byte 1
  push.reg.word 0
  push.byte 'c'
  mset.byte 2

  push.reg.word 0
  mov.word 8
  push.reg.word 0
  call print_cptr
  push.reg.word 8
  mdelete

  push.byte '\n'
  print.char

  halt

;;; print_cptr: Prints pointer to a buffer of characters.  Pointer
;;; should be on the stack as a word.
print_cptr:
  mov.word 0
  push.word 0
  mov.word 1
  push.reg.word 0
  push.reg.word 1
  mget.stack.byte
  print.char
  push.reg.word 1
  push.word 1
  plus.word
  mov.word 1
  push.reg.word 1
  push.reg.word 0
  msize
  eq.word
  not.byte
  jump.if.byte *-13
  ret
