  ;; Allocate a buffer of 3 characters
  malloc.byte 3
  mov.word 0
  ;; Setup the buffer to be equivalent to "abc"
  push.reg.word 0
  push.byte 'a'
  mset.byte 0
  push.reg.word 0
  push.byte 'b'
  mset.byte 1
  push.reg.word 0
  push.byte 'c'
  mset.byte 2

  ;; Save buffer to W[8] because the first 8 registers should be
  ;; reserved for library routines as it may be overwritten
  push.reg.word 0
  mov.word 8

  push.reg.word 0
  ;; Call the routine
  call print_cptr

  ;; Delete allocated buffer
  push.reg.word 8
  mdelete

  halt

;;; print_cptr: Prints pointer to a buffer of characters.  Pointer
;;; should be on the stack as a word.
print_cptr:
  ;; Save pointer in layout to W[0], P -> W[0]
  mov.word 0
  ;; iterator I -> W[1]
  push.word 0
  mov.word 1
  ;; (W[0])[W[1]] -> P[I]
  push.reg.word 0               ; <-- #
  push.reg.word 1
  mget.stack.byte
  print.char

  ;; I += 1
  push.reg.word 1
  push.word 1
  plus.word
  mov.word 1

  ;; if I != |P| ...
  push.reg.word 1
  push.reg.word 0
  msize
  eq.word
  not.byte
  ;; then go to #
  jump.if.byte *-13
  ;; else print a newline
  push.byte '\n'
  print.char
  ;; return back to the caller
  ret
